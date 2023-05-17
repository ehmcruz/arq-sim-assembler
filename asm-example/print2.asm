name: .asciiz "Maria"

_start:
	mov r1, name
	mov r0, 1             # 1 is print string service
	syscall
	mov r0, 2             # 2 is newline print service
	syscall
	jump _start