#include <windows.h>
#include <stdio.h>


int main(void) {
	PVOID pVallocBuff;
	DWORD dwError;
	DWORD dwProcessId, dwTID;
	HMODULE hKernel32 = NULL;
	HANDLE hProcess, hThread;
	LPTHREAD_START_ROUTINE startLibrary = NULL;
	wchar_t dllPath[MAX_PATH] = L"C:\\dllPATH";
	size_t ui64BytesWritten = 0;


	size_t zDllPathSize = sizeof(dllPath);
	printf("Initializing the injector\n please input the PID to attach : ");

	if (scanf_s("%d", &dwProcessId) == EOF) 
	{
		printf("Invalid input!\n");
	}

	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
	if (!hProcess) {
		printf("Error attaching PID!\n");
	}
	else {
		printf("Process handle opened.\n");
	}

	hKernel32 = GetModuleHandleA("kernel32");
	startLibrary = (LPTHREAD_START_ROUTINE)GetProcAddress(hKernel32, "LoadLibraryW");

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

	hThread = CreateRemoteThread(hProcess, NULL, 0, startLibrary, pVallocBuff, 0, &dwTID);
	if (hThread) {
		printf("Thread created with success! Executing injector...\n");
	}
	else {
		printf("COuld not create thread!\n");
	}

	WaitForSingleObject(hProcess, 0xffffffff);


	return EXIT_SUCCESS;
}
