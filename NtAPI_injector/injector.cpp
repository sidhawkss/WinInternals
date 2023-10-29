#include <windows.h>
#include <stdio.h>

#include "ntheaders.h"

int main(int argc, char* argv[]) {
	PVOID pVallocBuff;
	DWORD dwError;
	DWORD dwProcessId, dwTID;
	HMODULE hKernel32, hNtDll = NULL;
	HANDLE hProcess, hThread;
	PTHREAD_START_ROUTINE kawLoadLibrary = NULL;
	wchar_t dllPath[MAX_PATH] = L"C:\\DLLPATH";
	size_t ui64BytesWritten = 0;
	OBJECT_ATTRIBUTES OA = { sizeof(OA), NULL };


	size_t zDllPathSize = sizeof(dllPath);
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
	kawLoadLibrary = (PTHREAD_START_ROUTINE)GetProcAddress(hKernel32,"LoadLibraryW");
	pNtCreateThreadEx kawRemoteThreadEx = (pNtCreateThreadEx)GetProcAddress( hNtDll,"NtCreateThreadEx");


	pVallocBuff = VirtualAllocEx(hProcess, NULL, zDllPathSize, (MEM_RESERVE | MEM_COMMIT), PAGE_EXECUTE_READWRITE);
	if (pVallocBuff) {
		printf("VA created in PID[%d].\n", dwProcessId);
	}
	else {
		printf("Could not create VA space.\n");
	}

	WriteProcessMemory(hProcess, pVallocBuff, dllPath, zDllPathSize, &ui64BytesWritten);
	if (ui64BytesWritten != 0) {
		printf("%d-Bytes written in memory\n", ui64BytesWritten);
	}



	/* CALL TO REMOTE THREAD */

	NTSTATUS status = kawRemoteThreadEx(&hThread, THREAD_ALL_ACCESS, &OA, hProcess, kawLoadLibrary, pVallocBuff, FALSE, NULL, NULL, NULL, NULL);



	WaitForSingleObject(hProcess, 0xffffffff);


	return EXIT_SUCCESS;
}