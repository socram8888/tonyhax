#!/usr/bin/env python3

import struct
import sys

assert(len(sys.argv) == 2)

def find_romdir(f):
	while True:
		line = f.read(16)
		if len(line) != 16:
			return False

		if line[0:10] == b'RESET\0\0\0\0\0':
			# Unread reset entry
			f.seek(-16, 1)
			return True

def print_romdir(f):
	address = 0xBFC00000

	while True:
		line = f.read(16)
		if len(line) != 16:
			return

		entry = struct.unpack('10sHI', line)

		entry_name = entry[0].rstrip(b'\0').decode('ascii')
		if len(entry_name) == 0:
			return
		print('%10s: %08X' % (entry_name, address))

		address = (address + entry[2] + 0xF) & 0xFFFFFFF0

with open(sys.argv[1], 'rb') as f:
	if find_romdir(f):
		print('ROMDIR found at %08X' % f.tell())
		print_romdir(f)
	else:
		print('ROMDIR not found', file=sys.stderr)
