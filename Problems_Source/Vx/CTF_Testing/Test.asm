.586              ; Target processor.  Use instructions for Pentium class machines
.model flat, C    ; Use the flat memory model. Use C calling conventions
.stack            ; Define a stack segment of 1KB (Not required for this example)
.data             ; Create a near data segment.  Local variables are declared after

.code

Tester proc
	mov eax, 5
	retn
Tester endp

end
