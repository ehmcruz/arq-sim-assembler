data: .data 50
vector: .word 0 1 2 3 4 5 6 7 8 9;
name: .asciiz "Eduardo"


_start:
alpha:
	add r1, r2, r3
	load r7, [r5]
	store [r6], r1
	jump alpha
	mov r4, data
	mov r6, 50
betha:
	mov r5, -10
	jump_cond r3, betha
end: