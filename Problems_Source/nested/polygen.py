# Random code encryption generation for FASM
import os
import math

OP_CNT = 3
ops = ['-', 'xor', '+']
# Generates a random, reversible, operation.
def gen_random_op():
	rand = ord(os.urandom(1))%OP_CNT
	return ops[rand]

# Generates a random, reversible, algorithm of given length of operations
def generate_encryption(length):
	algo = []
	for i in range(0,length):
		algo.append(gen_random_op())
	return algo

# Generates a list of random byte sized numbers of a given length
def gen_nums(length):
	nums = []
	for i in range(0, length):
		nums.append((ord(os.urandom(1)) % 254) + 1)
	return nums

# Reverses an operations
def reverse_op(op):
	op_index = ops.index(op)
	mid_index = int(OP_CNT/2)
	op_index = mid_index - op_index
	op_index += mid_index
	return ops[op_index]

# Reverses a list of operations
def reverse_algo(algo):
	new_algo = []
	for op in algo:
		new_algo.append(reverse_op(op))
	return new_algo[::-1]

mnemonics = ['sub', 'xor', 'add']

# Returns a mnemonic from an operation
def get_mnemonic(op):
	return mnemonics[ops.index(op)]

# Generates an FASM macro for encrypting code with the generated algorithm
def encryption_macro(algo, numbers, name):
	macro = 'macro %s code_begin,code_length\n{\n\tlocal ..byte\n\trepeat code_length\n\t\tload ..byte from code_begin+%%-1\n\t' % name
	for i in range(0,len(algo)):
		macro += '\t..byte = (..byte %s %d) and 0xff\n\t' % (algo[i], numbers[i])
	macro += '\tstore ..byte at code_begin+%-1\n\tend repeat\n}\n'
	return macro

# Generates a FASM decryption code for a given algorithm
# Arg0, pushed last, ptr to code
# Arg1, pushed first, length of code, should be an even 2^value
def gen_fasm_decrypt_code(algo, numbers, name, reg0 = 'ecx', reg1 = 'edx'):
	if reg0 == 'eax' or reg1 == 'eax':
		assert 'Bad register \'eax\' for the gen_fasm_code arguments'
		exit(1)
	code = '%s:\n' % name
	num_list = numbers[::-1]
	code += '\tpush ebp\n'
	code += '\tcall .pseudo_random\n\tmov %s, eax\n' % reg0
	code += '\tcall .pseudo_random\n\tmov ebp, eax\n\tmov %s, ebp\n' % reg1
	decrp = reverse_algo(algo)
	code += '.loop_start:\n'
	code += '\tmov eax, %s\n' % reg0
	code += '\txor eax, %s\n' % reg1
	code += '\tadd eax, dword [esp + 8]\n'
	code += '\tpush %s\n' % reg0
	code += '\tmov %s, eax\n' % reg0
	code += '\txor eax, eax\n'
	code += '\tmov al, byte [%s]\n' % (reg0)
	for i in range(0, len(algo)):
		code += '\t%s eax, %#x\n' % (get_mnemonic(decrp[i]),num_list[i]) 
	code += '\tmov byte [%s], al\n' % reg0
	code += '\tpop %s\n' % reg0
	# This was originally operating under the theory that I could add a pseudo random number everytime
	# and just drop the least significant bit such that i still looped through everything.
	# did not work.
	#code += '\tcall .pseudo_random\n\tand eax, 0xfffffffe\n'
	code += '\tadd %s, esp\n' % reg0
	code += '\tpush edi\n'
	code += '\tmov edi, dword [esp + 0x10]\n'
	code += '\tsub edi, 1\n'
	code += '\tand %s, edi\n' % reg0
	code += '\tinc %s\n\tand %s, edi\n' % (reg1, reg1)
	code += '\tpop edi\n' 
	code += '\tcmp %s, ebp\n\tjnz .loop_start\n' % reg1
	code += '\tpop ebp\n\tjmp .finished\n'
	code += '.pseudo_random:\n\tpush edx\n\trdtsc\n\tmov edx, dword [esp + 0x14]\n'
	code += '\tsub edx, 1\n\tand eax, edx\n\tpop edx\n'
	code += '.finished:\n\tret\n'
	return code

# Generates LINEAR algo for decryption
# Arg0, pushed last, ptr to code
# Arg1, pushed first, length of code, should be an even 2^value
def gen_linear_decrypt_code(algo, numbers, name, reg0 = 'ecx', reg1 = 'edx'):
	if reg0 == 'eax' or reg1 == 'eax':
		assert 'Bad register \'eax\' for the gen_fasm_code arguments'
		exit(1)
	code = '%s:\n' % name
	num_list = numbers[::-1]
	code += '\tpush ebp\n'
	code += '\tmov %s, 0\n' % reg0
	code += '\tmov %s, dword [esp + 0xc]\n' % reg1
	decrp = reverse_algo(algo)
	code += '.loop_start:\n'
	code += '\tmov eax, %s\n' % reg0
	code += '\tadd eax, dword [esp + 8]\n'
	code += '\tpush %s\n' % reg0
	code += '\tmov %s, eax\n' % reg0
	code += '\txor eax, eax\n'
	code += '\tmov al, byte [%s]\n' % (reg0)
	for i in range(0, len(algo)):
		code += '\t%s eax, %#x\n' % (get_mnemonic(decrp[i]),num_list[i]) 
	code += '\tmov byte [%s], al\n' % reg0
	code += '\tpop %s\n' % reg0
	code += '\tinc %s\n' % reg0
	code += '\tcmp %s, %s\n\tjnz .loop_start\n' % (reg0, reg1)
	code += '\tpop ebp\n'
	code += '.finished:\n\tret\n'
	return code

ALGO_LEN = 15

encryption = generate_encryption(ALGO_LEN)
rev_encryp = reverse_algo(encryption)
magicks = gen_nums(ALGO_LEN)
#print(encryption)
#print(rev_encryp)
#print(magicks)

enc_macro = encryption_macro(encryption, magicks, 'algo_enc')
print(enc_macro)
print('macro drop_enc_code name\n{')
print(gen_linear_decrypt_code(encryption, magicks, 'name'))
print('}')