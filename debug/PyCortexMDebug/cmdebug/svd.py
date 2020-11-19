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
from copy import deepcopy
from collections import OrderedDict
import os
import traceback


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
    def __init__(self, fname):
        f = objectify.parse(os.path.expanduser(fname))
        root = f.getroot()
        periph = root.peripherals.getchildren()
        self.peripherals = OrderedDict()
        # XML elements
        for p in periph:
            try:
                self.peripherals[str(p.name)] = SVDPeripheral(p, self)
            except SVDNonFatalError as e:
                print(e)


def add_register(parent, node):
    if hasattr(node, "dim"):
        dim = int(str(node.dim), 0)
        # dimension is not used, number of split indexes should be same
        incr = int(str(node.dimIncrement), 0)
        default_dim_index = ",".join((str(i) for i in range(dim)))
        dim_index = str(getattr(node, "dimIndex", default_dim_index))
        indexes = dim_index.split(",")
        offset = 0
        for i in indexes:
            name = str(node.name) % i
            reg = SVDPeripheralRegister(node, parent)
            reg.name = name
            reg.offset += offset
            parent.registers[name] = reg
            offset += incr
    else:
        try:
            parent.registers[str(node.name)] = SVDPeripheralRegister(node, parent)
        except:
            pass


def add_cluster(parent, node):
    if hasattr(node, "dim"):
        dim = int(str(node.dim), 0)
        # dimension is not used, number of split indexes should be same
        incr = int(str(node.dimIncrement), 0)
        default_dim_index = ",".join((str(i) for i in range(dim)))
        dim_index = str(getattr(node, "dimIndex", default_dim_index))
        indexes = dim_index.split(",")
        offset = 0
        for i in indexes:
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
    def __init__(self, svd_elem, parent):
        self.parent = parent
        self.address_offset = int(str(svd_elem.addressOffset), 0)
        self.base_address = self.address_offset + parent.base_address
        # This doesn't inherit registers from anything
        children = svd_elem.getchildren()
        self.description = str(svd_elem.description)
        self.name = str(svd_elem.name)
        self.registers = OrderedDict()
        self.clusters = OrderedDict()
        for r in children:
            if r.tag == "register":
                add_register(self, r)

    def refactor_parent(self, parent):
        self.parent = parent
        self.base_address = parent.base_address + self.address_offset
        try:
            values = self.registers.itervalues()
        except AttributeError:
            values = self.registers.values()
        for r in values:
            r.refactor_parent(self)

    def __unicode__(self):
        return str(self.name)


class SVDPeripheral:
    def __init__(self, svd_elem, parent):
        self.parent = parent
        if not hasattr(svd_elem, "baseAddress"):
            raise SVDNonFatalError("Periph without base address")
        self.base_address = int(str(svd_elem.baseAddress), 0)
        if "derivedFrom" in svd_elem.attrib:
            derived_from = svd_elem.attrib["derivedFrom"]
            try:
                self.name = str(svd_elem.name)
            except:
                self.name = parent.peripherals[derived_from].name
            try:
                self.description = str(svd_elem.description)
            except:
                self.description = parent.peripherals[derived_from].description
            self.registers = deepcopy(parent.peripherals[derived_from].registers)
            self.clusters = deepcopy(parent.peripherals[derived_from].clusters)
            self.refactor_parent(parent)
        else:
            # This doesn't inherit registers from anything
            registers = svd_elem.registers.getchildren()
            self.description = str(svd_elem.description)
            self.name = str(svd_elem.name)
            self.registers = OrderedDict()
            self.clusters = OrderedDict()
            for r in registers:
                if r.tag == "cluster":
                    add_cluster(self, r)
                else:
                    add_register(self, r)

    def refactor_parent(self, parent):
        self.parent = parent
        try:
            values = self.registers.itervalues()
        except AttributeError:
            values = self.registers.values()
        for r in values:
            r.refactor_parent(self)
        try:
            for c in self.clusters.itervalues():
                c.refactor_parent(self)
        except AttributeError:
            for c in self.clusters.values():
                c.refactor_parent(self)

    def __unicode__(self):
        return str(self.name)


class SVDPeripheralRegister:
    def __init__(self, svd_elem, parent):
        self.parent = parent
        self.name = str(svd_elem.name)
        self.description = str(svd_elem.description)
        self.offset = int(str(svd_elem.addressOffset), 0)
        try:
            self.access = str(svd_elem.access)
        except:
            self.access = "read-write"
        try:
            self.size = int(str(svd_elem.size), 0)
        except:
            self.size = 0x20
        self.fields = OrderedDict()
        if hasattr(svd_elem, "fields"):
            fields = svd_elem.fields.getchildren()
            for f in fields:
                self.fields[str(f.name)] = SVDPeripheralRegisterField(f, self)

    def refactor_parent(self, parent):
        self.parent = parent
        try:
            fields = self.fields.itervalues()
        except AttributeError:
            fields = self.fields.values()
        for f in fields:
            f.refactor_parent(self)

    def address(self):
        return self.parent.base_address + self.offset

    def readable(self):
        return self.access in ["read-only", "read-write", "read-writeOnce"]

    def writable(self):
        return self.access in [
            "write-only",
            "read-write",
            "writeOnce",
            "read-writeOnce",
        ]

    def __unicode__(self):
        return str(self.name)


class SVDPeripheralRegisterField:
    def __init__(self, svd_elem, parent):
        self.parent = parent
        self.name = str(svd_elem.name)
        self.description = str(getattr(svd_elem, "description", ""))

        try:
            self.offset = int(str(svd_elem.bitOffset))
            self.width = int(str(svd_elem.bitWidth))
        except:
            try:
                bitrange = list(
                    map(int, str(svd_elem.bitRange).strip()[1:-1].split(":"))
                )
                self.offset = bitrange[1]
                self.width = 1 + bitrange[0] - bitrange[1]
            except:
                lsb = int(str(svd_elem.lsb))
                msb = int(str(svd_elem.msb))
                self.offset = lsb
                self.width = 1 + msb - lsb
        self.access = str(getattr(svd_elem, "access", parent.access))
        self.enum = {}

        if hasattr(svd_elem, "enumeratedValues"):
            for v in svd_elem.enumeratedValues.getchildren():
                if v.tag == "name":
                    continue
                # Some Kinetis parts have values with # instead of 0x...
                value = str(v.value).replace("#", "0x")
                self.enum[int(value, 0)] = (str(v.name), str(v.description))

    def refactor_parent(self, parent):
        self.parent = parent

    def readable(self):
        return self.access in ["read-only", "read-write", "read-writeOnce"]

    def writable(self):
        return self.access in [
            "write-only",
            "read-write",
            "writeOnce",
            "read-writeOnce",
        ]

    def __unicode__(self):
        return str(self.name)


if __name__ == "__main__":
    for f in sys.argv[1:]:
        print("Testing file: {}".format(f))
        svd = SVDFile(f)
        print(svd.peripherals)
        key = list(svd.peripherals)[0]
        print("Registers in peripheral '{}':".format(key))
        print(svd.peripherals[key].registers)
        print("Done testing file: {}".format(f))
