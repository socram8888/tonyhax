MEMORY {
	ram(wrx)	:ORIGIN = 0x801FC100, LENGTH = 0x1F00
}
SECTIONS {
	.text : {
		*(.test)
	} > ram
}
ENTRY (start)
