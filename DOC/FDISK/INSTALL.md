# Installation

## Building from source
Free FDISK is confirmed to build with the following toolchains:

 - Open Watcom 1.9
 - Borland C++ 3.1
 - Turbo C++ 3.0

Beside a C Compiler the Netwide Assembler (NASM) must be installed.

If the executable packer UPX is installed and accessible via path the
generated `FDISK.EXE` gets compressed during the build process.

### Building with Open Watcom
Open Watcom is the preferred toolchain. FDISK may be built by calling
Watcom Make in the `SOURCE\FDISK` sub directory:
```
wmake -f makefile.wat
```

### Building with Borland Tools
The makefile expects Borland C++ 3.1 to be located in `C:\BORLANDC`. FDISK
may be build by calling in the `SOURCE\FDISK` sub directory:
```
make -f makefile.bor
```

If the toolchain is not located in the standard location you have to manually
specify it via
```
make -f makefile.bor -DCCBASE=<path to toolchain>
```

Building with Turbo C++ may be performed by:
```
make -f makefile.bor -DCC=tcc -DCCBASE=<path to toolchain>
```
