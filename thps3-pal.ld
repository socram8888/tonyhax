MEMORY {
	ram(wrx)	:ORIGIN = 0x800BC9C0, LENGTH = 0x4B0
}
SECTIONS {
	.text : {
		*(.test)
	} > ram
}
ENTRY (start)
