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

parser = argparse.ArgumentParser(description='Turn cooked Flipper .bm files back into .xbm')

parser.add_argument('infile', metavar='i',
                    help='Input file')
parser.add_argument('outfile', metavar='o',
                    help='File to write to')
parser.add_argument('Width', metavar='W', type=int, nargs="?", default="128",
                    help='Width of the image. Find from meta.txt or directory name')
parser.add_argument('Height', metavar='H', type=int, nargs="?",  default="64",
                    help='Height of the image. Find from meta.txt or directory name')

args = vars(parser.parse_args())

r = open(args["infile"],"rb")
w = open(args["outfile"],"w")

fileStream=r.read()
filename=os.path.splitext(os.path.basename(args["outfile"]))[0]


imageWidth=args["Width"]
imageHeight=args["Height"]


#remove headers and padding 
if(fileStream[0:2] == bytes([0x01,0x00])):
	unpad=fileStream[4:]
else:
	if(fileStream[0:1] == bytes([0x00])):
		unpad=fileStream[2:]



#lzss decompress
data_decoded_str = subprocess.check_output(
    ["heatshrink", "-d","-w8","-l4"], input=unpad
)

#turn it back into xbm

b=list(data_decoded_str)
c=', '.join(padded_hex(my_int,2) for my_int in b)

width_out = "#define "+ filename+ "_width "+ str(imageWidth) + "\n"
height_out = "#define "+ filename+ "_height "+ str(imageHeight) + "\n"
bytes_out = "static unsigned char "+ filename+ "_bits[] = {"+  str(c) +  "};"

data=width_out+height_out+bytes_out

w.write(data)
r.close()
w.close()