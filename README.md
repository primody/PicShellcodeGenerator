# Pic Shellcode Generator
Pic Shellcode Generator is a Visual Studio project that allows you to use C/C++ to create functionality and build to shellcode easily. 

## Features
- Written in C++
- Each compile hashes DLL and function names differently
- Write shellcode without knowing Assembly
- Can be extended easily

## How To Compile
1. Open the Solution file in Visual Studio
2. Build solution

## Possible Future Changes
- Custom LoadLibrary()
- Compress/encrypt stub wrapper
- Random function selection
- ???

## How To Use
1. Open `pic_sc_generate.cpp` and find the function `code()`.
2. Write your C/C++ code in this function.
3. If you add functions, make sure they are created above `code()`.
4. If you need to call Win32 functions you will need to get address of function. This is done using the macro `FIND_FUNCTION("dll.name", FunctionNameA)`. A example is provided in the `code()` already.
5. Each compile runs a prebuild event that will generate a random number used for hashing DLL and function names.
6. After compilation run `pic_sc_generate.exe`. No arguments will output usage information.
7. If save option is used it will save the shellcode ready to be used.
8. Run `PicShellcodeGenerator.exe` with no arguments for more info.

## Example
     1| VOID WINAPI code() {
	 2|     CHAR strUser32[] = { 'u','s','e','r','3','2','.','d','l','l',0 };
	 3|     FIND_FUNCTION("kernel32.dll", LoadLibraryA);
	 4|     LoadLibraryA(strUser32);
     5|
	 6|     char one[] = { 'T', 'h', 'i', 's', '\0'};
	 7|     char two[] = { 'T', 'h', 'a', 't', '\0' };
	 8|     FIND_FUNCTION("user32.dll", MessageBoxA);
	 9|     MessageBoxA(NULL, one, two, MB_OK);
    10|     return;
    11| }

Line 2 is a char array showing how we can create strings without showing up in `strings.exe`.
Line 3 is the format when you need to use a Windows function, such as `LoadLibraryA`.
Line 4 is now referencing our function pointer created from line 3 and loading `user32.dll` so we can get a function pointer. 
Line 6 and 7 are char arrays again.
Line 8 is now getting pointer to `MessageBoxA`.
Line 9 calls `MessageBoxA` and a message box is displayed.

## Notes
Function pointers are only valid in the scope they are created. It would be worth creating a global struct to store function pointers if you want to call from other functions.

A reference to the `code()` needs to be present else it will get optimized away. To prevent this I added a command line argument `-r` that will run the code. This will keep the code building correctly.

I have not actually used this for anything more than a hello world program. I am not sure of its limitations beyond that but hope to see what all can be made from it.

## Development
Want to contribute? Great! Just make changes and send a pull request.

## Credits
During a project I was working on I needed the ability to create shellcode easily. After lots of searching I came across a Reddit post that took me to this repository. After going over it I wanted to make a few changes to it. This repository is my version that I will continue to work on. The original can be found here: https://github.com/jackullrich/ShellcodeStdio

