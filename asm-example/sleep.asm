sleep_str: .asciiz "Sleeping ...\n"
wake_up_str: .asciiz "Wake up at time "


_start:

loop:
	mov r1, sleep_str
	mov r0, 1             # 1 is print string service
	syscall

	mov r1, 2
	mov r0, 6             # 6 is sleep service
	syscall

	mov r1, wake_up_str
	mov r0, 1             # 1 is print string service
	syscall

	mov r0, 7             # 7 is a service that returns time in r1
	syscall

	mov r0, 3             # 3 is integer print service
	syscall

	mov r0, 2             # 2 is newline print service
	syscall

	jump loop

end: