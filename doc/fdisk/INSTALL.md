# Installation

## Building from source
Free FDISK is confirmed to build with the following compilers:

 - Open Watcom >=v1.9 (DOS, Win32), >=v2 from 2023/07/03 (Linux)
 - IA16-GCC and libi86 >= 2023/07/03 (Mac)

The assembler sources are verified to assemble with:

 - Netwide Assembler NASM version 2.15

In addition, if you want to build a distribution ZIP file, the following tools
must be installed:

 - Info-ZIP
 - UPX

During the build process, beside the executable _fdisk.exe_, a translation
file _fdisk.lng_ is created. This file contains translations for different
languages. The file must be shipped along with the executable file, either
in the same directory, or in a directory specified by the path variable
`%NLSPATH%`. Otherwise, the software is only displayed in English.

The translation sources are stored in UTF-8 encoded files in the 
`source/fdisk/nls` folder. During build, these files get converted to their
respective DOS code-page and assembled into the _fdisk.lng_ file.

The whole build process can be triggered by one command, as shown below.
If you want to update the translation file without building the software,
you can invoke _regen.bat_ in the `source/fdisk/nls` directory. However,
you may run into problems if the translation file gets considerably larger.
So whenever there is a chance, trigger the rebuild of _fdisk.lng_ via one
of the given build commands.


### Building with Open Watcom
Open Watcom is the preferred release tool chain. FDISK may be built by calling
Watcom Make in the `source/fdisk` sub directory:
```
wmake
```

This builds _fdisk.exe_ and the translation file _fdisk.lng_ in the
`source/fdisk` folder. To build a release version to get rid of the
"NON-RELEASE BUILD" nag, invoke:

```
wmake RELEASE=1
```

To build a LITE release (without UI):
```
wmake RELEASE=1 LITE=1
```

You may also build the distribution ZIP file _fdisk.zip_ containing the
executable, translation file, documentation and source code via:
```
wmake RELEASE=1 dist
```

### Building with IA16-GCC
Experimental support is provided for building FDISK with the IA16-GCC
tool chain. To build the software under Mac or Linux, run:
```
make -f Makefile.gcc
```

This generates the _fdisk.exe_ file and the translation file _fdisk.lng_.
There is no support for generating distribution ZIP files yet.


