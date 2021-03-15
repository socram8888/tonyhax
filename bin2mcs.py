#!/usr/bin/env python3

import sys
import struct
import os

assert(len(sys.argv) == 2)

binfile = sys.argv[1]
mcsfile = sys.argv[1] + '.mcs'
with open(binfile, 'rb') as f:
	memcard = f.read()

if len(memcard) % 8192 != 0:
	print('Size is not aligned to a 8K boundary. Aborting.')
	sys.exit(1)

# From https://problemkaputt.de/psx-spx.htm#memorycarddataformat

header = bytearray(128)
nextblock = 2 if len(memcard) > 8192 else 0xFFFF
filename = os.path.basename(binfile)

struct.pack_into('<IIH21s96x', header, 0, 0x00000051, len(memcard), nextblock, filename.encode('ascii'))

checkxor = 0
for b in header[:127]:
	checkxor ^= b
header[127] = checkxor

with open(mcsfile, 'wb') as f:
	f.write(header)
	f.write(memcard)
