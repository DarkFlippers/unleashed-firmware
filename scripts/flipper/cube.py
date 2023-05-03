import logging
import subprocess


class CubeProgrammer:
    """STM32 Cube Programmer cli wrapper"""

    def __init__(self, config={}):
        assert isinstance(config, dict)
        # Params base
        self.params = []
        # Connect params
        connect = []
        if "port" in config and config["port"]:
            connect.append(f"port={config['port']}")
        else:
            connect.append("port=swd")
        if "serial" in config and config["serial"]:
            connect.append(f"sn={config['serial']}")
        self.params.append("-c " + " ".join(connect))
        # Other params
        if "params" in config:
            self.params += config["params"]
        # logging
        self.logger = logging.getLogger()

    def _execute(self, args):
        try:
            params = [
                "STM32_Programmer_CLI",
                "-q",
                *self.params,
                *args,
            ]
            self.logger.debug(f"_execute: {params}")
            output = subprocess.check_output(params)
        except subprocess.CalledProcessError as e:
            if e.output:
                print("Process output:\n", e.output.decode())
            print("Process return code:", e.returncode)
            raise e
        assert output
        return output.decode()

    def getVersion(self):
        self._execute(["--version"])

    def checkOptionBytes(self, option_bytes):
        output = self._execute(["-ob displ"])
        ob_correct = True
        for line in output.split("\n"):
            line = line.strip()
            if ":" not in line:
                self.logger.debug(f"Skipping line: {line}")
                continue
            key, data = line.split(":", 1)
            key = key.strip()
            data = data.strip()
            if key not in option_bytes.keys():
                self.logger.debug(f"Skipping key: {key}")
                continue
            self.logger.debug(f"Processing key: {key} {data}")
            value, comment = data.split(" ", 1)
            value = value.strip()
            comment = comment.strip()
            if option_bytes[key][0] != value:
                self.logger.error(
                    f"Invalid OB: {key} {value}, expected: {option_bytes[key][0]}"
                )
                ob_correct = False
        return ob_correct

    def setOptionBytes(self, option_bytes):
        options = []
        for key, (value, attr) in option_bytes.items():
            if "w" in attr:
                options.append(f"{key}={value}")
        self._execute(["-ob", *options])
        return True

    def flashBin(self, address, filename):
        self._execute(
            [
                "-d",
                filename,
                f"{address}",
            ]
        )

    def flashCore2(self, address, filename):
        self._execute(
            [
                "-fwupgrade",
                filename,
                f"{address}",
            ]
        )

    def deleteCore2RadioStack(self):
        self._execute(["-fwdelete"])

    def resetTarget(self):
        self._execute([])
