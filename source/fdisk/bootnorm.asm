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

	TOP_OF_STACK	 equ 0x7be0 ; right below original boot code
				    ; with 0x20 bytes of safety margin
	BOOTSECT_OFFSET	 equ 0x7c00
	RELOCATED_OFFSET equ 0x0600
	PARTTBL_SIZE	 equ 64
	PARTTBL_OFFSET	 equ 0x1be
	CODE_SIZE	 equ 440

org RELOCATED_OFFSET

start:
	cli
	xor ax, ax
	mov ss, ax		; initialize stack
	mov sp, TOP_OF_STACK	; SS:SP = 0:7be0
	sti
	cld
	mov ds, ax		; do not trust BIOS to have the segment
	mov es, ax		; registers initialized to zero
	  ; move MBR code out of the way to make room for volume boot record
relocate:	
	mov si, BOOTSECT_OFFSET
	mov di, RELOCATED_OFFSET
	mov cx, 0x0100
	rep movsw
	jmp 0:.relocated	; far jump to be sure we get CS right
.relocated:

fix_bootdrive:
	test dl, dl		; is boot drive in DL given by BIOS zero?
	jnz .dl_good		; if yes this can be considered a BIOS bug,
	mov dl, 0x80		; because we know we boot from HD		
  .dl_good:
  	mov [driveno], dl	; save driveno in case BIOS destroys DL

	  ; Test if one of the four primary partitions is active by checking
	  ; for value 0x80 in the first byte of the entries.
scan_for_active_partition:
	mov di, parttbl				; start of partition table
  .l:	test byte [di], 0x80			; is partition active?
	jnz chainload_bootsect
	add di, 0x10                    	; next table entry
	cmp di, signature			; scanned beyond end of table?
	jb .l
  .no_active:
	call fatal				; does not return
	db 'no active partition found', 0

	  ; We found an active partition. Load its boot sector to 0:7c00,
	  ; test the signature and far jump to 0:7c00
chainload_bootsect:
	push di				; save parttbl entry (restore to SI)
	call read_boot_sector		; reads one sector                    	
	jnc .check_signature		; no read error occured?
	call fatal			; does not return
	db 'read error while reading drive', 0		
  .check_signature:
	cmp word [signature], 0xaa55
	je handoff_to_volume_bootrecord
	call fatal				; does not return
	db 'partition signature != 55AA', 0

;-----------------------------------------------------------------------------
handoff_to_volume_bootrecord:
	pop si				; restore parttbl entry to SI, to
					; comply with lDOS boot protocol
	mov dl, [driveno]
	jmp 0:0x7c00        		; far jump to volume boot code
;-----------------------------------------------------------------------------


;-----------------------------------------------------------------------------
; read_boot_sector				
;
; IN : DI--> partition info
; OUT: CARRY

read_boot_sector:
  .check_lba:
  	push ds			; temporarly set DS to 0x40 (BIOS data area)
  	mov ax, 0x40		; to work around a BIOS bug
  	mov ds, ax		; https://github.com/FDOS/kernel/issues/156
	mov bx, 0x55aa		; magic value shoud be changed after call
	mov ah, 0x41		; query INT 13h LBA capabilities
	stc
	int 0x13
	pop ds
	jc  .read_chs		; no support if carry set
	cmp bx, 0xaa55		; no support if 55aa not swapped      
	jne .read_chs
	shr cl, 1		; no support if LBA flag not set
	jnc .read_chs
  .read_lba:
	mov ax, [di + 8]	; copy start sector of partition to DAP
	mov [dap.lba_low], ax
	mov ax, [di + 10]
	mov [dap.lba_high], ax
	mov ah, 0x42		; LBA read function
	mov si, dap
	jmp short .intcall
  .read_chs:
	mov ax, 0x0201		; read one sector
	mov bx, 0x7c00		; to 0:7c00
	mov cx, [di + 2]
	mov dh, [di + 1]
  .intcall:
	stc
	int 0x13
	ret	


;-----------------------------------------------------------------------------
; Fatal error handler. Displays error message and waits forever
; IN: CS:IP = ASCIIZ with error message to print

__fatal_print_char:        
	xor bx, bx               ; video page 0
	mov ah, 0x0E             ; else print it
	int 0x10                 ; via TTY mode
fatal:	pop si                   ; this is the first character
	lodsb                    ; get token
	push si                  ; stack up potential return address
	cmp al, 0                ; end of string?
	jne __fatal_print_char   ; until done
  .wait_forever:
	jmp short .wait_forever


;-----------------------------------------------------------------------------
; Padding bytes, BIOS disk access packet used by ext. INT13 LBA read function,
; reserved space for partition table and BIOS signature

DAP_PACKET_SIZE equ 16
PADDING_BYTES   equ CODE_SIZE - DAP_PACKET_SIZE - ($ - $$)

%if PADDING_BYTES < 0
  ; Not strictly needed, because this is catched by the times
  ; directive below. But this gives a more meaningful error message.
  %error "code too large, try to decrease size"
%endif

; padding bytes to ensure bootsector is 512 bytes in size
	times PADDING_BYTES nop

; By prepending the disk access packet to the partition table it is ensured
; that it is word-aligned.
dap:
  .packet_size	db 0x10
  .reserved1	db 0
  .sector_count db 1
  .reserved2	db 0
  .buf_off	dw 0x7c00
  .buf_seg	dw 0x0000
  .lba_low  	dw 0
  .lba_high 	dw 0
  		dd 0
.end:

%if dap.end - dap != DAP_PACKET_SIZE
  ; Be sure to get DAP_PACKET_SIZE right. We defined it manually above because
  ; times directive needs a defined expression value.
  %error "Wrong DAP size"
%endif


reserved times 6 db 0		; bytes 0x440-0x445 may contain operating
				; system specific data
parttbl:
	times PARTTBL_SIZE db 0	; space for partition table
signature:
	db 0x55, 0xaa		; BIOS signature

%if $ - $$ != 512
  %error "wrong bootsector size"
%endif


absolute RELOCATED_OFFSET
driveno resb 1  ; BIOS drive number to boot from
