macro test_enc code_begin,code_length
{
	local ..byte
	repeat code_length
		load ..byte from code_begin+%-1
		..byte = (..byte xor 252) and 0xff
		..byte = (..byte xor 59) and 0xff
		..byte = (..byte xor 56) and 0xff
		store ..byte at code_begin+%-1
	end repeat
}

macro algo_enc code_begin,code_length
{
	local ..byte
	repeat code_length
		load ..byte from code_begin+%-1
		..byte = (..byte + 41) and 0xff
		..byte = (..byte xor 6) and 0xff
		..byte = (..byte + 25) and 0xff
		..byte = (..byte + 119) and 0xff
		..byte = (..byte xor 1) and 0xff
		..byte = (..byte + 193) and 0xff
		..byte = (..byte xor 118) and 0xff
		..byte = (..byte + 28) and 0xff
		..byte = (..byte xor 248) and 0xff
		..byte = (..byte xor 135) and 0xff
		store ..byte at code_begin+%-1
	end repeat
}

macro drop_enc_code name
{
name:
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
	mov al, byte [eax]
	xor al, 0x38
	xor al, 0x3b
	xor al, 0xfc
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
}