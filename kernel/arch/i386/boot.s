.intel_syntax noprefix

# Declare constants for the multiboot header.
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot

.set VIRT_ADDRESS_BASE_BOOT, 0xC0000000

# Declare a header as in the Multiboot Standard.
.section .multiboot.data, "aw"
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# Reserve a stack for the initial thread.
.globl stack_top, stack_bottom
.section .bootstrap_stack, "aw", @nobits
.align 16
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

# Preallocate pages used for paging. Don't hard-code addresses and assume they
# are available, as the bootloader might have loaded its multiboot structures or
# modules there. This lets the bootloader know it must avoid the addresses.
.globl page_directory, page_table1, scratch_space
.section .bss, "aw", @nobits
.align 4096
page_directory:
	.skip 4096
page_table1:
	.skip 4096
scratch_space:
	.skip 4096
	.skip 4096
# The kernel entry point.
.section .multiboot.text, "a"
.global _start
.type _start, @function
_start:
	# Physical address of page_table1.
	lea edi, [page_table1 - VIRT_ADDRESS_BASE_BOOT]
	xor esi, esi
	mov ecx, 1023

1:
	# Only map the kernel.
	cmp esi, OFFSET _kernel_start
	jl 2f
	cmp esi, OFFSET _kernel_end - VIRT_ADDRESS_BASE_BOOT
	jge 3f

	# Map physical address as "present, writable". Note that this maps
	# .text and .rodata as writable. Mind security and map them as non-writable.
	mov edx, esi
	or edx, 0x003
	mov [edi], edx

2:
	# Size of page is 4096 bytes.
	add esi, 4096
	# Size of entries in page_table1 is 4 bytes.
	add edi, 4
	# Loop to the next entry if we haven't finished.
	loop 1b

3:
	# Map VGA video memory to 0xC03FF000 as "present, writable".
	mov DWORD PTR page_table1 - VIRT_ADDRESS_BASE_BOOT + 1023 * 4, 0x000B8000 | 0x003

	# The page table is used at both page directory entry 0 (virtually from 0x0
	# to 0x3FFFFF) (thus identity mapping the kernel) and page directory entry
	# 768 (virtually from 0xC0000000 to 0xC03FFFFF) (thus mapping it in the
	# higher half). The kernel is identity mapped because enabling paging does
	# not change the next instruction, which continues to be physical. The CPU
	# would instead page fault if there was no identity mapping.

	# Map the page table to both virtual addresses 0x00000000 and 0xC0000000.
	mov DWORD PTR page_directory - VIRT_ADDRESS_BASE_BOOT + 0,       OFFSET page_table1 - VIRT_ADDRESS_BASE_BOOT + 0x003
	mov DWORD PTR page_directory - VIRT_ADDRESS_BASE_BOOT + 768 * 4, OFFSET page_table1 - VIRT_ADDRESS_BASE_BOOT + 0x003

	# Set cr3 to the address of the page_directory.
	lea ecx, [page_directory - VIRT_ADDRESS_BASE_BOOT]
	mov cr3, ecx

	# Enable paging and the write-protect bit.
	mov ecx, cr0
	or ecx, 0x80010000
	mov cr0, ecx

	# Jump to higher half with an absolute jump.
	lea ecx, 4f
	jmp ecx

.section .text

4:
	# Unmap the identity mapping as it is now unnecessary.
	mov DWORD PTR page_directory + 0, 0

	# Reload crc3 to force a TLB flush so the changes to take effect.
	mov ecx, cr3
	mov cr3, ecx

	# Set up the stack.
	lea esp, [stack_top]

	# Setup boot_info
	push eax
	push ebx

	# Call the global constructors.
	call _init

	# Transfer control to the main kernel.
	call kernel_main

	# Hang if kernel_main unexpectedly returns.
	cli
1:	hlt
	jmp 1b

.globl VIRT_ADDRESS_BASE
VIRT_ADDRESS_BASE:
  .long VIRT_ADDRESS_BASE_BOOT
