format PE console
use32

;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Includes of macros
include '../../asm/include/win32a.inc'
include 'std.inc'
include 'enc.inc'
;;;;;;;;;;;;;;;;;;;;;;;;;;;

entry start

section '.nest' code readable executable

drop_enc_code decrypt_binary

data import
	library msvcrt32, 'msvcrt.dll'

	import msvcrt32, \
		printf , 'printf'
end data

start:
	push outputString
	call [printf]
	add esp, 4
	ret	
	push endFileLabel - outputFile
	push outputFile
	call decrypt_binary

section '...?' data readable writeable

outputString db 'I really like those russian nesting dolls!', ENDL, 0

outputFile file 'flag.exe'

endFileLabel:

algo_enc outputFile, $-outputFile
