#!/usr/bin/env python3

import argparse
import xml.etree.ElementTree as XML
import sys


def getArgs():
    parser = argparse.ArgumentParser(
        description="chiplist.xml to C array converter",
    )
    parser.add_argument("file", help="chiplist.xml file")
    return parser.parse_args()


def getXML(file):
    tree = XML.parse(file)
    root = tree.getroot()
    return root


def parseChip(cur, arr, vendor, vendorCodeArr):
    chip = {}
    chipAttr = cur.attrib
    if "page" not in chipAttr:  # chip without page size not supported
        return
    if "id" not in chipAttr:  # I2C not supported yet
        return
    if len(chipAttr["id"]) < 6:  # ID wihout capacity id not supported yet
        return
    chip["modelName"] = cur.tag
    chip["vendorEnum"] = "SPIMemChipVendor" + vendor
    chip["vendorID"] = "0x" + chipAttr["id"][0] + chipAttr["id"][1]
    chip["typeID"] = chipAttr["id"][2] + chipAttr["id"][3]
    chip["capacityID"] = chipAttr["id"][4] + chipAttr["id"][5]
    chip["size"] = chipAttr["size"]
    if chipAttr["page"] == "SSTW":
        chip["writeMode"] = "SPIMemChipWriteModeAAIWord"
        chip["pageSize"] = "1"
    elif chipAttr["page"] == "SSTB":
        chip["writeMode"] = "SPIMemChipWriteModeAAIByte"
        chip["pageSize"] = "1"
    else:
        chip["writeMode"] = "SPIMemChipWriteModePage"
        chip["pageSize"] = chipAttr["page"]
    arr.append(chip)
    vendorCodeArr[vendor].add(chip["vendorID"])


def cleanEmptyVendors(vendors):
    for cur in list(vendors):
        if not vendors[cur]:
            vendors.pop(cur)


def getVendors(xml, interface):
    arr = {}
    for cur in xml.find(interface):
        arr[cur.tag] = set()
    return arr


def parseXML(xml, interface, vendorCodeArr):
    arr = []
    for vendor in xml.find(interface):
        for cur in vendor:
            parseChip(cur, arr, vendor.tag, vendorCodeArr)
    return arr


def getVendorNameEnum(vendorID):
    try:
        return vendors[vendorID]
    except:
        print("Unknown vendor: " + vendorID)
        sys.exit(1)


def generateCArr(arr, filename):
    with open(filename, "w") as out:
        print('#include "spi_mem_chip_i.h"', file=out)
        print("const SPIMemChip SPIMemChips[] = {", file=out)
        for cur in arr:
            print("    {" + cur["vendorID"] + ",", file=out, end="")
            print(" 0x" + cur["typeID"] + ",", file=out, end="")
            print(" 0x" + cur["capacityID"] + ",", file=out, end="")
            print(' "' + cur["modelName"] + '",', file=out, end="")
            print(" " + cur["size"] + ",", file=out, end="")
            print(" " + cur["pageSize"] + ",", file=out, end="")
            print(" " + cur["vendorEnum"] + ",", file=out, end="")
            if cur == arr[-1]:
                print(" " + cur["writeMode"] + "}};", file=out)
            else:
                print(" " + cur["writeMode"] + "},", file=out)

def main():
    filename = "spi_mem_chip_arr.c"
    args = getArgs()
    xml = getXML(args.file)
    vendors = getVendors(xml, "SPI")
    chipArr = parseXML(xml, "SPI", vendors)
    cleanEmptyVendors(vendors)
    for cur in vendors:
        print('    {"' + cur + '", SPIMemChipVendor' + cur + "},")
    generateCArr(chipArr, filename)


if __name__ == "__main__":
    main()
