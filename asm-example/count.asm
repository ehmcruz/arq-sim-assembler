value: .word 10 ;

_start:
	mov r7, 0
	mov r1, 50
	mov r5, 1
	mov r6, value
loop:
	cmp_equal r3, r7, r1
	jump_cond r3, fim
	load r4, [r6]
	add r4, r4, r5
	store [r6], r4
	add r7, r7, r5
	jump loop
fim:
	mov r0, 0     # end service
	syscall
