# Installation

## Building from source
Free FDISK is confirmed to build with the following toolchains:

 - Open Watcom 1.9
 - Borland C++ 3.1
 - Turbo C++ 3.0

Beside a C Compiler the Netwide Assembler (NASM) must be installed.

If the executable packer UPX is installed and accessible via path the
generated `FDISK.EXE` gets compressed during the creation build process.

## Building/refreshing translation files
FDISK relies on the SvarLANG library for translations. Every language is
originally stored in a UTF-8 text file in the SOURCE\NLS directory.

Executing the REGEN.BAT script converts the original UTF-8 text files into
files with proper DOS codepages and compiles all the files inside a single
FDISK.LNG resource file. This FDISK.LNG file contains all translations to be
used by FDISK. This file must be placed in %NLSPATH% on the target system.

Along with the FDISK.LNG file, a DEFLANG.C file is also generated in FDISK'S
SOURCE directory. This file is used during FDISK's compilation process as it
stores all english strings that are used if FDISK.LNG cannot be loaded.

NOTE: Rebuilding translation resources requires the UTF8TOCP tool to be
installed and accessible in %PATH%.

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
