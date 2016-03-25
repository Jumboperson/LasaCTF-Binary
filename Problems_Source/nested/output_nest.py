
def create_fasm(file, sec1, sec2, output, name):
	outFile = """format PE console\nuse32\ninclude '../../asm/include/win32a.inc'\ninclude 'std.inc'\ninclude 'enc.inc'\nentry start
section '.%s' code readable executable
drop_enc_code decrypt_binary
data import
\tlibrary msvcrt32, 'msvcrt.dll'\n\timport msvcrt32, printf , 'printf'
end data
start:
\tpush outputString
\tcall [printf]
\tadd esp, 4
\tret	
\tpush endFileLabel - outputFile
\tpush outputFile
\tcall decrypt_binary
section '.%s' data readable writeable
outputString db '%s', ENDL, 0
outputFile file '%s.exe'
endFileLabel:
algo_enc outputFile, $-outputFile""" % (sec1, sec2, output, name)
	f = open('%s.asm' % file, 'w')
	f.write(outFile)
	f.close()
	return

def create_batch(name, file_prefix):
	output = 'echo off\n'
	create_fasm('%s0' % file_prefix, 'sec1', 'sec2', 'Hello!', 'nested')
	output += 'fasm %s%d.asm\n' % (file_prefix, 0)
	###############################
	# Important number
	MAX_FILE = 100
	###############################
	for i in range(1, MAX_FILE):
		create_fasm('%s%d' % (file_prefix, i), 'sec1', 'sec2', 'I am phase %d' % (MAX_FILE - i), '%s%d' % (file_prefix, (i-1)))
		output += 'fasm %s%d.asm\n' % (file_prefix, i)
		output += 'python polygen.py > enc.inc\n'
	output += 'del %s*.asm' % file_prefix
	f = open('%s.bat' % name, 'w')
	f.write(output)
	f.close()
	return

create_batch('build_bins', 'zzTest')
