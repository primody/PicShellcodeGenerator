#define RAND_NUM 1951 
// Do not remove the above line. This is generated at compile time.

#pragma once
#include <Windows.h>
#include <winternl.h>
#include <stdio.h>

#pragma warning(disable:4996)

// Macro to get function pointer. If DLL is not loaded, it will attempt to load.
#define FIND_FUNCTION(module_name, function_name) \
    constexpr DWORD module_hash##function_name = get_hash_from_string(module_name); \
    constexpr DWORD function_hash##function_name = get_hash_from_string(#function_name); \
    typedef decltype(function_name) ptr_##function_name; \
    ptr_##function_name *##function_name = (ptr_##function_name *)get_proc_address(module_hash##function_name, function_hash##function_name, module_name); \

#ifdef _WIN64
#define START VOID code_start() {code();}
#else
#define START __declspec(naked) VOID code_start() { __asm {jmp code}};
#endif

#ifdef _WIN64
#define END VOID code_end() {};
#else
#define END __declspec(naked) VOID code_end() { };
#endif

VOID WINAPI code();
