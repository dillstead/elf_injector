kujawk@rpi:~/projects/scratch/new_entry$ readelf -h -l -S /bin/ls
ELF Header:
  Magic:   7f 45 4c 46 01 01 01 00 00 00 00 00 00 00 00 00 
  Class:                             ELF32
  Data:                              2's complement, little endian
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI Version:                       0
  Type:                              EXEC (Executable file)
  Machine:                           ARM
  Version:                           0x1
  Entry point address:               0x13afc
  Start of program headers:          52 (bytes into file)
  Start of section headers:          111816 (bytes into file)
  Flags:                             0x5000400, Version5 EABI, hard-float ABI
  Size of this header:               52 (bytes)
  Size of program headers:           32 (bytes)
  Number of program headers:         9
  Size of section headers:           40 (bytes)
  Number of section headers:         29
  Section header string table index: 28

Section Headers:
  [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
  [ 0]                   NULL            00000000 000000 000000 00      0   0  0
  [ 1] .interp           PROGBITS        00010154 000154 000019 00   A  0   0  1
  [ 2] .note.gnu.bu[...] NOTE            00010170 000170 000024 00   A  0   0  4
  [ 3] .note.ABI-tag     NOTE            00010194 000194 000020 00   A  0   0  4
  [ 4] .gnu.hash         GNU_HASH        000101b4 0001b4 000400 04   A  5   0  4
  [ 5] .dynsym           DYNSYM          000105b4 0005b4 0007f0 10   A  6   1  4
  [ 6] .dynstr           STRTAB          00010da4 000da4 0005b4 00   A  0   0  1
  [ 7] .gnu.version      VERSYM          00011358 001358 0000fe 02   A  5   0  2
  [ 8] .gnu.version_r    VERNEED         00011458 001458 000080 00   A  6   3  4
  [ 9] .rel.dyn          REL             000114d8 0014d8 000040 08   A  5   0  4
  [10] .rel.plt          REL             00011518 001518 000360 08  AI  5  22  4
  [11] .init             PROGBITS        00011878 001878 00000c 00  AX  0   0  4
  [12] .plt              PROGBITS        00011884 001884 000524 04  AX  0   0  4
  [13] .text             PROGBITS        00011da8 001da8 014894 00  AX  0   0  8
  [14] .fini             PROGBITS        0002663c 01663c 000008 00  AX  0   0  4
  [15] .rodata           PROGBITS        00026644 016644 004782 00   A  0   0  4
  [16] .ARM.exidx        ARM_EXIDX       0002adc8 01adc8 000008 00  AL 13   0  4
  [17] .eh_frame         PROGBITS        0002add0 01add0 000004 00   A  0   0  4
  [18] .init_array       INIT_ARRAY      0003aefc 01aefc 000004 04  WA  0   0  4
  [19] .fini_array       FINI_ARRAY      0003af00 01af00 000004 04  WA  0   0  4
  [20] .data.rel.ro      PROGBITS        0003af04 01af04 000004 00  WA  0   0  4
  [21] .dynamic          DYNAMIC         0003af08 01af08 0000f8 08  WA  6   0  4
  [22] .got              PROGBITS        0003b000 01b000 0001c0 04  WA  0   0  4
  [23] .data             PROGBITS        0003b1c0 01b1c0 00013c 00  WA  0   0  8
  [24] .bss              NOBITS          0003b300 01b2fc 001204 00  WA  0   0  8
  [25] .ARM.attributes   ARM_ATTRIBUTES  00000000 01b2fc 00002f 00      0   0  1
  [26] .gnu_debugaltlink PROGBITS        00000000 01b32b 00004c 00      0   0  1
  [27] .gnu_debuglink    PROGBITS        00000000 01b378 000034 00      0   0  4
  [28] .shstrtab         STRTAB          00000000 01b3ac 00011a 00      0   0  1
Key to Flags:
  W (write), A (alloc), X (execute), M (merge), S (strings), I (info),
  L (link order), O (extra OS processing required), G (group), T (TLS),
  C (compressed), x (unknown), o (OS specific), E (exclude),
  y (purecode), p (processor specific)

Program Headers:
  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
  EXIDX          0x01adc8 0x0002adc8 0x0002adc8 0x00008 0x00008 R   0x4
  PHDR           0x000034 0x00010034 0x00010034 0x00120 0x00120 R   0x4
  INTERP         0x000154 0x00010154 0x00010154 0x00019 0x00019 R   0x1
      [Requesting program interpreter: /lib/ld-linux-armhf.so.3]
  LOAD           0x000000 0x00010000 0x00010000 0x1add4 0x1add4 R E 0x10000
  LOAD           0x01aefc 0x0003aefc 0x0003aefc 0x00400 0x01608 RW  0x10000
  DYNAMIC        0x01af08 0x0003af08 0x0003af08 0x000f8 0x000f8 RW  0x4
  NOTE           0x000170 0x00010170 0x00010170 0x00044 0x00044 R   0x4
  GNU_STACK      0x000000 0x00000000 0x00000000 0x00000 0x00000 RW  0x10
  GNU_RELRO      0x01aefc 0x0003aefc 0x0003aefc 0x00104 0x00104 R   0x1

 Section to Segment mapping:
  Segment Sections...
   00     .ARM.exidx 
   01     
   02     .interp 
   03     .interp .note.gnu.build-id .note.ABI-tag .gnu.hash .dynsym .dynstr .gnu.version .gnu.version_r .rel.dyn .rel.plt .init .plt .text .fini .rodata .ARM.exidx .eh_frame 
   04     .init_array .fini_array .data.rel.ro .dynamic .got .data .bss 
   05     .dynamic 
   06     .note.gnu.build-id .note.ABI-tag 
   07     
   08     .init_array .fini_array .data.rel.ro .dynamic
        