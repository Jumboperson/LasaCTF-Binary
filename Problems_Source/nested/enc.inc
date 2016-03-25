macro algo_enc code_begin,code_length
{
	local ..byte
	repeat code_length
		load ..byte from code_begin+%-1
		..byte = (..byte xor 96) and 0xff
		..byte = (..byte - 107) and 0xff
		..byte = (..byte xor 193) and 0xff
		..byte = (..byte + 46) and 0xff
		..byte = (..byte + 253) and 0xff
		..byte = (..byte + 36) and 0xff
		..byte = (..byte xor 83) and 0xff
		..byte = (..byte + 45) and 0xff
		..byte = (..byte xor 105) and 0xff
		..byte = (..byte - 200) and 0xff
		..byte = (..byte xor 69) and 0xff
		..byte = (..byte + 184) and 0xff
		..byte = (..byte + 24) and 0xff
		..byte = (..byte + 158) and 0xff
		..byte = (..byte xor 171) and 0xff
		store ..byte at code_begin+%-1
	end repeat
}

macro drop_enc_code name
{
name:
	push ebp
	mov ecx, 0
	mov edx, dword [esp + 0xc]
.loop_start:
	mov eax, ecx
	add eax, dword [esp + 8]
	push ecx
	mov ecx, eax
	xor eax, eax
	mov al, byte [ecx]
	xor eax, 0xab
	sub eax, 0x9e
	sub eax, 0x18
	sub eax, 0xb8
	xor eax, 0x45
	add eax, 0xc8
	xor eax, 0x69
	sub eax, 0x2d
	xor eax, 0x53
	sub eax, 0x24
	sub eax, 0xfd
	sub eax, 0x2e
	xor eax, 0xc1
	add eax, 0x6b
	xor eax, 0x60
	mov byte [ecx], al
	pop ecx
	inc ecx
	cmp ecx, edx
	jnz .loop_start
	pop ebp
.finished:
	ret

}
