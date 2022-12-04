import logging
import argparse
import subprocess
import io
import os
import sys

def padded_hex(i, l):
    given_int = i
    given_len = l

    hex_result = hex(given_int)[2:] # remove '0x' from beginning of str
    num_hex_chars = len(hex_result)
    extra_zeros = '0' * (given_len - num_hex_chars) # may not get used..

    return ('0x' + hex_result if num_hex_chars == given_len else
            '?' * given_len if num_hex_chars > given_len else
            '0x' + extra_zeros + hex_result if num_hex_chars < given_len else
            None)


parser = argparse.ArgumentParser(description='Turn icon char arrays back into .xbm')

parser.add_argument('infile', metavar='i',
                    help='Input file')
parser.add_argument('outfile', metavar='o',
                    help='File to write to')
parser.add_argument('Width', metavar='W', type=int, nargs="?", default="128",
                    help='Width of the image. Find from meta.txt or directory name')
parser.add_argument('Height', metavar='H', type=int, nargs="?",  default="64",
                    help='Height of the image. Find from meta.txt or directory name')
parser.add_argument('Trim', metavar='T', type=int, nargs="?",  default="8",
                    help='Number of bytes off the start/header to trim. Multiples of 2 required.')
args = vars(parser.parse_args())

r = open(args["infile"],"r")
w = open(args["outfile"],"w")
imageWidth=args["Width"]
imageHeight=args["Height"]
trimStart=args["Trim"]

output = subprocess.check_output(["cat", args["infile"]]) #yes this is terrible.
f = io.StringIO(output.decode().strip())

data = f.read().strip().replace(";","").replace("{","").replace("}","")
data_str = data.replace(",", "").replace("0x", "")
data_bin = bytearray.fromhex(data_str[trimStart:])

data_decoded_str = subprocess.check_output(
    ["heatshrink", "-d","-w8","-l4"], input=data_bin
)

b=list(data_decoded_str)

c=', '.join(padded_hex(my_int,2) for my_int in b)

width_out = "#define icon_width "+ str(imageWidth) + "\n"
height_out = "#define icon_height "+ str(imageHeight) + "\n"
bytes_out = "static unsigned char icon_bits[] = {"+  str(c) +  "};"

data=width_out+height_out+bytes_out

w.write(data)
r.close()
w.close()
