MEMORY {
	ram(wrx)	:ORIGIN = 0x801FC100, LENGTH = 0x1F00
}
SECTIONS {
	.text :
	{
		*(.start)
		*(.text .text.*)
	} >ram
	.rodata :
	{
		*(.rodata .rodata.*)
	} >ram
	.data :
	{
		*(.data .data.*)
	} >ram
	.bss :
	{
		*(.bss .bss.*)
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
