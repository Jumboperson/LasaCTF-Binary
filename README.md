## ReadMe ##

This repository contains problems authored by me for LasaCTF. 

The problem sources are provided, but not necessarily the solve methods.

### Problems ###

There are 8 problems written by me, each will be summarized below by point value in ascending order.

- **Easy (30 exp): ** This problem was a binary that had the flag in clearly in the binary, can be solved by a hex editor.

- **Xor (100 exp): ** A little binary that takes 1 command line argument, which is then converted to an integer, each byte of the flag is xor'd by the int, then printed. Intended to be solved by doing `'l' ^ [first letter of buffer]` and using that as the key

- **Simple Rop (120 exp): ** A small elf file, linked with libc, that has a stack overflow vulnerability in it which allows for overwriting of the return address of the function. Also contains a function which gives the user a shell.

- **Bad Buffer (150 exp): ** Binary which allows one to write code over itself because the buffer is in a Read/Write/Execute section and is above the main function in memory. 

- **Ransomland (190 exp): ** A piece of "ransomware" which only creates an encrypted rar of wonder.txt because I don't want to distribute working malware. Meant to be attacked by looking at the unix time of the file creation and moving backwards from there due to the algo using pseudo-randoms seeded by time.

- **ROP Phun (210 exp): ** A binary with a stack overflow vulnerability that contains self modifying code. Multiple correct solutions, meant for people to solve creatively. 

- **Nested (220 exp): ** A binary which contains a encrypted version of another binary, which contains an encrypted version of another binary... 100 times. Meant to be solved either via emulation or through actually executing the decryption on the binary. `nested_dumper` has my solution for this.

- **Vx1 (230 exp): ** An obfuscated and obtuse virtual machine that executes code written in its own assembly language (modelled after x86-64). 
