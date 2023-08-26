teste_load: .word 123;
teste_separador: .word 0;
teste_store: .word 0;

_start:
	mov r0, teste_load
	load r1, [r0]
	mov r0, teste_store
	store [r0], r1

	mov r0, 0     # end service
	syscall
