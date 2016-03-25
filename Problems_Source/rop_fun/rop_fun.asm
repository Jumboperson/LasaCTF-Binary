format ELF

;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Includes of macros
include 'elf.inc'
include 'char.inc'
include 'test_enc.inc'
;;;;;;;;;;;;;;;;;;;;;;;;;;;

	PHUN_SIZE 	equ 1024
	SEC_SIZE 	equ 1024
	SHELL_LEN	equ 28

section '.phun' executable writeable

let_them_write_code:
	jmp .over_text
	db 'Like gadgets?',0
	db 'Not sure you will need all of these, have fun!',0,0
.over_text:
	pop ecx
	pop edx
	pop ebp
	ret

	call .local_call
.local_call:
	pop ecx
	add ecx, write_code - $
	ret

	int 0x80
	ret

	popfd
	ret

	popad
	ret

decrypt_algo_pt2:
	push ebp
	call .pseudo_random
	mov ecx, eax
	call .pseudo_random
	mov ebp, eax
	mov edx, ebp
.loop_start:
	mov eax, ecx
	xor eax, edx
	add eax, dword [esp + 8]
	push ecx
	mov ecx, eax
	xor eax, eax
	mov al, byte [ecx]
	xor eax, 0x87
	xor eax, 0xf8
	sub eax, 0x1c
	xor eax, 0x76
	sub eax, 0xc1
	xor eax, 0x1
	sub eax, 0x77
	sub eax, 0x19
	xor eax, 0x6
	sub eax, 0x29
	mov byte [ecx], al
	pop ecx
	add ecx, esp
	push edi
	mov edi, dword [esp + 0x10]
	sub edi, 1
	and ecx, edi
	inc edx
	and edx, edi
	pop edi
	cmp edx, ebp
	jnz .loop_start
	pop ebp
	jmp .finished
.pseudo_random:
	push edx
	rdtsc
	mov edx, dword [esp + 0x14]
	sub edx, 1
	and eax, edx
	pop edx
.finished:
	ret

resv_stuff PHUN_SIZE-($-let_them_write_code)

write_code:
	call ebp
	add esp, 4
	call .local_call
.local_call:
	pop ecx
	add ecx, congrats-.local_call
	print ecx
	ret
	congrats db 'Nice work, you are almost there!', ENDL, 0

resv_stuff SEC_SIZE-($-write_code)

algo_enc write_code, SEC_SIZE

test_enc let_them_write_code, PHUN_SIZE+SEC_SIZE

section '.rop' executable

drop_enc_code decrypt_algo

public main

decrpyt_code:
	push PHUN_SIZE+SEC_SIZE
	push let_them_write_code
	call decrypt_algo
	add esp, 8
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

overflow_crap:
	push ebp
	mov ebp, esp

	sub esp, 0x100

	mov ecx, esp
	call read_stdin

	mov esp, ebp
	pop ebp
	ret

main:
	mov esp, stk_end
	print hellof

	call overflow_crap

	exit 0

hellof db 'Hi, enjoy ropping about!', 0

section '.stk' writeable
temp:
	rd 1

	rd 0xfff
stk_end:
	rd 1
