from typing import Set, ClassVar
from dataclasses import dataclass, field


@dataclass(frozen=True)
class ApiEntryFunction:
    name: str
    returns: str
    params: str

    csv_type: ClassVar[str] = "Function"

    def dictify(self):
        return dict(name=self.name, type=self.returns, params=self.params)


@dataclass(frozen=True)
class ApiEntryVariable:
    name: str
    var_type: str

    csv_type: ClassVar[str] = "Variable"

    def dictify(self):
        return dict(name=self.name, type=self.var_type, params=None)


@dataclass(frozen=True)
class ApiHeader:
    name: str

    csv_type: ClassVar[str] = "Header"

    def dictify(self):
        return dict(name=self.name, type=None, params=None)


@dataclass
class ApiEntries:
    # These are sets, to avoid creating duplicates when we have multiple
    # declarations with same signature
    functions: Set[ApiEntryFunction] = field(default_factory=set)
    variables: Set[ApiEntryVariable] = field(default_factory=set)
    headers: Set[ApiHeader] = field(default_factory=set)
