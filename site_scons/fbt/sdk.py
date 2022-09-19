import operator
import os
import csv
import operator

from enum import Enum, auto
from typing import List, Set, ClassVar, Any
from dataclasses import dataclass, field

from cxxheaderparser.parser import CxxParser


# 'Fixing' complaints about typedefs
CxxParser._fundamentals.discard("wchar_t")

from cxxheaderparser.types import (
    EnumDecl,
    Field,
    ForwardDecl,
    FriendDecl,
    Function,
    Method,
    Typedef,
    UsingAlias,
    UsingDecl,
    Variable,
    Pointer,
    Type,
    PQName,
    NameSpecifier,
    FundamentalSpecifier,
    Parameter,
    Array,
    Value,
    Token,
    FunctionType,
)

from cxxheaderparser.parserstate import (
    State,
    EmptyBlockState,
    ClassBlockState,
    ExternBlockState,
    NamespaceBlockState,
)


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


class SymbolManager:
    def __init__(self):
        self.api = ApiEntries()
        self.name_hashes = set()

    # Calculate hash of name and raise exception if it already is in the set
    def _name_check(self, name: str):
        name_hash = gnu_sym_hash(name)
        if name_hash in self.name_hashes:
            raise Exception(f"Hash collision on {name}")
        self.name_hashes.add(name_hash)

    def add_function(self, function_def: ApiEntryFunction):
        if function_def in self.api.functions:
            return
        self._name_check(function_def.name)
        self.api.functions.add(function_def)

    def add_variable(self, variable_def: ApiEntryVariable):
        if variable_def in self.api.variables:
            return
        self._name_check(variable_def.name)
        self.api.variables.add(variable_def)

    def add_header(self, header: str):
        self.api.headers.add(ApiHeader(header))


def gnu_sym_hash(name: str):
    h = 0x1505
    for c in name:
        h = (h << 5) + h + ord(c)
    return str(hex(h))[-8:]


class SdkCollector:
    def __init__(self):
        self.symbol_manager = SymbolManager()

    def add_header_to_sdk(self, header: str):
        self.symbol_manager.add_header(header)

    def process_source_file_for_sdk(self, file_path: str):
        visitor = SdkCxxVisitor(self.symbol_manager)
        with open(file_path, "rt") as f:
            content = f.read()
        parser = CxxParser(file_path, content, visitor, None)
        parser.parse()

    def get_api(self):
        return self.symbol_manager.api


def stringify_array_dimension(size_descr):
    if not size_descr:
        return ""
    return stringify_descr(size_descr)


def stringify_array_descr(type_descr):
    assert isinstance(type_descr, Array)
    return (
        stringify_descr(type_descr.array_of),
        stringify_array_dimension(type_descr.size),
    )


def stringify_descr(type_descr):
    if isinstance(type_descr, (NameSpecifier, FundamentalSpecifier)):
        return type_descr.name
    elif isinstance(type_descr, PQName):
        return "::".join(map(stringify_descr, type_descr.segments))
    elif isinstance(type_descr, Pointer):
        # Hack
        if isinstance(type_descr.ptr_to, FunctionType):
            return stringify_descr(type_descr.ptr_to)
        return f"{stringify_descr(type_descr.ptr_to)}*"
    elif isinstance(type_descr, Type):
        return (
            f"{'const ' if type_descr.const else ''}"
            f"{'volatile ' if type_descr.volatile else ''}"
            f"{stringify_descr(type_descr.typename)}"
        )
    elif isinstance(type_descr, Parameter):
        return stringify_descr(type_descr.type)
    elif isinstance(type_descr, Array):
        # Hack for 2d arrays
        if isinstance(type_descr.array_of, Array):
            argtype, dimension = stringify_array_descr(type_descr.array_of)
            return (
                f"{argtype}[{stringify_array_dimension(type_descr.size)}][{dimension}]"
            )
        return f"{stringify_descr(type_descr.array_of)}[{stringify_array_dimension(type_descr.size)}]"
    elif isinstance(type_descr, Value):
        return " ".join(map(stringify_descr, type_descr.tokens))
    elif isinstance(type_descr, FunctionType):
        return f"{stringify_descr(type_descr.return_type)} (*)({', '.join(map(stringify_descr, type_descr.parameters))})"
    elif isinstance(type_descr, Token):
        return type_descr.value
    elif type_descr is None:
        return ""
    else:
        raise Exception("unsupported type_descr: %s" % type_descr)


class SdkCxxVisitor:
    def __init__(self, symbol_manager: SymbolManager):
        self.api = symbol_manager

    def on_variable(self, state: State, v: Variable) -> None:
        if not v.extern:
            return

        self.api.add_variable(
            ApiEntryVariable(
                stringify_descr(v.name),
                stringify_descr(v.type),
            )
        )

    def on_function(self, state: State, fn: Function) -> None:
        if fn.inline or fn.has_body:
            return

        self.api.add_function(
            ApiEntryFunction(
                stringify_descr(fn.name),
                stringify_descr(fn.return_type),
                ", ".join(map(stringify_descr, fn.parameters))
                + (", ..." if fn.vararg else ""),
            )
        )

    def on_define(self, state: State, content: str) -> None:
        pass

    def on_pragma(self, state: State, content: str) -> None:
        pass

    def on_include(self, state: State, filename: str) -> None:
        pass

    def on_empty_block_start(self, state: EmptyBlockState) -> None:
        pass

    def on_empty_block_end(self, state: EmptyBlockState) -> None:
        pass

    def on_extern_block_start(self, state: ExternBlockState) -> None:
        pass

    def on_extern_block_end(self, state: ExternBlockState) -> None:
        pass

    def on_namespace_start(self, state: NamespaceBlockState) -> None:
        pass

    def on_namespace_end(self, state: NamespaceBlockState) -> None:
        pass

    def on_forward_decl(self, state: State, fdecl: ForwardDecl) -> None:
        pass

    def on_typedef(self, state: State, typedef: Typedef) -> None:
        pass

    def on_using_namespace(self, state: State, namespace: List[str]) -> None:
        pass

    def on_using_alias(self, state: State, using: UsingAlias) -> None:
        pass

    def on_using_declaration(self, state: State, using: UsingDecl) -> None:
        pass

    def on_enum(self, state: State, enum: EnumDecl) -> None:
        pass

    def on_class_start(self, state: ClassBlockState) -> None:
        pass

    def on_class_field(self, state: State, f: Field) -> None:
        pass

    def on_class_method(self, state: ClassBlockState, method: Method) -> None:
        pass

    def on_class_friend(self, state: ClassBlockState, friend: FriendDecl) -> None:
        pass

    def on_class_end(self, state: ClassBlockState) -> None:
        pass


@dataclass(frozen=True)
class SdkVersion:
    major: int = 0
    minor: int = 0

    csv_type: ClassVar[str] = "Version"

    def __str__(self) -> str:
        return f"{self.major}.{self.minor}"

    def as_int(self) -> int:
        return ((self.major & 0xFFFF) << 16) | (self.minor & 0xFFFF)

    @staticmethod
    def from_str(s: str) -> "SdkVersion":
        major, minor = s.split(".")
        return SdkVersion(int(major), int(minor))

    def dictify(self) -> dict:
        return dict(name=str(self), type=None, params=None)


class VersionBump(Enum):
    NONE = auto()
    MAJOR = auto()
    MINOR = auto()


class ApiEntryState(Enum):
    PENDING = "?"
    APPROVED = "+"
    DISABLED = "-"
    # Special value for API version entry so users have less incentive to edit it
    VERSION_PENDING = "v"


# Class that stores all known API entries, both enabled and disabled.
# Also keeps track of API versioning
# Allows comparison and update from newly-generated API
class SdkCache:
    CSV_FIELD_NAMES = ("entry", "status", "name", "type", "params")

    def __init__(self, cache_file: str, load_version_only=False):
        self.cache_file_name = cache_file
        self.version = SdkVersion(0, 0)
        self.sdk = ApiEntries()
        self.disabled_entries = set()
        self.new_entries = set()
        self.loaded_dirty_version = False

        self.version_action = VersionBump.NONE
        self._load_version_only = load_version_only
        self.load_cache()

    def is_buildable(self) -> bool:
        return (
            self.version != SdkVersion(0, 0)
            and self.version_action == VersionBump.NONE
            and not self._have_pending_entries()
        )

    def _filter_enabled(self, sdk_entries):
        return sorted(
            filter(lambda e: e not in self.disabled_entries, sdk_entries),
            key=operator.attrgetter("name"),
        )

    def get_valid_names(self):
        syms = set(map(lambda e: e.name, self.get_functions()))
        syms.update(map(lambda e: e.name, self.get_variables()))
        return syms

    def get_functions(self):
        return self._filter_enabled(self.sdk.functions)

    def get_variables(self):
        return self._filter_enabled(self.sdk.variables)

    def get_headers(self):
        return self._filter_enabled(self.sdk.headers)

    def _get_entry_status(self, entry) -> str:
        if entry in self.disabled_entries:
            return ApiEntryState.DISABLED
        elif entry in self.new_entries:
            if isinstance(entry, SdkVersion):
                return ApiEntryState.VERSION_PENDING
            return ApiEntryState.PENDING
        else:
            return ApiEntryState.APPROVED

    def _format_entry(self, obj):
        obj_dict = obj.dictify()
        obj_dict.update(
            dict(
                entry=obj.csv_type,
                status=self._get_entry_status(obj).value,
            )
        )
        return obj_dict

    def save(self) -> None:
        if self._load_version_only:
            raise Exception("Only SDK version was loaded, cannot save")

        if self.version_action == VersionBump.MINOR:
            self.version = SdkVersion(self.version.major, self.version.minor + 1)
        elif self.version_action == VersionBump.MAJOR:
            self.version = SdkVersion(self.version.major + 1, 0)

        if self._have_pending_entries():
            self.new_entries.add(self.version)
            print(
                f"API version is still WIP: {self.version}. Review the changes and re-run command."
            )
            print(f"Entries to review:")
            print(
                "\n".join(
                    map(
                        str,
                        filter(
                            lambda e: not isinstance(e, SdkVersion), self.new_entries
                        ),
                    )
                )
            )
        else:
            print(f"API version {self.version} is up to date")

        regenerate_csv = (
            self.loaded_dirty_version
            or self._have_pending_entries()
            or self.version_action != VersionBump.NONE
        )

        if regenerate_csv:
            str_cache_entries = [self.version]
            name_getter = operator.attrgetter("name")
            str_cache_entries.extend(sorted(self.sdk.headers, key=name_getter))
            str_cache_entries.extend(sorted(self.sdk.functions, key=name_getter))
            str_cache_entries.extend(sorted(self.sdk.variables, key=name_getter))

            with open(self.cache_file_name, "wt", newline="") as f:
                writer = csv.DictWriter(f, fieldnames=SdkCache.CSV_FIELD_NAMES)
                writer.writeheader()

                for entry in str_cache_entries:
                    writer.writerow(self._format_entry(entry))

    def _process_entry(self, entry_dict: dict) -> None:
        entry_class = entry_dict["entry"]
        entry_status = entry_dict["status"]
        entry_name = entry_dict["name"]

        entry = None
        if entry_class == SdkVersion.csv_type:
            self.version = SdkVersion.from_str(entry_name)
            if entry_status == ApiEntryState.VERSION_PENDING.value:
                self.loaded_dirty_version = True
        elif entry_class == ApiHeader.csv_type:
            self.sdk.headers.add(entry := ApiHeader(entry_name))
        elif entry_class == ApiEntryFunction.csv_type:
            self.sdk.functions.add(
                entry := ApiEntryFunction(
                    entry_name,
                    entry_dict["type"],
                    entry_dict["params"],
                )
            )
        elif entry_class == ApiEntryVariable.csv_type:
            self.sdk.variables.add(
                entry := ApiEntryVariable(entry_name, entry_dict["type"])
            )
        else:
            print(entry_dict)
            raise Exception("Unknown entry type: %s" % entry_class)

        if entry is None:
            return

        if entry_status == ApiEntryState.DISABLED.value:
            self.disabled_entries.add(entry)
        elif entry_status == ApiEntryState.PENDING.value:
            self.new_entries.add(entry)

    def load_cache(self) -> None:
        if not os.path.exists(self.cache_file_name):
            raise Exception(
                f"Cannot load symbol cache '{self.cache_file_name}'! File does not exist"
            )

        with open(self.cache_file_name, "rt") as f:
            reader = csv.DictReader(f)
            for row in reader:
                self._process_entry(row)
                if self._load_version_only and row.get("entry") == SdkVersion.csv_type:
                    break

    def _have_pending_entries(self) -> bool:
        return any(
            filter(
                lambda e: not isinstance(e, SdkVersion),
                self.new_entries,
            )
        )

    def sync_sets(
        self, known_set: Set[Any], new_set: Set[Any], update_version: bool = True
    ):
        new_entries = new_set - known_set
        if new_entries:
            print(f"New: {new_entries}")
            known_set |= new_entries
            self.new_entries |= new_entries
            if update_version and self.version_action == VersionBump.NONE:
                self.version_action = VersionBump.MINOR
        removed_entries = known_set - new_set
        if removed_entries:
            print(f"Removed: {removed_entries}")
            known_set -= removed_entries
            # If any of removed entries was a part of active API, that's a major bump
            if update_version and any(
                filter(
                    lambda e: e not in self.disabled_entries
                    and e not in self.new_entries,
                    removed_entries,
                )
            ):
                self.version_action = VersionBump.MAJOR
            self.disabled_entries -= removed_entries
            self.new_entries -= removed_entries

    def validate_api(self, api: ApiEntries) -> None:
        self.sync_sets(self.sdk.headers, api.headers, False)
        self.sync_sets(self.sdk.functions, api.functions)
        self.sync_sets(self.sdk.variables, api.variables)
