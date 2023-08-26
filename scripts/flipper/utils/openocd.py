import socket
import subprocess
import logging


class OpenOCD:
    """OpenOCD cli wrapper"""

    COMMAND_TOKEN = "\x1a"

    def __init__(self, config: dict = {}) -> None:
        assert isinstance(config, dict)

        # Params base
        self.params = []

        self.gdb_port = 3333
        self.telnet_port = 4444
        self.tcl_port = 6666

        # Port
        if port_base := config.get("port_base", None):
            self.gdb_port = port_base
            self.tcl_port = port_base + 1
            self.telnet_port = port_base + 2

        self._add_command(f"gdb_port {self.gdb_port}")
        self._add_command(f"tcl_port {self.tcl_port}")
        self._add_command(f"telnet_port {self.telnet_port}")

        # Config files

        if interface := config.get("interface", None):
            pass
        else:
            interface = "interface/stlink.cfg"

        if target := config.get("target", None):
            pass
        else:
            target = "target/stm32wbx.cfg"

        self._add_file(interface)
        self._add_file(target)

        # Programmer settings
        if serial := config.get("serial", None):
            self._add_command(f"{serial}")

        # Other params
        if "params" in config:
            self.params += config["params"]

        # logging
        self.logger = logging.getLogger()

    def _add_command(self, command: str):
        self.params.append("-c")
        self.params.append(command)

    def _add_file(self, file: str):
        self.params.append("-f")
        self.params.append(file)

    def start(self, args: list[str] = []):
        """Start OpenOCD process"""

        params = ["openocd", *self.params, *args]
        self.logger.debug(f"_execute: {params}")
        self.process = subprocess.Popen(
            params, stderr=subprocess.PIPE, stdout=subprocess.PIPE
        )

        self._wait_for_openocd_tcl()

        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.connect(("127.0.0.1", self.tcl_port))

    def _wait_for_openocd_tcl(self):
        """Wait for OpenOCD to start"""
        # TODO Fl-3538: timeout
        while True:
            stderr = self.process.stderr
            if not stderr:
                break
            line = stderr.readline()
            if not line:
                break
            line = line.decode("utf-8").strip()
            self.logger.debug(f"OpenOCD: {line}")
            if "Listening on port" in line and "for tcl connections" in line:
                break

    def stop(self):
        self.send_tcl("exit")
        self.send_tcl("shutdown")
        self.socket.close()
        try:
            self.process.wait(timeout=10)
        except subprocess.TimeoutExpired as e:
            self.process.kill()
            self.logger.error("Failed to stop OpenOCD")
            self.logger.exception(e)
            self.postmortem()

    def send_tcl(self, cmd) -> str:
        """Send a command string to TCL RPC. Return the result that was read."""

        try:
            data = (cmd + OpenOCD.COMMAND_TOKEN).encode("utf-8")
            self.logger.debug(f"<- {data}")

            self.socket.send(data)
        except Exception as e:
            self.logger.error("Failed to send command to OpenOCD")
            self.logger.exception(e)
            self.postmortem()
            raise

        try:
            data = self._recv()
            return data
        except Exception as e:
            self.logger.error("Failed to receive response from OpenOCD")
            self.logger.exception(e)
            self.postmortem()
            raise

    def _recv(self):
        """Read from the stream until the token (\x1a) was received."""
        # TODO FL-3538: timeout
        data = bytes()
        while True:
            chunk = self.socket.recv(4096)
            data += chunk
            if bytes(OpenOCD.COMMAND_TOKEN, encoding="utf-8") in chunk:
                break

        self.logger.debug(f"-> {data}")

        data = data.decode("utf-8").strip()
        data = data[:-1]  # strip trailing \x1a

        return data

    def postmortem(self) -> None:
        """Postmortem analysis of the OpenOCD process"""
        stdout, stderr = self.process.communicate()

        log = self.logger.error
        if self.process.returncode == 0:
            log = self.logger.debug
            log("OpenOCD exited normally")
        else:
            log("OpenOCD exited with error")

        log(f"Exit code: {self.process.returncode}")
        for line in stdout.decode("utf-8").splitlines():
            log(f"Stdout: {line}")

        for line in stderr.decode("utf-8").splitlines():
            log(f"Stderr: {line}")

    def read_32(self, addr: int) -> int:
        """Read 32-bit value from memory"""
        data = self.send_tcl(f"mdw {addr}").strip()
        data = data.split(": ")[-1]
        data = int(data, 16)
        return data

    def write_32(self, addr: int, value: int) -> None:
        """Write 32-bit value to memory"""
        self.send_tcl(f"mww {addr} {value}")
