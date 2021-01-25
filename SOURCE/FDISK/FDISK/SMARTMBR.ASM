;
; extremely clever DOS MBR by tom ehlert
;
; Copyright 2006 tom ehlert
; all rights reserved
;


segment _DATA           class=DATA  align=2

                 
  global  _smart_mbr            
_smart_mbr:

  global  _BootSmart_code
_BootSmart_code:

;-----------------------------------------------------------------------
;   ENTRY (copied from freedos bootsector)
;
; IN: DL = boot drive
;OUT: DL = boot drive
;
;-----------------------------------------------------------------------
align 16

real_start:     
									; 1) whereever we are, we copy ourself
									; 2) to 0:7c00

				call $+3
				pop  si				; get current location
				
				
                cld
                xor     ax, ax
                mov     ds, ax
                mov     es, ax
                mov     di, 0x7802

                mov     ss, ax          ; initialize stack

                mov     sp, di
                inc     di
                

                mov     cx, 0x0200-3  
                
                                
                db 0x2e  				; CS: copy from current code segment
                rep     movsb 

                jmp     word 0x0000:0x7800+ cont-_smart_mbr

cont:           


				mov     ds, ax
                mov     ss, ax
                xor     ax,ax
                mov     es,ax
                
               call    print
               db      "DriveSmart...",0


										 ; search for active partition
                mov bp,0x79be				 ; start of partition table
test_next_for_active:				
                test byte [bp],0x80
                jne  active_partition_found
                add  bp,0x10                    ; next table
                cmp  bp, 07800h+0x1fe; scanned beyond end of table ??
                jb  test_next_for_active

;*****************************************************************				
                call print
                db 'no active partition found',0
				
WAIT_FOR_REBOOT: 
				mov ax,0x4c00
				int 0x21
				hlt
                jmp WAIT_FOR_REBOOT


;*****************************************************************
trouble_reading_drive:
                call print
                db 'read error',0
                jmp WAIT_FOR_REBOOT

;*****************************************************************

invalid_partition_code:
                call print
                db 'signature != 55AA',0
			
                jmp WAIT_FOR_REBOOT

								
;*****************************************************************

active_partition_found:

				call print
				db 'loading active partition',0

                call read_boot_sector                    
				
                jc  trouble_reading_drive
			
                cmp word [0x7c00+0x1fe],0xaa55
                jne invalid_partition_code
			
;               call print
;               db '.jump DOS..',0

;

	call get_drive_params
	jc  gogogo

;  now we patch the new disk geometry into loaded boot sector
	
	mov si,0x7c00
	
	mov eax,[bp+8]
	mov [si+0x1c],eax		; hidden sectors
	
	mov [si+0x1a],dx		; heads
	mov [si+0x18],bx		; sectors per track




gogogo:			
    call print
    db 'go',0

    ; jmp WAIT_FOR_REBOOT


    jmp word 0x0:0x7c00             ; and jump to boot sector code


;*****************************
; read_boot_sector				
;
; IN: DI--> partition info
;OUT:CARRY
;*****************************

read_boot_sector:

   				int 3 
   				
		mov word [0x7c00+0x1fe],0

                ;  /* check for LBA support */
		mov bx,0x55aa		
		mov ah,0x41
		mov dl,[bp]
		int 0x13

		jc  StandardBios    ;  if (regs.b.x != 0xaa55 || (regs.flags & 0x01))
		cmp bx,0xaa55       ;    goto StandardBios;
		jne StandardBios

                              ;  /* if DAP cannot be used, don't use LBA */
                              ;  if ((regs.c.x & 1) == 0)
                              ;    goto StandardBios;
 		test cl,1
 		jz StandardBios
 
        jmp short LBABios



;struct _bios_LBA_address_packet            /* Used to access a hard disk via LBA */
;{
;  unsigned char packet_size;    /* size of this packet...set to 16  */
;  unsigned char reserved_1;     /* set to 0...unused                */
;  unsigned char number_of_blocks;       /* 0 < number_of_blocks < 128       */
;  unsigned char reserved_2;     /* set to 0...unused                */
;  UBYTE far *buffer_address;    /* addr of transfer buffer          */
;  unsigned long block_address;  /* LBA address                      */
;  unsigned long block_address_high;     /* high bytes of LBA addr...unused  */
;};

_bios_LBA_address_packet:
	db 0x10
	db 0
	db 4				; read four sectors - why not
	db 0
	dw 0x7c00			; fixed boot address for DOS sector
	dw 0x0000	
_bios_LBA_low  dw 0
_bios_LBA_high dw 0
	dw 0,0


LBABios:
						; copy start adpdress of partition to DAP
	mov ax,[bp+8]
	mov [0x7800+ (_bios_LBA_low-real_start)],ax
	mov ax,[bp+8+2]
	mov [0x7800+ (_bios_LBA_high-real_start)],ax

    mov ax,0x4200		;  regs.a.x = LBA_READ;
    mov si,0x7800+ (_bios_LBA_address_packet-real_start); regs.si = FP_OFF(&dap);

	int 0x13
	ret

;*****************************************************************
; read disk, using standard BIOS
;
StandardBios:
;	call print
;	db 'standard BIOS',0  



	call get_drive_params
	jnc  lba_chs

							; take dx/cx 

	mov cx,[bp+2]			;      regs.c.x =
          					; ((chs.Cylinder & 0xff) << 8) + ((chs.Cylinder & 0x300) >> 2) +
          					; chs.Sector;
          					; that was easy ;-)
    mov dh,[bp+1]			;  regs.d.b.h = chs.Head;
    						;  regs.es = FP_SEG(buffer);


read_normal_bios2:

    mov ax,0x0204			;  regs.a.x = 0x0201;
    mov bx,0x7c00			;  regs.b.x = FP_OFF(buffer);
    mov dl,[bp]						
	int 0x13
	ret	
	

lba_chs:
	;	DX heads 
	;	CX CYL
	;   BX sectors

							; transform LBA into CHS 
	mov eax, [bp+8]  
	
	movzx esi,bx		; ESI=sectors
	movzx edi,dx		; EDI=heads
	
	xor edx,edx
	div esi				; eax = eax/sectors = cylinder
	                        ; edx = sectors 

	inc dl					; sectors are 1..63	                        
	push dx					; mod --> dh -> sector number to read
	
	xor edx,edx
	div edi					; eax = eax/heads
							; edx = heads
	
	shr ah,6
	
	pop cx					; sectors
	or cl,ah				; 2 high bits of cyl count
	
	mov ch,al				; CH = low byte of cyl
	mov dh,dl               ; DH = head

	jmp read_normal_bios2 	
 	




;****** PRINT
; prints text after call to this function.

print_1char:        
                xor   bx, bx                   ; video page 0
                mov   ah, 0x0E                 ; else print it
                int   0x10                     ; via TTY mode
print:          pop   si                       ; this is the first character
print1:         lodsb                          ; get token
                push  si                       ; stack up potential return address
                cmp   al, 0                    ; end of string?
                jne   print_1char              ; until done
                ret                            ; and jump to it
                
                
;INT 13 - DISK - GET DRIVE PARAMETERS (PC,XT286,CONV,PS,ESDI,SCSI)
;	AH = 08h
;	DL = drive (bit 7 set for hard disk)
;	ES:DI = 0000h:0000h to guard against BIOS bugs
;Return: CF set on error
;	    AH = status (07h) (see #00234)
;	CF clear if successful
;	    AH = 00h
;	    AL = 00h on at least some BIOSes
;	    BL = drive type (AT/PS2 floppies only) (see #00242)
;	    CH = low eight bits of maximum cylinder number
;	    CL = maximum sector number (bits 5-0)
;		 high two bits of maximum cylinder number (bits 7-6)
;	    DH = maximum head number
;

;	    CH = low eight bits of maximum cylinder number
;	    CL = maximum sector number (bits 5-0)
;		 high two bits of maximum cylinder number (bits 7-6)
;	    DH = maximum head number

drive_heads       dw 0
drive_secpertrack dw 0


	; in  BP->partition entry
	; out 
	;	DX heads 
	;	CX CYL
	;   BX sectors

get_drive_params:

	mov ah,0x08
	mov dl,[bp]
	int 0x13
	jc  no_info
	
	mov bl,cl
	and bx,0x3f		; max sector 	
	cbw
	
	shr cl,6
	xchg ch,cl		; max cyl
	
	inc cx			; cyl count
	
	mov dl,dh
	mov dh,0
	inc dx          ; total head count
	
	
	mov [drive_heads],dx
	mov [drive_secpertrack],bx
	   
	clc   
no_info:	
    ret


		times	0x1fe-$+$$-0x40 db 0			; fill up to partition table
		

; TEST disk                                                
db 0x00,0x01,0x01,0x00,0x06,0xfe,0x3f,0x81,0x3f,0x00,0x00,0x00,0xc3,0xdd,0x1f,0x00
db 0x80,0x00,0x01,0x82,0x06,0xfe,0x7f,0x03,0x02,0xde,0x1f,0x00,0x02,0xde,0x1f,0x00                                                
                                                
                                                ; Toms disk1 
;db 0x80,0x01,0x01,0x00,0x06,0xfe,0x3f,0x81,0x3f,0x00,0x00,0x00,0xc3,0xdd,0x00,0x00 ;T1
;db 0x80,0x00,0xc1,0xff,0x07,0xfe,0xff,0xff,0x43,0xc0,0xfb,0x00,0xb6,0x50,0xf4,0x04 
                                                
; db 0x80,0x01,0x01,0x00,0x07,0xFE,0x3F,0x81,0x3F,0x00,0x00,0x00,0xC3,0xDD,0x1F,0x00
; db 0x00,0x00,0x01,0x82,0x0F,0xFE,0xFF,0xFF,0x02,0xDE,0x1F,0x00,0x25,0x09,0xD0,0x01
db 0x00,0x00,0xC1,0xFF,0x07,0xFE,0xFF,0xFF,0x27,0xE7,0xEF,0x01,0x7C,0xE7,0x9B,0x00
db 0x00,0x00,0xC1,0xFF,0x07,0xFE,0xFF,0xFF,0xB3,0x7B,0xCE,0x02,0x12,0x64,0x83,0x06

		times	0x1fe-$+$$ db 0                 ; fill up to magic 55 AA
 		
		db 0x55,0xaa
		                               

	
	
	
	
	









