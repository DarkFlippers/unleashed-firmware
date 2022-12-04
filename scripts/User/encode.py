import logging
import argparse
import subprocess
import io
import os
import sys


parser = argparse.ArgumentParser(description='Turn .xbm files into cooked .bm files for flipper FS')

parser.add_argument('infile', metavar='i',
                    help='Input file')
parser.add_argument('outfile', metavar='o',
                    help='File to write to')

args = vars(parser.parse_args())

r = open(args["infile"],"r")
w = open(args["outfile"],"wb")


output = subprocess.check_output(["cat", args["infile"]])
f = io.StringIO(output.decode().strip())
print("Image Dimensions:")
width = int(f.readline().strip().split(" ")[2])
print("W: ", width)
height = int(f.readline().strip().split(" ")[2])
print("H: ", height)


data = f.read().strip().replace("\n", "").replace(" ", "").split("=")[1][:-1]
data_str = data[1:-1].replace(",", " ").replace("0x", "")

data_bin = bytearray.fromhex(data_str)
data_encoded_str = subprocess.check_output(
    ["heatshrink", "-e", "-w8", "-l4"], input=data_bin
)

assert data_encoded_str

data_enc = bytearray(data_encoded_str)
data_enc = bytearray([len(data_enc) & 0xFF, len(data_enc) >> 8]) + data_enc
if len(data_enc) < len(data_bin) + 1:
    data = b"\x01\x00" + data_enc
else:
    data = b"\x00" + data_bin
w.write(data)
r.close()
w.close()
