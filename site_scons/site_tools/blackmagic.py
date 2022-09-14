from SCons.Errors import StopError


class BlackmagicResolver:
    BLACKMAGIC_HOSTNAME = "blackmagic.local"

    def __init__(self, env):
        self.env = env

    # On Win:
    #    'location': '1-5:x.0', 'name': 'COM4',
    #    'location': '1-5:x.2', 'name': 'COM13',
    # On Linux:
    #    'location': '1-1.2:1.0', 'name': 'ttyACM0',
    #    'location': '1-1.2:1.2', 'name': 'ttyACM1',
    # On MacOS:
    #    'location': '0-1.3', 'name': 'cu.usbmodemblackmagic1',
    #    'location': '0-1.3', 'name': 'cu.usbmodemblackmagic3',
    def _find_probe(self):
        import serial.tools.list_ports as list_ports

        ports = list(list_ports.grep("blackmagic"))
        if len(ports) == 0:
            # Blackmagic probe serial port not found, will be handled later
            pass
        elif len(ports) > 2:
            raise StopError("More than one Blackmagic probe found")
        else:
            # If you're getting any issues with auto lookup, uncomment this
            # print("\n".join([f"{p.device} {vars(p)}" for p in ports]))
            return sorted(ports, key=lambda p: f"{p.location}_{p.name}")[0]

    # Look up blackmagic probe hostname with dns
    def _resolve_hostname(self):
        import socket

        try:
            return socket.gethostbyname(self.BLACKMAGIC_HOSTNAME)
        except socket.gaierror:
            print("Failed to resolve Blackmagic hostname")
            return None

    def get_serial(self):
        if not (probe := self._find_probe()):
            return None
        # print(f"Found Blackmagic probe on {probe.device}")
        if self.env.subst("$PLATFORM") == "win32":
            return f"\\\\.\\{probe.device}"
        return probe.device

    def get_networked(self):
        if not (probe := self._resolve_hostname()):
            return None

        return f"tcp:{probe}:2345"

    def __str__(self):
        # print("distenv blackmagic", self.env.subst("$BLACKMAGIC"))
        if (blackmagic := self.env.subst("$BLACKMAGIC")) != "auto":
            return blackmagic

        # print("Looking for Blackmagic...")
        if probe := self.get_serial() or self.get_networked():
            return probe

        raise StopError("Please specify BLACKMAGIC=...")


def generate(env):
    env.SetDefault(BLACKMAGIC_ADDR=BlackmagicResolver(env))


def exists(env):
    return True
