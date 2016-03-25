import os

BASE = 0x56d5c9dc
for i in range(0,60):
	print('dec_ransom.exe %x wonder.txt.enc' % (BASE + i))
	print('pause')
