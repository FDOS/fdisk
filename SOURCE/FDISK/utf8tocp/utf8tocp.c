/*
 * utf8tocp Copyright (C) 2013-2020 Mateusz Viste
 * converts UTF-8 text files to other codepages, as well as the other way around.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * See the license.txt file for all licensing details.
 */

#include <stdio.h>

#define pVer "0.9.5"
#define pDate "2013-2020"


/* returns 0 if s1 and s2 strings are equal, non-zero otherwise */
static int striseq(char *s1, char *s2) {
  while (*s1 == *s2) {
    if (*s1 == 0) return(0);
    s1++;
    s2++;
  }
  return(-1);
}


static void cplist(void) {
  puts(" encid   codepage description\n"
       " ---------------------------------------------------------------------\n"
       "  437    Original IBM PC codepage 437\n"
       "  775    CP 775 (a.k.a. 'MS-DOS Baltic Rim')\n"
       "  808    CP 808 (like 866, but with euro symbol)\n"
       "  850    CP 850 (a.k.a. 'Latin 1')\n"
       "  852    CP 852 (a.k.a. 'Latin 2')\n"
       "  857    CP 857 ('MS-DOS Turkish')");
  puts("  858    CP 858 (like 850, but with euro symbol)\n"
       "  866    CP 866\n"
       " 1250    Windows-1250 codepage\n"
       " 1252    Windows-1252 codepage\n"
       "  kam    Kamenicky encoding (a.k.a. CP867 or CP895)\n"
       "  maz    Mazovia (a.k.a. CP667, CP790 or CP991)\n");
}


static void about(void) {
  puts("utf8tocp v" pVer " Copyright (C) " pDate " Mateusz Viste\n"
       "utf8tocp converts text files between UTF-8 and other codepages, both ways.\n"
       "\n"
       "Usage: utf8tocp [-r] encid file.txt\n"
       "       utf8tocp -d encid outfile.bin\n"
       "       utf8tocp -l\n"
       "");
  puts(" -r  if specified, reverses the conversion (codepage -> UTF-8)\n"
       " -d  dumps the map table for encid into a file, output format is a sequence of\n"
       "     16-bit little-endian unicode codepoints for each codepage byte in the\n"
       "     range 128..255\n"
       " -l  displays the list of supported codepages\n"
       "\n"
       "This program is distributed under the terms of the BSD '2-clause' license.\n"
       "Read license.txt for details.   |   homepage: http://utf8tocp.sourceforge.net");
}


/* Loads appropriate encoding table - but only the differences against CP437 */
static int loadlookuptable(char *cpname, unsigned short *lookuptable) {
  if (striseq(cpname, "437") == 0) {           /* IBM CP437 */
    return(0);
  } else if (striseq(cpname, "775") == 0) {  /* CP775 - "MSDOS Baltic rim" */
    lookuptable[0x80 - 128] = 0x0106;
    lookuptable[0x83 - 128] = 0x0101;
    lookuptable[0x85 - 128] = 0x0123;
    lookuptable[0x87 - 128] = 0x0107;
    lookuptable[0x88 - 128] = 0x0142;
    lookuptable[0x89 - 128] = 0x0113;
    lookuptable[0x8a - 128] = 0x0156;
    lookuptable[0x8b - 128] = 0x0157;
    lookuptable[0x8c - 128] = 0x012b;
    lookuptable[0x8d - 128] = 0x0179;
    lookuptable[0x93 - 128] = 0x014d;
    lookuptable[0x95 - 128] = 0x0122;
    lookuptable[0x96 - 128] = 0x00a2;
    lookuptable[0x97 - 128] = 0x015a;
    lookuptable[0x98 - 128] = 0x015b;
    lookuptable[0x9b - 128] = 0x00f8;
    lookuptable[0x9d - 128] = 0x00d8;
    lookuptable[0x9e - 128] = 0x00d7;
    lookuptable[0x9f - 128] = 0x00a4;
    lookuptable[0xa0 - 128] = 0x0100;
    lookuptable[0xa1 - 128] = 0x012a;
    lookuptable[0xa3 - 128] = 0x017b;
    lookuptable[0xa4 - 128] = 0x017c;
    lookuptable[0xa5 - 128] = 0x017a;
    lookuptable[0xa6 - 128] = 0x201d;
    lookuptable[0xa7 - 128] = 0x00a6;
    lookuptable[0xa8 - 128] = 0x00a9;
    lookuptable[0xa9 - 128] = 0x00ae;
    lookuptable[0xaa - 128] = 0x00ac;
    lookuptable[0xad - 128] = 0x0141;
    lookuptable[0xb5 - 128] = 0x0104;
    lookuptable[0xb6 - 128] = 0x010c;
    lookuptable[0xb7 - 128] = 0x0118;
    lookuptable[0xb8 - 128] = 0x0116;
    lookuptable[0xbd - 128] = 0x012e;
    lookuptable[0xbe - 128] = 0x0160;
    lookuptable[0xc6 - 128] = 0x0172;
    lookuptable[0xc7 - 128] = 0x016a;
    lookuptable[0xcf - 128] = 0x017d;
    lookuptable[0xd0 - 128] = 0x0105;
    lookuptable[0xd1 - 128] = 0x010d;
    lookuptable[0xd2 - 128] = 0x0119;
    lookuptable[0xd3 - 128] = 0x0117;
    lookuptable[0xd4 - 128] = 0x012f;
    lookuptable[0xd5 - 128] = 0x0161;
    lookuptable[0xd6 - 128] = 0x0173;
    lookuptable[0xd7 - 128] = 0x016b;
    lookuptable[0xd8 - 128] = 0x017e;
    lookuptable[0xe0 - 128] = 0x00d3;
    lookuptable[0xe2 - 128] = 0x014c;
    lookuptable[0xe3 - 128] = 0x0143;
    lookuptable[0xe4 - 128] = 0x00f5;
    lookuptable[0xe5 - 128] = 0x00d5;
    lookuptable[0xe7 - 128] = 0x0144;
    lookuptable[0xe8 - 128] = 0x0136;
    lookuptable[0xe9 - 128] = 0x0137;
    lookuptable[0xea - 128] = 0x013b;
    lookuptable[0xeb - 128] = 0x013c;
    lookuptable[0xec - 128] = 0x0146;
    lookuptable[0xed - 128] = 0x0112;
    lookuptable[0xee - 128] = 0x0145;
    lookuptable[0xef - 128] = 0x2019;
    lookuptable[0xf0 - 128] = 0x00ad;
    lookuptable[0xf2 - 128] = 0x201c;
    lookuptable[0xf3 - 128] = 0x00be;
    lookuptable[0xf4 - 128] = 0x00b6;
    lookuptable[0xf5 - 128] = 0x00a7;
    lookuptable[0xf7 - 128] = 0x201e;
    lookuptable[0xfb - 128] = 0x00b9;
    lookuptable[0xfc - 128] = 0x00b3;
    return(0);
  } else if (striseq(cpname, "808") == 0) {  /* CP808 - like 866, but with Euro sign at 0xFD */
    loadlookuptable("866", lookuptable);  /* load CP866 first */
    lookuptable[128 - 253] = 0x20AC;      /* add the euro sign */
    return(0);
  } else if (striseq(cpname, "850") == 0) {  /* 850 codepage */
    lookuptable[155 - 128] = 0x00F8;
    lookuptable[157 - 128] = 0x00D8;
    lookuptable[158 - 128] = 0x00D7;
    lookuptable[169 - 128] = 0x00AE;
    lookuptable[181 - 128] = 0x00C1;
    lookuptable[182 - 128] = 0x00C2;
    lookuptable[183 - 128] = 0x00C0;
    lookuptable[184 - 128] = 0x00A9;
    lookuptable[189 - 128] = 0x00A2;
    lookuptable[190 - 128] = 0x00A5;
    lookuptable[198 - 128] = 0x00E3;
    lookuptable[199 - 128] = 0x00C3;
    lookuptable[207 - 128] = 0x00A4;
    lookuptable[208 - 128] = 0x00F0;
    lookuptable[209 - 128] = 0x00D0;
    lookuptable[210 - 128] = 0x00CA;
    lookuptable[211 - 128] = 0x00CB;
    lookuptable[212 - 128] = 0x00C8;
    lookuptable[213 - 128] = 0x0131;
    lookuptable[214 - 128] = 0x00CD;
    lookuptable[215 - 128] = 0x00CE;
    lookuptable[216 - 128] = 0x00CF;
    lookuptable[221 - 128] = 0x00A6;
    lookuptable[222 - 128] = 0x00CC;
    lookuptable[224 - 128] = 0x00D3;
    lookuptable[226 - 128] = 0x00D4;
    lookuptable[227 - 128] = 0x00D2;
    lookuptable[228 - 128] = 0x00F5;
    lookuptable[229 - 128] = 0x00D5;
    lookuptable[231 - 128] = 0x00FE;
    lookuptable[232 - 128] = 0x00DE;
    lookuptable[233 - 128] = 0x00DA;
    lookuptable[234 - 128] = 0x00DB;
    lookuptable[235 - 128] = 0x00D9;
    lookuptable[236 - 128] = 0x00FD;
    lookuptable[237 - 128] = 0x00DD;
    lookuptable[238 - 128] = 0x00AF;
    lookuptable[239 - 128] = 0x00B4;
    lookuptable[240 - 128] = 0x00AD;
    lookuptable[242 - 128] = 0x2017;
    lookuptable[243 - 128] = 0x00BE;
    lookuptable[244 - 128] = 0x00B6;
    lookuptable[245 - 128] = 0x00A7;
    lookuptable[247 - 128] = 0x00B8;
    lookuptable[249 - 128] = 0x00A8;
    lookuptable[251 - 128] = 0x00B9;
    lookuptable[252 - 128] = 0x00B3;
    return(0);
  } else if (striseq(cpname, "852") == 0) {  /* CP852 */
    lookuptable[0x85 - 128] = 0x016f;
    lookuptable[0x86 - 128] = 0x0107;
    lookuptable[0x88 - 128] = 0x0142;
    lookuptable[0x8a - 128] = 0x0150;
    lookuptable[0x8b - 128] = 0x0151;
    lookuptable[0x8d - 128] = 0x0179;
    lookuptable[0x8f - 128] = 0x0106;
    lookuptable[0x91 - 128] = 0x0139;
    lookuptable[0x92 - 128] = 0x013a;
    lookuptable[0x95 - 128] = 0x013d;
    lookuptable[0x96 - 128] = 0x013e;
    lookuptable[0x97 - 128] = 0x015a;
    lookuptable[0x98 - 128] = 0x015b;
    lookuptable[0x9b - 128] = 0x0164;
    lookuptable[0x9c - 128] = 0x0165;
    lookuptable[0x9d - 128] = 0x0141;
    lookuptable[0x9e - 128] = 0x00d7;
    lookuptable[0x9f - 128] = 0x010d;
    lookuptable[0xa4 - 128] = 0x0104;
    lookuptable[0xa5 - 128] = 0x0105;
    lookuptable[0xa6 - 128] = 0x017d;
    lookuptable[0xa7 - 128] = 0x017e;
    lookuptable[0xa8 - 128] = 0x0118;
    lookuptable[0xa9 - 128] = 0x0119;
    lookuptable[0xab - 128] = 0x017a;
    lookuptable[0xac - 128] = 0x010c;
    lookuptable[0xad - 128] = 0x015f;
    lookuptable[0xb5 - 128] = 0x00c1;
    lookuptable[0xb6 - 128] = 0x00c2;
    lookuptable[0xb7 - 128] = 0x011a;
    lookuptable[0xb8 - 128] = 0x015e;
    lookuptable[0xbd - 128] = 0x017b;
    lookuptable[0xbe - 128] = 0x017c;
    lookuptable[0xc6 - 128] = 0x0102;
    lookuptable[0xc7 - 128] = 0x0103;
    lookuptable[0xcf - 128] = 0x00a4;
    lookuptable[0xd0 - 128] = 0x0111;
    lookuptable[0xd1 - 128] = 0x0110;
    lookuptable[0xd2 - 128] = 0x010e;
    lookuptable[0xd3 - 128] = 0x00cb;
    lookuptable[0xd4 - 128] = 0x010f;
    lookuptable[0xd5 - 128] = 0x0147;
    lookuptable[0xd6 - 128] = 0x00cd;
    lookuptable[0xd7 - 128] = 0x00ce;
    lookuptable[0xd8 - 128] = 0x011b;
    lookuptable[0xdd - 128] = 0x0162;
    lookuptable[0xde - 128] = 0x016e;
    lookuptable[0xe0 - 128] = 0x00d3;
    lookuptable[0xe2 - 128] = 0x00d4;
    lookuptable[0xe3 - 128] = 0x0143;
    lookuptable[0xe4 - 128] = 0x0144;
    lookuptable[0xe5 - 128] = 0x0148;
    lookuptable[0xe6 - 128] = 0x0160;
    lookuptable[0xe7 - 128] = 0x0161;
    lookuptable[0xe8 - 128] = 0x0154;
    lookuptable[0xe9 - 128] = 0x00da;
    lookuptable[0xea - 128] = 0x0155;
    lookuptable[0xeb - 128] = 0x0170;
    lookuptable[0xec - 128] = 0x00fd;
    lookuptable[0xed - 128] = 0x00dd;
    lookuptable[0xee - 128] = 0x0163;
    lookuptable[0xef - 128] = 0x00b4;
    lookuptable[0xf0 - 128] = 0x00ad;
    lookuptable[0xf1 - 128] = 0x02dd;
    lookuptable[0xf2 - 128] = 0x02db;
    lookuptable[0xf3 - 128] = 0x02c7;
    lookuptable[0xf4 - 128] = 0x02d8;
    lookuptable[0xf5 - 128] = 0x00a7;
    lookuptable[0xf7 - 128] = 0x00b8;
    lookuptable[0xf8 - 128] = 0x00b0;
    lookuptable[0xf9 - 128] = 0x00a8;
    lookuptable[0xfa - 128] = 0x02d9;
    lookuptable[0xfb - 128] = 0x0171;
    lookuptable[0xfc - 128] = 0x0158;
    lookuptable[0xfd - 128] = 0x0159;
    return(0);
  } else if (striseq(cpname, "857") == 0) {  /* CP857 ('MS-DOS Turkish') */
    loadlookuptable("858", lookuptable);  /* load CP858 first */
    lookuptable[0x8d - 128] = 0x0131;
    lookuptable[0x98 - 128] = 0x0130;
    lookuptable[0x9e - 128] = 0x015e;
    lookuptable[0x9f - 128] = 0x015f;
    lookuptable[0xa6 - 128] = 0x011e;
    lookuptable[0xa7 - 128] = 0x011f;
    lookuptable[0xd0 - 128] = 0x00ba;
    lookuptable[0xd1 - 128] = 0x00aa;
    lookuptable[0xe8 - 128] = 0x00d7;
    lookuptable[0xec - 128] = 0x00ec;
    lookuptable[0xed - 128] = 0x00ff;
    return(0);
  } else if (striseq(cpname, "858") == 0) {  /* CP858 */
    loadlookuptable("850", lookuptable);  /* load the the CP850 first */
    lookuptable[0xd5 - 128] = 0x20AC;      /* add the euro sign */
    return(0);
  } else if (striseq(cpname, "866") == 0) {  /* CP866 */
    int xbyte;
    for (xbyte = 0 ; xbyte < 48; xbyte++) lookuptable[xbyte] = 0x0410 + xbyte;
    for (xbyte = 96 ; xbyte < 112; xbyte++) lookuptable[xbyte] = (0x0440 - 96) + xbyte;
    lookuptable[240 - 128] = 0x0401;
    lookuptable[241 - 128] = 0x0451;
    lookuptable[242 - 128] = 0x0404;
    lookuptable[243 - 128] = 0x0454;
    lookuptable[244 - 128] = 0x0407;
    lookuptable[245 - 128] = 0x0457;
    lookuptable[246 - 128] = 0x040E;
    lookuptable[247 - 128] = 0x045E;
    lookuptable[252 - 128] = 0x2216;
    lookuptable[253 - 128] = 0x00A4;
    return(0);
  } else if (striseq(cpname, "1250") == 0) { /* Windows-1250 codepage */
    loadlookuptable("1252", lookuptable); /* preload with CP1252 */
    lookuptable[131 - 128] = '?';
    lookuptable[136 - 128] = '?';
    lookuptable[140 - 128] = 0x015A;
    lookuptable[141 - 128] = 0x0164;
    lookuptable[143 - 128] = 0x0179;
    lookuptable[152 - 128] = '?';
    lookuptable[156 - 128] = 0x015B;
    lookuptable[157 - 128] = 0x0165;
    lookuptable[159 - 128] = 0x017A;
    lookuptable[161 - 128] = 0x02C7;
    lookuptable[162 - 128] = 0x02D8;
    lookuptable[163 - 128] = 0x0141;
    lookuptable[165 - 128] = 0x0104;
    lookuptable[170 - 128] = 0x015E;
    lookuptable[175 - 128] = 0x017B;
    lookuptable[178 - 128] = 0x02DB;
    lookuptable[179 - 128] = 0x0142;
    lookuptable[185 - 128] = 0x0105;
    lookuptable[186 - 128] = 0x015F;
    lookuptable[188 - 128] = 0x013D;
    lookuptable[189 - 128] = 0x02DD;
    lookuptable[190 - 128] = 0x013E;
    lookuptable[191 - 128] = 0x017C;
    lookuptable[192 - 128] = 0x0154;
    lookuptable[195 - 128] = 0x0102;
    lookuptable[197 - 128] = 0x0139;
    lookuptable[198 - 128] = 0x0106;
    lookuptable[200 - 128] = 0x010C;
    lookuptable[202 - 128] = 0x0118;
    lookuptable[204 - 128] = 0x011A;
    lookuptable[207 - 128] = 0x010E;
    lookuptable[208 - 128] = 0x0110;
    lookuptable[209 - 128] = 0x0143;
    lookuptable[210 - 128] = 0x0147;
    lookuptable[213 - 128] = 0x0150;
    lookuptable[216 - 128] = 0x0158;
    lookuptable[217 - 128] = 0x016E;
    lookuptable[219 - 128] = 0x0170;
    lookuptable[222 - 128] = 0x0162;
    lookuptable[224 - 128] = 0x0155;
    lookuptable[227 - 128] = 0x0103;
    lookuptable[229 - 128] = 0x013A;
    lookuptable[230 - 128] = 0x0107;
    lookuptable[232 - 128] = 0x010D;
    lookuptable[234 - 128] = 0x0119;
    lookuptable[236 - 128] = 0x011B;
    lookuptable[239 - 128] = 0x010F;
    lookuptable[240 - 128] = 0x0111;
    lookuptable[241 - 128] = 0x0144;
    lookuptable[242 - 128] = 0x0148;
    lookuptable[245 - 128] = 0x0151;
    lookuptable[248 - 128] = 0x0159;
    lookuptable[249 - 128] = 0x016F;
    lookuptable[251 - 128] = 0x0171;
    lookuptable[254 - 128] = 0x0163;
    lookuptable[255 - 128] = 0x02D9;
    return(0);
  } else if (striseq(cpname, "1252") == 0) { /* Windows-1252 codepage */
    lookuptable[128 - 128] = 0x20AC;
    lookuptable[129 - 128] = '?';
    lookuptable[130 - 128] = 0x201A;
    lookuptable[131 - 128] = 0x0192;
    lookuptable[132 - 128] = 0x201E;
    lookuptable[133 - 128] = 0x2026;
    lookuptable[134 - 128] = 0x2020;
    lookuptable[135 - 128] = 0x2021;
    lookuptable[136 - 128] = 0x02C6;
    lookuptable[137 - 128] = 0x2030;
    lookuptable[138 - 128] = 0x0160;
    lookuptable[139 - 128] = 0x2039;
    lookuptable[140 - 128] = 0x0152;
    lookuptable[141 - 128] = '?';
    lookuptable[142 - 128] = 0x017D;
    lookuptable[143 - 128] = '?';
    lookuptable[144 - 128] = '?';
    lookuptable[145 - 128] = 0x2018;
    lookuptable[146 - 128] = 0x2019;
    lookuptable[147 - 128] = 0x201C;
    lookuptable[148 - 128] = 0x201D;
    lookuptable[149 - 128] = 0x2022;
    lookuptable[150 - 128] = 0x2013;
    lookuptable[151 - 128] = 0x2014;
    lookuptable[152 - 128] = 0x02DC;
    lookuptable[153 - 128] = 0x2122;
    lookuptable[154 - 128] = 0x0161;
    lookuptable[155 - 128] = 0x203A;
    lookuptable[156 - 128] = 0x0153;
    lookuptable[157 - 128] = '?';
    lookuptable[158 - 128] = 0x017E;
    lookuptable[159 - 128] = 0x0178;
    lookuptable[160 - 128] = 0x00A0;
    lookuptable[161 - 128] = 0x00A1;
    lookuptable[162 - 128] = 0x00A2;
    lookuptable[163 - 128] = 0x00A3;
    lookuptable[164 - 128] = 0x00A4;
    lookuptable[165 - 128] = 0x00A5;
    lookuptable[166 - 128] = 0x00A6;
    lookuptable[167 - 128] = 0x00A7;
    lookuptable[168 - 128] = 0x00A8;
    lookuptable[169 - 128] = 0x00A9;
    lookuptable[170 - 128] = 0x00AA;
    lookuptable[171 - 128] = 0x00AB;
    lookuptable[172 - 128] = 0x00AC;
    lookuptable[173 - 128] = 0x00AD;
    lookuptable[174 - 128] = 0x00AE;
    lookuptable[175 - 128] = 0x00AF;
    lookuptable[176 - 128] = 0x00B0;
    lookuptable[177 - 128] = 0x00B1;
    lookuptable[178 - 128] = 0x00B2;
    lookuptable[179 - 128] = 0x00B3;
    lookuptable[180 - 128] = 0x00B4;
    lookuptable[181 - 128] = 0x00B5;
    lookuptable[182 - 128] = 0x00B6;
    lookuptable[183 - 128] = 0x00B7;
    lookuptable[184 - 128] = 0x00B8;
    lookuptable[185 - 128] = 0x00B9;
    lookuptable[186 - 128] = 0x00BA;
    lookuptable[187 - 128] = 0x00BB;
    lookuptable[188 - 128] = 0x00BC;
    lookuptable[189 - 128] = 0x00BD;
    lookuptable[190 - 128] = 0x00BE;
    lookuptable[191 - 128] = 0x00BF;
    lookuptable[192 - 128] = 0x00C0;
    lookuptable[193 - 128] = 0x00C1;
    lookuptable[194 - 128] = 0x00C2;
    lookuptable[195 - 128] = 0x00C3;
    lookuptable[196 - 128] = 0x00C4;
    lookuptable[197 - 128] = 0x00C5;
    lookuptable[198 - 128] = 0x00C6;
    lookuptable[199 - 128] = 0x00C7;
    lookuptable[200 - 128] = 0x00C8;
    lookuptable[201 - 128] = 0x00C9;
    lookuptable[202 - 128] = 0x00CA;
    lookuptable[203 - 128] = 0x00CB;
    lookuptable[204 - 128] = 0x00CC;
    lookuptable[205 - 128] = 0x00CD;
    lookuptable[206 - 128] = 0x00CE;
    lookuptable[207 - 128] = 0x00CF;
    lookuptable[208 - 128] = 0x00D0;
    lookuptable[209 - 128] = 0x00D1;
    lookuptable[210 - 128] = 0x00D2;
    lookuptable[211 - 128] = 0x00D3;
    lookuptable[212 - 128] = 0x00D4;
    lookuptable[213 - 128] = 0x00D5;
    lookuptable[214 - 128] = 0x00D6;
    lookuptable[215 - 128] = 0x00D7;
    lookuptable[216 - 128] = 0x00D8;
    lookuptable[217 - 128] = 0x00D9;
    lookuptable[218 - 128] = 0x00DA;
    lookuptable[219 - 128] = 0x00DB;
    lookuptable[220 - 128] = 0x00DC;
    lookuptable[221 - 128] = 0x00DD;
    lookuptable[222 - 128] = 0x00DE;
    lookuptable[223 - 128] = 0x00DF;
    lookuptable[224 - 128] = 0x00E0;
    lookuptable[225 - 128] = 0x00E1;
    lookuptable[226 - 128] = 0x00E2;
    lookuptable[227 - 128] = 0x00E3;
    lookuptable[228 - 128] = 0x00E4;
    lookuptable[229 - 128] = 0x00E5;
    lookuptable[230 - 128] = 0x00E6;
    lookuptable[231 - 128] = 0x00E7;
    lookuptable[232 - 128] = 0x00E8;
    lookuptable[233 - 128] = 0x00E9;
    lookuptable[234 - 128] = 0x00EA;
    lookuptable[235 - 128] = 0x00EB;
    lookuptable[236 - 128] = 0x00EC;
    lookuptable[237 - 128] = 0x00ED;
    lookuptable[238 - 128] = 0x00EE;
    lookuptable[239 - 128] = 0x00EF;
    lookuptable[240 - 128] = 0x00F0;
    lookuptable[241 - 128] = 0x00F1;
    lookuptable[242 - 128] = 0x00F2;
    lookuptable[243 - 128] = 0x00F3;
    lookuptable[244 - 128] = 0x00F4;
    lookuptable[245 - 128] = 0x00F5;
    lookuptable[246 - 128] = 0x00F6;
    lookuptable[247 - 128] = 0x00F7;
    lookuptable[248 - 128] = 0x00F8;
    lookuptable[249 - 128] = 0x00F9;
    lookuptable[250 - 128] = 0x00FA;
    lookuptable[251 - 128] = 0x00FB;
    lookuptable[252 - 128] = 0x00FC;
    lookuptable[253 - 128] = 0x00FD;
    lookuptable[254 - 128] = 0x00FE;
    lookuptable[255 - 128] = 0x00FF;
    return(0);
  } else if (striseq(cpname, "maz") == 0) {  /* MAZOVIA (PL) */
    lookuptable[134 - 128] = 0x0105;
    lookuptable[141 - 128] = 0x0107;
    lookuptable[143 - 128] = 0x0104;
    lookuptable[144 - 128] = 0x0118;
    lookuptable[145 - 128] = 0x0119;
    lookuptable[146 - 128] = 0x0142;
    lookuptable[149 - 128] = 0x0106;
    lookuptable[152 - 128] = 0x015A;
    lookuptable[156 - 128] = 0x0141;
    lookuptable[158 - 128] = 0x015B;
    lookuptable[160 - 128] = 0x0179;
    lookuptable[161 - 128] = 0x017B;
    lookuptable[163 - 128] = 0x00D3;
    lookuptable[164 - 128] = 0x0144;
    lookuptable[165 - 128] = 0x0143;
    lookuptable[166 - 128] = 0x017A;
    lookuptable[167 - 128] = 0x017C;
    return(0);
  } else if (striseq(cpname, "kam") == 0) { /* Kamenicky encoding (CZ) */
    lookuptable[128 - 128] = 0x010C;
    lookuptable[131 - 128] = 0x010F;
    lookuptable[133 - 128] = 0x010E;
    lookuptable[134 - 128] = 0x0164;
    lookuptable[135 - 128] = 0x010D;
    lookuptable[136 - 128] = 0x011B;
    lookuptable[137 - 128] = 0x011A;
    lookuptable[138 - 128] = 0x0139;
    lookuptable[139 - 128] = 0x00CD;
    lookuptable[140 - 128] = 0x013E;
    lookuptable[141 - 128] = 0x013A;
    lookuptable[143 - 128] = 0x00C1;
    lookuptable[145 - 128] = 0x017E;
    lookuptable[146 - 128] = 0x017D;
    lookuptable[149 - 128] = 0x00D3;
    lookuptable[150 - 128] = 0x016F;
    lookuptable[151 - 128] = 0x00DA;
    lookuptable[152 - 128] = 0x00FD;
    lookuptable[155 - 128] = 0x0160;
    lookuptable[156 - 128] = 0x013D;
    lookuptable[157 - 128] = 0x00DD;
    lookuptable[158 - 128] = 0x0158;
    lookuptable[159 - 128] = 0x0165;
    lookuptable[164 - 128] = 0x0148;
    lookuptable[165 - 128] = 0x0147;
    lookuptable[166 - 128] = 0x016E;
    lookuptable[167 - 128] = 0x00D4;
    lookuptable[168 - 128] = 0x0161;
    lookuptable[169 - 128] = 0x0159;
    lookuptable[170 - 128] = 0x0155;
    lookuptable[171 - 128] = 0x0154;
    lookuptable[173 - 128] = 0x00A7;
    return(0);
  } else { /* else it's an unknown encoding */
    return(-1);
  }
}


static int codepagelookup(long codepoint, unsigned short *lookuptable) {
  int x;
  /* if the codepoint is 7bit, don't look further, since it's valid ASCII already */
  if (codepoint < 128) return((int)codepoint);
  /* values higher than 127 must be mapped */
  for (x = 0; x < 128; x++) {
    if (lookuptable[x] == codepoint) return(x + 128);
  }
  /* ignore BOM markers */
  if (codepoint == 0xFEFFl) return(-1);
  /* if the codepoint is not present in the codepage, return a fallback character */
  return('?');
}


static unsigned short unicodelookup(int bytecode, unsigned short *lookuptable) {
  /* if the codepoint is 7bit, don't look further, since it's valid ASCII already */
  if (bytecode < 128) return(bytecode);
  /* values higher than 127 must be mapped */
  return(lookuptable[bytecode - 128]);
}


static long getNextUnicodeTokenFromFile(FILE *fd) {
  int tmpbyte, bytelen, x;
  long result = 0;
  /* read the 1st byte - this will tell us how many bytes follow */
  result = fgetc(fd);
  if (result == EOF) return(EOF);
  if ((result & 0x80) == 0) {              /* 0xxxxxxx (1 byte) */
    bytelen = 1;
  } else if ((result & 0xE0) == 0xC0) {  /* 110xxxxx (2 bytes) */
    bytelen = 2;
    result &= 0x1F;
  } else if ((result & 0xF0) == 0xE0) {  /* 1110xxxx (3 bytes) */
    bytelen = 3;
    result &= 0xF;
  } else if ((result & 0xF8) == 0xF0) {  /* 11110xxx (4 bytes) */
    bytelen = 4;
    result &= 0x7;
  } else { /* invalid UTF-8 byte */
    return('?');
  }
  /* read all following bytes */
  for (x = 1; x < bytelen; x++) {
    tmpbyte = fgetc(fd);
    tmpbyte &= 0x3F;
    result <<= 6;
    result |= tmpbyte;
  }
  return(result);
}


static void outputUnicodeToken(unsigned short codepoint) {
  if (codepoint < 0x80) {              /* single byte, same as ASCII */
    putchar(codepoint);
  } else if (codepoint < 0x800l) {    /* two bytes */
    putchar(0xC0 | ((codepoint >> 6) & 0x1F));   /* 110xxxxx */
    putchar(0x80 | (codepoint & 0x3F));          /* 10xxxxxx */
  } else {  /* three bytes */
    putchar(0xE0 | ((codepoint >> 12) & 0xF));   /* 1110xxxx */
    putchar(0x80 | ((codepoint >> 6) & 0x3F));   /* 10xxxxxx */
    putchar(0x80 | (codepoint & 0x3F));          /* 10xxxxxx */
  }
}


#define ACTION_ENCODE 0
#define ACTION_DECODE 1
#define ACTION_DUMP 2

int main(int argc, char **argv) {
  /* lookuptable is preloaded with CP437 at start */
  unsigned short lookuptable[128] = {0x00C7,0x00FC,0x00E9,0x00E2,0x00E4,0x00E0,0x00E5,0x00E7,0x00EA,0x00EB,0x00E8,0x00EF,0x00EE,0x00EC,0x00C4,0x00C5,
                           0x00C9,0x00E6,0x00C6,0x00F4,0x00F6,0x00F2,0x00FB,0x00F9,0x00FF,0x00D6,0x00DC,0x00A2,0x00A3,0x00A5,0x20A7,0x0192,
                           0x00E1,0x00ED,0x00F3,0x00FA,0x00F1,0x00D1,0x00AA,0x00BA,0x00BF,0x2310,0x00AC,0x00BD,0x00BC,0x00A1,0x00AB,0x00BB,
                           0x2591,0x2592,0x2593,0x2502,0x2524,0x2561,0x2562,0x2556,0x2555,0x2563,0x2551,0x2557,0x255D,0x255C,0x255B,0x2510,
                           0x2514,0x2534,0x252C,0x251C,0x2500,0x253C,0x255E,0x255F,0x255A,0x2554,0x2569,0x2566,0x2560,0x2550,0x256C,0x2567,
                           0x2568,0x2564,0x2565,0x2559,0x2558,0x2552,0x2553,0x256B,0x256A,0x2518,0x250C,0x2588,0x2584,0x258C,0x2590,0x2580,
                           0x03B1,0x00DF,0x0393,0x03C0,0x03A3,0x03C3,0x00B5,0x03C4,0x03A6,0x0398,0x03A9,0x03B4,0x221E,0x03C6,0x03B5,0x2229,
                           0x2261,0x00B1,0x2265,0x2264,0x2320,0x2321,0x00F7,0x2248,0x00B0,0x2219,0x00B7,0x221A,0x207F,0x00B2,0x25A0,0x00A0};
  char *filename;
  char *cpname;
  int action = ACTION_ENCODE;
  FILE *fd;

  /* I expect the program to be called with exactly two or three arguments */
  if (argc == 3) {
    cpname = argv[1];
    filename = argv[2];
  } else if (argc == 4) {
    if (striseq(argv[1], "-r") == 0) {
      action = ACTION_DECODE;
    } else if (striseq(argv[1], "-d") == 0) {
      action = ACTION_DUMP;
    } else {
      puts("ERROR: Unknown option.");
      return(3);
    }
    cpname = argv[2];
    filename = argv[3];
  } else if ((argc == 2) && (striseq(argv[1], "-l") == 0)) {
    cplist();
    return(0);
  } else {
    about();
    return(0);
  }

  /* resolve the codepage name to a numerical id */
  if (loadlookuptable(cpname, lookuptable) != 0) {
    puts("ERROR: Unknown target encoding.");
    return(1);
  }

  /* is this about dumping the codepage map? */
  if (action == ACTION_DUMP) {
    size_t i;
    /* abort if output file exists */
    fd = fopen(filename, "rb");
    if (fd != NULL) {
      fclose(fd);
      puts("ERROR: output file already exists.");
      return(1);
    }
    /* create output file */
    fd = fopen(filename, "wb");
    if (fd == NULL) {
      puts("ERROR: output file could not be created.");
      return(1);
    }
    /* dump the conversion table (little-endian format) */
    for (i = 0; i < 128; i++) {
      unsigned char c;
      c = lookuptable[i] & 0xff;
      fwrite(&c, 1, 1, fd);
      c = lookuptable[i] >> 8;
      fwrite(&c, 1, 1, fd);
    }
    fclose(fd);
    return(0);
  }

  /* open the input file */
  fd = fopen(filename, "rb");
  if (fd == NULL) {
    puts("ERROR: Failed to open input file.");
    return(2);
  }

  /* Start converting the file */
  if (action == ACTION_ENCODE) {
    long unicodeToken;
    int cpbyte;
    for (;;) {
      unicodeToken = getNextUnicodeTokenFromFile(fd);
      if (unicodeToken == EOF) break;
      cpbyte = codepagelookup(unicodeToken, lookuptable);
      if (cpbyte < 0) continue; /* some markers must be ignored */
      putchar(cpbyte);
    }
  } else { /* reverse operation: codepage -> utf8 */
    int bytebuff;
    for (;;) {
      bytebuff = getc(fd);
      if (bytebuff == EOF) break;
      outputUnicodeToken(unicodelookup(bytebuff, lookuptable));
    }
  }

  /* close the input file */
  fclose(fd);

  return(0);
}
