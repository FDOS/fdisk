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
FDISK may be built by calling Borland Make in the `SOURCE\FDISK` sub
directory. It defaults to Borland C++ (bcc):
```
make -f makefile.bor
```

I you want to compile with Turbo C++ (tcc) you must specify it:
```
make -f makefile.bor -DCC=tcc
```

### Building a release version
To build a release version to get rid of the "NON-RELEASE BUILD" nag invoke:
```
wmake -f makefile.wat RELEASE=1
```
or the Borland Tools equivalent.

To build a FreeDOS FDISK branded release:
```
wmake -f makefile.wat RELEASE=1 FREEDOS=1
```
