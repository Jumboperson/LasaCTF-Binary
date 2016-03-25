format PE console
use32

;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Includes of macros
include '../../asm/include/win32a.inc'
include 'std.inc'
;;;;;;;;;;;;;;;;;;;;;;;;;;;

entry start

section '.nice' code readable executable

data import
	library msvcrt32, 'msvcrt.dll'

	import msvcrt32, \
		printf , 'printf'
end data

output db 'Heres your silly flag!', ENDL, 'lasactf{n3st_your_b1narie5_f0r_fun_n_pr0fit!}',0

start:
	push output
	call [printf]
	add esp, 4
	xor eax, eax
	ret
