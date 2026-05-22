ENTRY(main)

MEMORY
{
	RAM (rxw) : ORIGIN = 0x10000, LENGTH = 64K
}

SECTIONS
{
	. = ORIGIN(RAM);
	.text :
	{
		*(.text)
		*(.text.*)
	} > RAM

	.rodata :
	{
		*(.rodata)
		*(.rodata.*)
	} > RAM

	.data :
	{
		*(.data)
		*(.data.*)
	} > RAM

	.bss ALIGN(0x1000) :
	{
		__bss_start = .;
		*(.bss)
		*(.bss.*)
		__bss_end = .;
	} > RAM
}
