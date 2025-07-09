### QMG

*Quramsoft Qmage* is a proprietary graphics format used in Samsung devices, notably for startup and shutdown animations.

This is my attempt at reversing the codec, it's totally 1009% 1:1 accurate amazing beautiful as you will see in the demo below.

---

### Demo · SM-S901N, SKTelecom

**Custom decoder output**

https://github.com/user-attachments/assets/71526c4d-24a5-4fd7-82f0-c6a83a2d54f4

 ...Looks *somewhat* familar... (It's just the color palette data for every single frame squashed into a video)

**Original**

https://github.com/user-attachments/assets/5226d20e-e0ea-4971-b813-a1f2eac9c858

---

### Sources and Tools
- [MMS Exploit - Project Zero](https://googleprojectzero.blogspot.com/2020/07/mms-exploit-part-1-introduction-to-qmage.html) - Basic information on QG images, not QM animations. Still quite applicable.
- [Samsung Theme Designer](https://www.softpedia.com/get/Mobile-Phone-Tools/Others/Samsung-Theme-Designer.shtml) - Created a bunch of blank QMGs to experiement with. Old and bad software.
- [simg2img and ext2explore](https://github.com/anestisb/android-simg2img) - Tools I used to extract QMGs from Samsung firmware.
- [Ghidra](https://github.com/NationalSecurityAgency/ghidra) - I can't afford IDA. Disassembled `libQmageDecoder.so` from a random firmware. Not too much information beyond symbols.
- [PixelViewer](https://github.com/carina-studio/PixelViewer) - Image viewer, opened QMGs in RGB565 mode. Found per-frame color palette data. 
- [ImHex](https://imhex.werwolv.net/) - Hex editor, visualized QMG headers/bodies with the pattern below.
```
import std.mem;
import type.magic;
import type.size;

struct Header {
    u16 magic;
    u32 unknown1;
    u16 width;
    u16 height;
    u48 unknown2;
    u16 total;
    u16 current;
    u32 unknown3;
    u32 bodySize;
    u32 footerSize;
    char body[bodySize - 32];
};

std::mem::MagicSearch<"QM", Header> header @ std::mem::base_address();
```

Mm have fun I'm not sure if I did
