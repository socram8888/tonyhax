MEMORY {
	ram(wrx)	:ORIGIN = 0x800BBCA0, LENGTH = 0x4B0
}
SECTIONS {
	.text :
	{
		*(.text)
	} >ram
	/DISCARD/ :
	{
		*(.MIPS.abiflags)
		*(.pdr)
		*(.gnu.attributes)
		*(.comment)
		*(.reginfo)
	}
}
ENTRY (start)
