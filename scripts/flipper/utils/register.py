from dataclasses import dataclass
from flipper.utils.openocd import OpenOCD


@dataclass
class RegisterBitDefinition:
    name: str
    offset: int
    size: int
    value: int = 0


class Register32:
    def __init__(self, address: int, definition_list: list[RegisterBitDefinition]):
        self.__dict__["names"] = [definition.name for definition in definition_list]
        self.names = [definition.name for definition in definition_list]  # typecheck
        self.address = address
        self.definition_list = definition_list
        self.openocd = None

        # Validate that the definitions are not overlapping
        for i in range(len(definition_list)):
            for j in range(i + 1, len(definition_list)):
                if self._is_overlapping(definition_list[i], definition_list[j]):
                    raise ValueError("Register definitions are overlapping")

        self.freezed = True

    def _is_overlapping(
        self, a: RegisterBitDefinition, b: RegisterBitDefinition
    ) -> bool:
        if a.offset + a.size <= b.offset:
            return False
        if b.offset + b.size <= a.offset:
            return False
        return True

    def _get_definition(self, name: str) -> RegisterBitDefinition:
        for definition in self.definition_list:
            if definition.name == name:
                return definition
        raise ValueError(f"Register definition '{name}' not found")

    def get_definition_list(self) -> list[RegisterBitDefinition]:
        return self.definition_list

    def get_address(self) -> int:
        return self.address

    def set_reg_value(self, name: str, value: int):
        definition = self._get_definition(name)
        if value > (1 << definition.size) - 1:
            raise ValueError(
                f"Value {value} is too large for register definition '{name}'"
            )
        definition.value = value

    def get_reg_value(self, name: str) -> int:
        definition = self._get_definition(name)
        return definition.value

    def __getattr__(self, attr):
        if str(attr) in self.names:
            return self.get_reg_value(str(attr))
        else:
            return self.__dict__[attr]

    def __setattr__(self, attr, value):
        if str(attr) in self.names:
            self.set_reg_value(str(attr), value)
        else:
            if attr in self.__dict__ or "freezed" not in self.__dict__:
                self.__dict__[attr] = value
            else:
                raise AttributeError(f"Attribute '{attr}' not found")

    def __dir__(self):
        return self.names

    def set_openocd(self, openocd: OpenOCD):
        self.openocd = openocd

    def get_openocd(self) -> OpenOCD:
        if self.openocd is None:
            raise RuntimeError("OpenOCD is not installed")
        return self.openocd

    def set(self, value: int):
        for definition in self.definition_list:
            definition.value = (value >> definition.offset) & (
                (1 << definition.size) - 1
            )

    def get(self) -> int:
        value = 0
        for definition in self.definition_list:
            value |= definition.value << definition.offset
        return value

    def load(self):
        self.set(self.get_openocd().read_32(self.address))

    def store(self):
        self.get_openocd().write_32(self.address, self.get())
