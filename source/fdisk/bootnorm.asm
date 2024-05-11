;
; MBR boot code installed by Free FDISK
; to assemble use Netwide Assembler (NASM)
;
;-----------------------------------------------------------------------
;   ENTRY (copied from freedos bootsector)
;
; IN: DL = boot drive
;OUT: DL = boot drive
;
;-----------------------------------------------------------------------

	RELOC_SEG	 equ 0x80
	PARTTBL_OFFSET	 equ 0x1be
	SIGNATURE_OFFSET equ 0x1fe


org 0x7c00

start:     
	cli
	xor ax, ax
	mov ss, ax		; initialize stack
	mov sp, start - 0x20	; SS:SP = 0:7be0
	sti
	cld
	mov bp, start
	mov ds, ax		; DS = 0
	  ; move MBR code out of the way to make room for volume boot record
relocate:	
	mov ax, RELOC_SEG
	mov es, ax
	mov si, bp
	mov di, bp
	mov cx, 0x0100
	rep movsw
	jmp word RELOC_SEG:.cs_changed
	  ; From now on, the code is located 0x800 bytes above 0x7c00
	  ; at absolute address 0x8400, or more accurate 80:7c000.
.cs_changed:
	mov ds, ax		; DS = RELOC_SEG
	xor ax, ax
	mov es, ax		; ES = 0

fix_bootdrive:
	test dl, dl		; is boot drive in DL given by BIOS zero?
	jnz .dl_good		; if yes this can be considered a BIOS bug,
	mov dl, 0x80		; because we know we boot from HD		
.dl_good:

	  ; Test if one of the four primary partitions is active by checkung
	  ; for value 0x80 in the first byte of the entries.
scan_for_active_partition:
	lea di, [bp + PARTTBL_OFFSET]		; start of partition table
  .l:	test byte [di], 0x80
	jnz chainload_bootsect
	add di, 0x10                    	; next table entry
	cmp di, start + SIGNATURE_OFFSET	; scanned beyond end of table?
	jb .l
  .no_active:
	call print
	db 'no active partition found', 0
	jmp $

	  ; We found an active partition. Load its boot sector to 0:7c00,
	  ; test the signature and far jump to 0:7c00
chainload_bootsect:
	call read_boot_sector                    	
	jc .read_error		
	cmp word [es:start + SIGNATURE_OFFSET], 0xaa55
	jne .invalid_partition_code	
;-----------------------------------------------------------------------------
	jmp word 0x0:0x7c00        	; jump to volume boot code
;-----------------------------------------------------------------------------
  .read_error:
	call print
	db 'read error while reading drive', 0
	jmp $
  .invalid_partition_code:
	call print
	db 'partition signature != 55AA', 0	
	jmp $


;-----------------------------------------------------------------------------
; BIOS disk access packet used by ext. INT13 LBA read function

dap:
  .packet_size	db 0x10
  .reserved1	db 0
  .sector_count db 1
  .reserved2	db 0
  .buf_off	dw 0x7c00
  .buf_seg	dw 0x0000
  .lba_low  	dw 0
  .lba_high 	dw 0
		dw 0,0


;-----------------------------------------------------------------------------
; read_boot_sector				
;
; IN : DI--> partition info
; OUT: CARRY

read_boot_sector:
  .check_lba:
	mov bx, 0x55aa		; magic value changed after call
	mov ah, 0x41
	int 0x13		; test for ext. INT13 LBA support
	jc  .read_chs		; no support if carry set
	cmp bx, 0xaa55		; no support if 55aa not swapped      
	jne .read_chs
	test cl, 1		; no support if LBA flag not set
	jz .read_chs
  .read_lba:
	mov ax, [di + 8]	; copy start sector of partition to DAP
	mov [dap.lba_low], ax
	mov ax, [di + 10]
	mov [dap.lba_high], ax
	mov ax, 0x4200
	mov si, dap
	int 0x13
	ret
  .read_chs:
	mov ax, 0x0201		; read one sector
	mov bx, 0x7c00		; to 0:7c00
	mov cx, [di + 2]
	mov dh, [di + 1]
	int 0x13
	ret	


;-----------------------------------------------------------------------------
; prints text after call to this function.

print_1char:        
	xor bx, bx               ; video page 0
	mov ah, 0x0E             ; else print it
	int 0x10                 ; via TTY mode
print:	pop si                   ; this is the first character
	lodsb                    ; get token
	push si                  ; stack up potential return address
	cmp al, 0                ; end of string?
	jne print_1char          ; until done
	ret                      ; and jump to it


; zero-fill rest of sector and put signature into place
	times	0x1fe - $ + $$ db 0
	db 0x55, 0xaa
