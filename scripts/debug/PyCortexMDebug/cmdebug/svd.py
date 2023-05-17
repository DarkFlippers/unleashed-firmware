#!/usr/bin/env python
"""
This file is part of PyCortexMDebug

PyCortexMDebug is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

PyCortexMDebug is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with PyCortexMDebug.  If not, see <http://www.gnu.org/licenses/>.
"""

import lxml.objectify as objectify
import sys
from collections import OrderedDict
import os
import pickle
import traceback
import re
import warnings


class SmartDict:
    """
    Dictionary for search by case-insensitive lookup and/or prefix lookup
    """

    def __init__(self):
        self.od = OrderedDict()
        self.casemap = {}

    def __getitem__(self, key):
        if key in self.od:
            return self.od[key]

        if key.lower() in self.casemap:
            return self.od[self.casemap[key.lower()]]

        return self.od[self.prefix_match(key)]

    def is_ambiguous(self, key):
        return (
            key not in self.od
            and key not in self.casemap
            and len(list(self.prefix_match_iter(key))) > 1
        )

    def prefix_match_iter(self, key):
        name, number = re.match(r"^(.*?)([0-9]*)$", key.lower()).groups()
        for entry, od_key in self.casemap.items():
            if entry.startswith(name) and entry.endswith(number):
                yield od_key

    def prefix_match(self, key):
        for od_key in self.prefix_match_iter(key):
            return od_key
        return None

    def __setitem__(self, key, value):
        if key in self.od:
            warnings.warn("Duplicate entry %s", key)
        elif key.lower() in self.casemap:
            warnings.warn(
                "Entry %s differs from duplicate %s only in cAsE",
                key,
                self.casemap[key.lower()],
            )

        self.casemap[key.lower()] = key
        self.od[key] = value

    def __delitem__(self, key):
        if (
            self.casemap[key.lower()] == key
        ):  # Check that we did not overwrite this entry
            del self.casemap[key.lower()]
        del self.od[key]

    def __contains__(self, key):
        return key in self.od or key.lower() in self.casemap or self.prefix_match(key)

    def __iter__(self):
        return iter(self.od)

    def __len__(self):
        return len(self.od)

    def items(self):
        return self.od.items()

    def keys(self):
        return self.od.keys()

    def values(self):
        return self.od.values()

    def __str__(self):
        return str(self.od)


class SVDNonFatalError(Exception):
    """Exception class for non-fatal errors
    So far, these have related to quirks in some vendor SVD files which are reasonable to ignore
    """

    def __init__(self, m):
        self.m = m
        self.exc_info = sys.exc_info()

    def __str__(self):
        s = "Non-fatal: {}".format(self.m)
        s += "\n" + str("".join(traceback.format_exc())).strip()
        return s


class SVDFile:
    """
    A parsed SVD file
    """

    def __init__(self, fname):
        """

        Args:
            fname: Filename for the SVD file
        """
        f = objectify.parse(os.path.expanduser(fname))
        root = f.getroot()
        periph = root.peripherals.getchildren()
        self.peripherals = SmartDict()
        self.base_address = 0

        # XML elements
        for p in periph:
            try:
                if p.tag == "peripheral":
                    self.peripherals[str(p.name)] = SVDPeripheral(p, self)
                else:
                    # This is some other tag
                    pass
            except SVDNonFatalError as e:
                print(e)


def add_register(parent, node):
    """
    Add a register node to a peripheral

    Args:
        parent: Parent SVDPeripheral object
        node: XML file node fot of the register
    """

    if hasattr(node, "dim"):
        dim = int(str(node.dim), 0)
        # dimension is not used, number of split indexes should be same
        incr = int(str(node.dimIncrement), 0)
        default_dim_index = ",".join((str(i) for i in range(dim)))
        dim_index = str(getattr(node, "dimIndex", default_dim_index))
        indices = dim_index.split(",")
        offset = 0
        for i in indices:
            name = str(node.name) % i
            reg = SVDPeripheralRegister(node, parent)
            reg.name = name
            reg.offset += offset
            parent.registers[name] = reg
            offset += incr
    else:
        try:
            reg = SVDPeripheralRegister(node, parent)
            name = str(node.name)
            if name not in parent.registers:
                parent.registers[name] = reg
            else:
                if hasattr(node, "alternateGroup"):
                    print("Register %s has an alternate group", name)
        except SVDNonFatalError as e:
            print(e)


def add_cluster(parent, node):
    """
    Add a register cluster to a peripheral
    """
    if hasattr(node, "dim"):
        dim = int(str(node.dim), 0)
        # dimension is not used, number of split indices should be same
        incr = int(str(node.dimIncrement), 0)
        default_dim_index = ",".join((str(i) for i in range(dim)))
        dim_index = str(getattr(node, "dimIndex", default_dim_index))
        indices = dim_index.split(",")
        offset = 0
        for i in indices:
            name = str(node.name) % i
            cluster = SVDRegisterCluster(node, parent)
            cluster.name = name
            cluster.address_offset += offset
            cluster.base_address += offset
            parent.clusters[name] = cluster
            offset += incr
    else:
        try:
            parent.clusters[str(node.name)] = SVDRegisterCluster(node, parent)
        except SVDNonFatalError as e:
            print(e)


class SVDRegisterCluster:
    """
    Register cluster
    """

    def __init__(self, svd_elem, parent):
        """

        Args:
            svd_elem: XML element for the register cluster
            parent: Parent SVDPeripheral object
        """
        self.parent_base_address = parent.base_address
        self.parent_name = parent.name
        self.address_offset = int(str(svd_elem.addressOffset), 0)
        self.base_address = self.address_offset + self.parent_base_address
        # This doesn't inherit registers from anything
        children = svd_elem.getchildren()
        self.description = str(svd_elem.description)
        self.name = str(svd_elem.name)
        self.registers = SmartDict()
        self.clusters = SmartDict()
        for r in children:
            if r.tag == "register":
                add_register(self, r)

    def refactor_parent(self, parent):
        self.parent_base_address = parent.base_address
        self.parent_name = parent.name
        self.base_address = self.parent_base_address + self.address_offset
        values = self.registers.values()
        for r in values:
            r.refactor_parent(self)

    def __str__(self):
        return str(self.name)


class SVDPeripheral:
    """
    This is a peripheral as defined in the SVD file
    """

    def __init__(self, svd_elem, parent):
        """

        Args:
            svd_elem: XML element for the peripheral
            parent: Parent SVDFile object
        """
        self.parent_base_address = parent.base_address

        # Look for a base address, as it is required
        if not hasattr(svd_elem, "baseAddress"):
            raise SVDNonFatalError("Periph without base address")
        self.base_address = int(str(svd_elem.baseAddress), 0)
        if "derivedFrom" in svd_elem.attrib:
            derived_from = svd_elem.attrib["derivedFrom"]
            try:
                self.name = str(svd_elem.name)
            except AttributeError:
                self.name = parent.peripherals[derived_from].name
            try:
                self.description = str(svd_elem.description)
            except AttributeError:
                self.description = parent.peripherals[derived_from].description

            # pickle is faster than deepcopy by up to 50% on svd files with a
            # lot of derivedFrom definitions
            def copier(a):
                return pickle.loads(pickle.dumps(a))

            self.registers = copier(parent.peripherals[derived_from].registers)
            self.clusters = copier(parent.peripherals[derived_from].clusters)
            self.refactor_parent(parent)
        else:
            # This doesn't inherit registers from anything
            self.description = str(svd_elem.description)
            self.name = str(svd_elem.name)
            self.registers = SmartDict()
            self.clusters = SmartDict()

            if hasattr(svd_elem, "registers"):
                registers = [
                    r
                    for r in svd_elem.registers.getchildren()
                    if r.tag in ["cluster", "register"]
                ]
                for r in registers:
                    if r.tag == "cluster":
                        add_cluster(self, r)
                    elif r.tag == "register":
                        add_register(self, r)

    def refactor_parent(self, parent):
        self.parent_base_address = parent.base_address
        values = self.registers.values()
        for r in values:
            r.refactor_parent(self)

        for c in self.clusters.values():
            c.refactor_parent(self)

    def __str__(self):
        return str(self.name)


class SVDPeripheralRegister:
    """
    A register within a peripheral
    """

    def __init__(self, svd_elem, parent):
        self.parent_base_address = parent.base_address
        self.name = str(svd_elem.name)
        self.description = str(svd_elem.description)
        self.offset = int(str(svd_elem.addressOffset), 0)
        if hasattr(svd_elem, "access"):
            self.access = str(svd_elem.access)
        else:
            self.access = "read-write"
        if hasattr(svd_elem, "size"):
            self.size = int(str(svd_elem.size), 0)
        else:
            self.size = 0x20
        self.fields = SmartDict()
        if hasattr(svd_elem, "fields"):
            # Filter fields to only consider those of tag "field"
            fields = [f for f in svd_elem.fields.getchildren() if f.tag == "field"]
            for f in fields:
                self.fields[str(f.name)] = SVDPeripheralRegisterField(f, self)

    def refactor_parent(self, parent):
        self.parent_base_address = parent.base_address

    def address(self):
        return self.parent_base_address + self.offset

    def readable(self):
        return self.access in ["read-only", "read-write", "read-writeOnce"]

    def writable(self):
        return self.access in [
            "write-only",
            "read-write",
            "writeOnce",
            "read-writeOnce",
        ]

    def __str__(self):
        return str(self.name)


class SVDPeripheralRegisterField:
    """
    Field within a register
    """

    def __init__(self, svd_elem, parent):
        self.name = str(svd_elem.name)
        self.description = str(getattr(svd_elem, "description", ""))

        # Try to extract a bit range (offset and width) from the available fields
        if hasattr(svd_elem, "bitOffset") and hasattr(svd_elem, "bitWidth"):
            self.offset = int(str(svd_elem.bitOffset))
            self.width = int(str(svd_elem.bitWidth))
        elif hasattr(svd_elem, "bitRange"):
            bitrange = list(map(int, str(svd_elem.bitRange).strip()[1:-1].split(":")))
            self.offset = bitrange[1]
            self.width = 1 + bitrange[0] - bitrange[1]
        else:
            assert hasattr(svd_elem, "lsb") and hasattr(
                svd_elem, "msb"
            ), "Range not found for field {} in register {}".format(self.name, parent)
            lsb = int(str(svd_elem.lsb))
            msb = int(str(svd_elem.msb))
            self.offset = lsb
            self.width = 1 + msb - lsb

        self.access = str(getattr(svd_elem, "access", parent.access))
        self.enum = {}

        if hasattr(svd_elem, "enumeratedValues"):
            values = [
                v
                for v in svd_elem.enumeratedValues.getchildren()
                if v.tag == "enumeratedValue"
            ]
            for v in values:
                # Skip the "name" tag and any entries that don't have a value
                if v.tag == "name" or not hasattr(v, "value"):
                    continue
                # Some Kinetis parts have values with # instead of 0x...
                value = str(v.value).replace("#", "0x")
                description = str(v.description) if hasattr(v, "description") else ""
                try:
                    index = int(value, 0)
                    self.enum[int(value, 0)] = (str(v.name), description)
                except ValueError:
                    # If the value couldn't be converted as a single integer, skip it
                    pass

    def readable(self):
        return self.access in ["read-only", "read-write", "read-writeOnce"]

    def writable(self):
        return self.access in [
            "write-only",
            "read-write",
            "writeOnce",
            "read-writeOnce",
        ]

    def __str__(self):
        return str(self.name)


def _main():
    """
    Basic test to parse a file and do some things
    """

    for f in sys.argv[1:]:
        print("Testing file: {}".format(f))
        svd = SVDFile(f)
        print(svd.peripherals)
        key = list(svd.peripherals)[0]
        print("Registers in peripheral '{}':".format(key))
        print(svd.peripherals[key].registers)
        print("Done testing file: {}".format(f))


if __name__ == "__main__":
    _main()
