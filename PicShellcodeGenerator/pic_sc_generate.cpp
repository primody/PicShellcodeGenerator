#include "pic_sc_generate.h"

START

// Function that hashes a string for obfuscation. Uses a random number generated during compilation.
constexpr DWORD get_hash_from_string(CONST CHAR* string) {
	DWORD hash = 0;
	CHAR ch = 0;

	while (*string) {
		ch = *string;
		if (ch >= 0x61 && ch <= 0x7A) {
			ch = *string - 0x20;
		}
		hash += (hash * RAND_NUM + ch) & 0xffffff;
		string++;
	}
	return hash;
}

// Custom GetProcAddress()
VOID* get_proc_address(DWORD module_hash, DWORD function_hash, CHAR* module_name) {
#ifdef _WIN64
	PEB* peb = (PEB*)__readgsqword(0x60);
#else
	PEB* peb = (PEB*)__readfsdword(0x30);
#endif
	bool again = false;

	// Get the head of list
	PLIST_ENTRY list_head = &peb->Ldr->InMemoryOrderModuleList;
	PLIST_ENTRY list_current = list_head;

	// Start looping through the double linked list
	while ((list_current = list_current->Flink) != list_head) {
		PLDR_DATA_TABLE_ENTRY entry = CONTAINING_RECORD(list_current, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);

		// Get all pointers setup
		BYTE* base_address = (BYTE*)entry->DllBase;
		IMAGE_DOS_HEADER* dos_header = (IMAGE_DOS_HEADER*)base_address;
		IMAGE_NT_HEADERS* nt_header = (IMAGE_NT_HEADERS*)(base_address + dos_header->e_lfanew);

		IMAGE_DATA_DIRECTORY* data_directory = &nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
		IMAGE_EXPORT_DIRECTORY* export_directory = (IMAGE_EXPORT_DIRECTORY*)(base_address + data_directory->VirtualAddress);

		// Skips the module if it doesn't have exports. Our own executeable is always loaded first so this is needed to skip it
		if (data_directory->VirtualAddress == NULL)
			continue;

		// Check if the module is the one we are looking for
		if (get_hash_from_string((CHAR*)(base_address + export_directory->Name)) != module_hash) {
			continue;
		}

		// Get table of name addresses
		DWORD* name_rvas = (DWORD*)(base_address + export_directory->AddressOfNames);

		// Loop through all of the names and hash them and compare for the function we want
		for (DWORD i = 0; i < export_directory->NumberOfNames; ++i) {
			if (function_hash == get_hash_from_string((CHAR*)(base_address + name_rvas[i]))) {
				WORD ordinal = ((WORD*)(base_address + export_directory->AddressOfNameOrdinals))[i];
				DWORD function_rva = ((DWORD*)(base_address + export_directory->AddressOfFunctions))[ordinal];
				return base_address + function_rva;
			}
		}
	}
}


VOID WINAPI code() {
	CHAR strUser32[] = { 'u','s','e','r','3','2','.','d','l','l', '\0'};
	FIND_FUNCTION("kernel32.dll", LoadLibraryA);
	LoadLibraryA(strUser32);

	char one[] = { 'T', 'h', 'i', 's', '\0'};
	char two[] = { 'T', 'h', 'a', 't', '\0' };
	FIND_FUNCTION("user32.dll", MessageBoxA);
	MessageBoxA(NULL, one, two, MB_OK);
	return;
}
END

void main(INT argc, CHAR* argv[]) {
	HANDLE hfile;

	if (argc == 1) {
		printf("Usage: \n-s\tSave to file.\n-c\tPrint C buffer format.\n-p\tPrint Python buffer format.\n-d\tDon't run code.");
		return;
	}

	ptrdiff_t shellcode_size = (char*)code_end - (char*)code_start;

	printf("[!] Size of shellcode: %lld\n", shellcode_size);

	for (INT i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-s")) {
			DWORD bytes_written;
			char path[MAX_PATH] = { 0 };

			bytes_written = GetCurrentDirectoryA(MAX_PATH, path);

			strcat(path + bytes_written, "\\shellcode.bin");

			printf("[!] Writing shellcode to: %s\n", path);

			hfile = CreateFileA(path, GENERIC_ALL, NULL, NULL, CREATE_ALWAYS, NULL, NULL);
			if (hfile == INVALID_HANDLE_VALUE) {
				printf("Failed to write file: %d\n", GetLastError());
				return;
			}

			if (!WriteFile(hfile, &code_start, (DWORD)shellcode_size, &bytes_written, NULL)) {
				printf("Failed to write file: %d\n", GetLastError());
				CloseHandle(hfile);
				return;
			}

			printf("[!] Successfully wrote to file.\n");
			CloseHandle(hfile);
		} else if (!strcmp(argv[i], "-c")) {
			printf("[!] Printing C array.\n");
			printf("unsigned char payload_buffer[]=\n\t\"");

			int cnt = 0;
			unsigned char* buff = (unsigned char*)code_start;

			for (DWORD i = 0; i < (DWORD)shellcode_size; i++) {
				if (cnt == 15) {
					cnt = 0; printf("\"\n\t\"");
				}
				cnt++;
				printf("\\x%02x", buff[i]);
			}
			printf("\";\n\n");
			continue;
		} else if (!strcmp(argv[i], "-p")) {
			printf("[!] Printing Python array.\n");
			printf("payload_buffer= b'");
			int cnt = 0;
			unsigned char* buff = (unsigned char*)code_start;

			for (DWORD i = 0; i < (DWORD)shellcode_size; i++) {
				if (cnt == 15) {
					cnt = 0; printf("' \\\n\t\tb'");
				}
				cnt++;
				printf("\\x%02x", buff[i]);
			}
			printf("'\n\n");
			continue;
		} else if (!strcmp(argv[i], "-r")) {
			code();
		}
	}

	return;
}