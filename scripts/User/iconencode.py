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
parser.add_argument('Width', metavar='W', type=int, nargs="?", default="128",
                    help='Width of the image. Find from meta.txt or directory name')
parser.add_argument('Height', metavar='H', type=int, nargs="?",  default="64",
                    help='Height of the image. Find from meta.txt or directory name')
args = vars(parser.parse_args())

r = open(args["infile"],"r")
infile=args["infile"].split(".")[0]

imageWidth=args["Width"]
imageHeight=args["Height"]
dims=str(imageWidth)+"x"+str(imageHeight)

output = subprocess.check_output(["cat", args["infile"]]) #yes this is terrible.
f = io.StringIO(output.decode().strip())

data = f.read().strip().replace(";","").replace("{","").replace("}","")
data_str = data.replace(",", "").replace("0x", "")
data_bin = bytearray.fromhex(data_str)

data_encoded_str = subprocess.check_output(
    ["heatshrink", "-e","-w8","-l4"], input=data_bin
)

b=list(data_encoded_str)

c=','.join(padded_hex(my_int,2) for my_int in b)

# a bit ugly.

framename="_I_"+infile+"_"+dims
print(len(b))
#d=len(b)
# if b > 255 split 0x1234 into 0x34,0x12
#d=hex(len(b))

char_out = "const uint8_t "+framename+"_0[] = {"+  str(c) +  ",};"
char_out2 = "const uint8_t "+framename+"[] = {"+framename+"_0};"
#data=bytes_out
print(char_out)
print(char_out2)
#w.write(data)
#w.close()
