format ELF executable 3
entry start

;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Buffer macros
	BUFF_SIZE equ 32
;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Includes of macros
include 'elf.inc'
include 'char.inc'
;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Code
segment executable writeable readable
put:
	print ecx
	ret

read_stdin:
	push ecx
	read temp,1
	cmp byte [temp], ENDL
	jz .done
	mov al, byte[temp]
	pop ecx
	mov byte [ecx], al
	add ecx, 1
	jmp read_stdin
.done:
	add esp, 4
	ret

reserve buff,BUFF_SIZE

start:
	mov ecx, msg
	call put
	mov ecx, buff
	call read_stdin
	mov ecx, buff
	call put
	exit 0
reserve no_code,20
;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Data
segment readable writeable
msg db 'Hello user, what is your name?',ENDL,0
reserve temp,1
;;;;;;;;;;;;;;;;;;;;;;;;;;;
