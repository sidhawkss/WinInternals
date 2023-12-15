.code

	SysNtOpenProcess PROC
		mov	r10,rcx
		mov eax,26h
		syscall
		ret
	SysNtOpenProcess ENDP
	
	SysNtAllocateVirtualMemory PROC
		mov	r10,rcx
		mov	eax,18h
		syscall
		ret
	SysNtAllocateVirtualMemory ENDP

	SysNtWriteVirtualMemory PROC
		mov	r10,rcx
		mov	eax,3Ah
		syscall
		ret
	SysNtWriteVirtualMemory ENDP

	SysNtCreateThreadEx PROC
		mov	r10,rcx
		mov	eax,02Ch
		syscall
		ret
	SysNtCreateThreadEx ENDP



end