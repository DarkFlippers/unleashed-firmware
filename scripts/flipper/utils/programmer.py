from abc import ABC, abstractmethod
from enum import Enum


class Programmer(ABC):
    def __init__(self):
        pass

    class RunMode(Enum):
        Run = "run"
        Stop = "stop"

    @abstractmethod
    def reset(self, mode: RunMode = RunMode.Run) -> bool:
        pass

    @abstractmethod
    def flash(self, address: int, file_path: str, verify: bool = True) -> bool:
        pass

    @abstractmethod
    def option_bytes_validate(self, file_path: str) -> bool:
        pass

    @abstractmethod
    def option_bytes_set(self, file_path: str) -> bool:
        pass

    @abstractmethod
    def option_bytes_recover(self) -> bool:
        pass

    @abstractmethod
    def otp_write(self, address: int, file_path: str) -> bool:
        pass
