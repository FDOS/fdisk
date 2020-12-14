;
; normal DOS boot sector
;

segment _BOOTEASY_DATA           class=DATA align=2
                 
  global  _booteasy_code
_booteasy_code:

; BOOTEASY, version 1.7.
; Developed from BOOTANY, with many improvements.
; The main idea was to simplify the installation
; procedure and to remove unnecessary partition type switching.
; Second hard disk switching added.
; Author: Serge Vakulenko, <vak@kiae.su>
; Ported to NASM by Tom Ehlert

F1_scancode     equ     59		; scancode of 'F1' key
Enter_scancode	equ	28		; scancode of 'Enter' key
Timeout		equ     5		; wait up to 5 seconds for reply

StdBase		equ     7c00h		; address where DOS loads us
Base		equ	600h		; address where we rewrite itself
Partab		equ	1beh		; partition table offset
NumDisks	equ	475h		; number of disk drives (BIOS data area)

BootIndicator   equ	0		; partition record: boot indicator
BeginHead       equ	1		; first partition sector: head number
BeginSector     equ	2		; sector and cylinder
SystemId        equ	4		; system type

; -------------------------------

;;dummy		segment at 0
;;		assume  cs:dummy
;;		org	Base		; address where we rewrite itself
;;BaseEntry:
;;		org	StdBase		; address where DOS loads us
;;StdBaseEntry:
;;dummy		ends

; -------------------------------

;;code		segment 
;;		assume  cs:code, ds:code, es:code, ss:code
;;		org     0
Boot:
;
;               Setup the stack and segment registers
;
		xor	AX,AX		; Zero register
		mov	ES,AX		; ES := 0
		mov	DS,AX		; DS := 0
		mov     SS,AX           ; disables intrs up to the next command
		mov	SP,StdBase	; SP at 7c00
;
;               DOS loads this pgm at 0000:7C00. Any boot routine
;               we call also expects to execute there so the first
;               exercise is to move this code somewhere else.
;
		cld			; Clear direction
		mov	SI,SP		; Copy from 7c00h...
		mov	DI,Base		; ...to 600h...
		mov	CX,256		; ...512 bytes, or 256 words
		repne	movsw		; Move itself to new location

        jmp     word 0x0000:Base+(Entry-Boot)

; -------------------------------
;
;               A valid function key was depressed (or defaulted)
;               Attempt to boot the corresponding partition.
;
Load:
		mov	DX,BP		; restore drive number
		pop	AX		; key '1'..'5'
		mov     [Base+default],AL	; save function key number
		cmp	AL,'5'
		je	SwitchDrive
		mov	AH,16
		mul	AH		; AX = key * 16
		add	AX,Base+Partab-'1'*16 ; subtract '1'
		mov	SI,AX
;
;               Check if the partition is empty.
;
		cmp	BYTE [SystemId+SI],0
		je      Menu		; Empty - display menu again
		cmp	BYTE [SystemId+SI],05h
		je      Menu		; DOS extended type - never load

		mov     BYTE [BootIndicator+SI],80h ; Mark partition as bootable
		call	SaveBoot
;
;               Read in and validate the partition's boot sector.
;
		mov     DH,[BeginHead+SI] ; head from partition table
		mov     CX,[BeginSector+SI]
		jmp	short loadboot
;
;               Read in the boot sector from second disk.
;
SwitchDrive:
		call	SaveBoot
		mov     CX,0001h	; cylinder 0, sector 1
		xor	DL,CL		; (DL ^= 1) switch drive
loadboot:
		mov	BX,StdBase	; ES already == 0
		mov     AX,0201h	; function, # of sectors
		int     13h		; read system boot record
		jc      Menu		; exit if error
		cmp     WORD [510+BX],0aa55h ; test signature
		jne     Menu		; reprompt if invalid
;
;		Jump to secondary boot.
;		DL now contains boot disk number (80h or 81h);
;		ES:SI contains the address of partition table
;		entry to boot.
;
		;jmp	FAR PTR StdBaseEntry    
		jmp word 0x0:0x7c00		; and jump to boot sector code

; -------------------------------
;
;		The main entry to the boot
;
Entry:
		cmp	DL,81h		; DL contains current drive number
		je      driveok		; is it valid?
		mov	DL,80h		; no - use the default value
driveok:	mov	BP,DX		; save the drive number
		inc	DX		; 80h -> 81h, 81h -> 82h
		xor	DL,80h+'3'	; 80h -> '2', 81h -> '1'
		mov	[Base+diskNum],DL
;
;               Display the menu
;
Menu:
		mov     DI,Base+Partab	; set index
		mov     CX,4		; set loop count
		mov	byte [Base+key],'1'	; set key number in message
		xor	DH,DH		; count of partitions
menuloop:
		mov     BYTE [BootIndicator+DI],CH ; Clear active flag
		mov	AL,[SystemId+DI]
		cmp	AL,0		; unused partition?
		je	next
		cmp	AL,5		; extended DOS partition?
		je	next
		inc	DH		; increment partition count

		lea     SI,[Base+FkeyMsg]	; get msg addr
		call    Output

		lea	SI,[Base+nameTable-2]
nameloop:
		inc	SI
		inc	SI
		mov	BX,[SI]
		or	BH,BH
		je	endnameloop
		xor     BH,[SystemId+DI]
		jne	nameloop
endnameloop:
		lea	SI,[Base+namtab+BX]
		call	Output
next:
		add	DI,byte 16		; next entry address
		inc	byte [Base+key]
		loop	menuloop

		cmp	BYTE [NumDisks],2 ; is the second disk present?
		je	have2disks	; have disk 2

		lea     SI,[Base+defaultMsg] ; prepare 'Default' message
		or	DH,DH		; no disk 2; do we have valid partitions?
		jne	prompt		; several partitions, wait for reply

		int	18h		; no partitions, load ROM basic
		jmp	short Menu		; repeat, if no ROM basic
have2disks:
		lea     SI,[Base+FkeyMsg]	; print 'F5'
		call    Output		; now SI points to "disk 2\nDefault..."
prompt:
		call    Output		; print 'Default' message
reprompt:
		xor     AH,AH		; GetTickCount
		int     1ah		; BiosTimerService
		mov     BX,DX           ; lo-order tick count
		add     BX,byte 192*Timeout/10 ; timeout value in ticks
;
;               Get the reply
;
waitkey:
		mov     AH,1		; keyboard status
		int     16h		; keybd bios service
		mov     AH,0            ; GetTickCount
		jnz     reply		; jump if reply
		int     1ah		; BiosTimerService
		cmp     DX,BX           ; check for timeout
		jb      waitkey		; wait for scancode
loaddefault:
		mov     AL,[Base+default]	; prior system id
		jmp     short testkey		; boot default system
reply:
		int     16h             ; AH=0, keybd bios service
		mov     AL,AH		; Copy to AL
		cmp	AL,Enter_scancode
		je	loaddefault
		add     AL,'1'-F1_scancode ; Turn into index
testkey:
		cmp     AL,'1'		; max Function key
		jb	reprompt	; Invalid code check
		cmp     AL,'5'		; max Function key
		jnbe    reprompt	; if not F1..F5, branch
		push	AX

		lea	SI,[Base+newLine]	; new line
		lea	BX,[Base+Load]
		push	BX		; call Output; jmp Load

; -------------------------------
;
;		Output line [SI] to screen, end of line marked with 80h
;
Output:
		cld			; reset direction flag
		lodsb			; load argument from string
		push	AX		; save byte
		and     AL,7fh		; insure valid character
		mov     AH,14		; write tty
		int     10h		; bios video service
		pop	AX		; restore byte
		test    AL,80h		; test for end of string
		jz      Output		; do until end of string
		ret			; return to caller

; -------------------------------
;
;		Save boot block default partition settings
;
SaveBoot:
		push	SI
		mov     AX,0301h	; write sector
		mov	BX,Base		; ES already == 0
		mov     CX,0001h	; cylinder 0, sector 1
		xor     DH,DH		; drive #, head 0
		int     13h		; replace boot record
		pop	SI
		mov     byte [Base+default],'?' ; reset default
		ret

; -------------------------------

newLine		db	13,10+80h
FkeyMsg         db      13,10,'F'
key             db      '0 . . .',' '+80h,'disk '
diskNum		db	'1'
defaultMsg	db	13,10,10,'Default: F'
default         db      '?',' '+80h

nameTable       db      fat     -namtab,   1
                db      fat     -namtab,   4
                db      fat     -namtab,   6
                db      hpfs    -namtab,   7
		db      os2     -namtab,  0Ah
                db      fat32   -namtab,  11
                db      fat32   -namtab,  12
                db      fat     -namtab,  14
                db      unix    -namtab,  63h
;               db      novell  -namtab,  64h
;               db      novell  -namtab,  65h
                db      minix   -namtab,  80h
		db      linux   -namtab,  81h
		db      linux   -namtab,  82h
		db      linux   -namtab,  83h
;               db      amoeba  -namtab,  93h
		db      freebsd -namtab, 0A5h
                db      bsdi    -namtab,  9fh
;               db      pcix    -namtab,  75h
                db      cpm     -namtab,  52h
                db      cpm     -namtab, 0dbh
;               db      venix   -namtab,  40h
;               db      dossec  -namtab, 0F2h
		db      noname  -namtab,   0

namtab:
fat             db      'FA','T'+80h
fat32           db      'FAT3','2'+80h
hpfs            db      'Hpf','s'+80h
os2             db      'Os','2'+80h
unix            db      'Uni','x'+80h
;novell          db      'Novel','l'+80h
minix           db      'Mini','x'+80h
linux           db      'Linu','x'+80h
;amoeba          db      'Amoeb','a'+80h
freebsd         db      'FreeBS','D'+80h
bsdi            db      'BSD','i'+80h
;pcix            db      'Pci','x'+80h
cpm             db      'Cp','m'+80h
;venix           db      'Veni','x'+80h
;dossec          db      'Dosse','c'+80h
noname		db	'?','?'+80h
                              
		times	0x1be-$+$$ db 0	; error if too big
		times	64         db 0 ; partition table
                              
		dw	0aa55h		; magic

;code		ends
		end
