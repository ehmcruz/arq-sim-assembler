str_result_alloc: .asciiz "allocate result "
str_result_dealloc: .asciiz "de-allocate result "
str_addr: .asciiz "addr "

_start:
	mov r7, 0

	#############################

	# allocate

	mov r0, 4     # malloc
	mov r1, 20
	syscall

	add r3, r1, r7    # r3 = r1 (result)
	add r4, r2, r7    # r4 = r2 (addr)

	# print result

	mov r0, 1
	mov r1, str_result_alloc
	syscall

	mov r0, 3
	add r1, r3, r7
	syscall

	# print newline

	mov r0, 2
	syscall

	# print addr

	mov r0, 1
	mov r1, str_addr
	syscall

	mov r0, 3
	add r1, r4, r7
	syscall

	# print newline

	mov r0, 2
	syscall

	#############################

	# de-allocate

	mov r0, 5
	add r1, r4, r7
	syscall

	add r3, r1, r7    # r3 = r1 (result)

	# print result

	mov r0, 1
	mov r1, str_result_dealloc
	syscall

	mov r0, 3
	add r1, r3, r7
	syscall

	# print newline

	mov r0, 2
	syscall

	#############################

	mov r0, 0     # end service
	syscall
