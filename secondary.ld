MEMORY {
	ram(wrx)	:ORIGIN = 0x801C000, LENGTH = 0x1F00
}
SECTIONS {
	.text : {
		*(.test)
	} > ram
}
ENTRY (start)
