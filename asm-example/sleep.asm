sleep_str: .asciiz "Sleeping ...\n"
wake_up_str: .asciiz "Wake up at time "


_start:

loop:
	mov r1, sleep_str
	mov r0, 1             # 1 is print string service
	syscall

	jump loop

end: