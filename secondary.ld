MEMORY {
	ram(wrx)	:ORIGIN = 0x801FC100, LENGTH = 0x1F00
}
SECTIONS {
	. = 0x801FC100;
	PROVIDE(__ROM_START__ = .);
	.text :
	{
		*(.start)
		*(.text .text.*)
	} >ram
	.rodata :
	{
		*(.rodata .rodata.*)
	} >ram
	PROVIDE(__ROM_END__ = .);
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
