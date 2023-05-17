name: .asciiz "Eduardo "

_start:
	mov r7, 0
	mov r6, 1
loop:
	mov r1, name
	mov r0, 1             # 1 is print string service
	syscall

	mov r1, 0
	add r1, r1, r7
	mov r0, 3             # 3 is integer print service
	syscall
	
	mov r0, 2             # 2 is newline print service
	syscall

	add r7, r7, r6
	jump loop