#ifndef X86_64ENUMS_HPP
#define X86_64ENUMS_HPP

namespace x86_64 {

    // the progression will be something around
    // - AMD64/IA64 with SSE extension
    // - possibly split into SSE, SSE2, SSE3, SSE4
    // - AVX
    // - AVX2
    // - AVX512
    // - APX
    // I'm probably not going to entertain the idea of x87 FPU usage,
    // this compiler probably isn't going to target anything older than 64 bit with SSE
    enum Registers {
        RAX = 0, EAX = 0, AX = 0, AL = 0,
        RBX = 1, EBX = 1, BX = 1, BL = 1,
        RCX = 2, ECX = 2, CX = 2, CL = 2,
        RDX = 3, EDX = 3, DX = 3, DL = 3,
        RSI = 4, ESI = 4, SI = 4, SIL = 4,
        RDI = 5, EDi = 5, DI = 5, DIL = 5,
        RSP = 6, ESP = 6, SP = 6, SPL = 6,
        RBP = 7, EBP = 7, BP = 7, BPL = 7,
        R8 = 8, R8D = 8, R8W = 8, R8B = 8,
        R9 = 9, R9D = 9, R9W = 9, R9B = 9,
        R10 = 10, R10D = 10, R10W = 10, R10B = 10,
        R11 = 11, R11D = 11, R11W = 11, R11B = 11,
        R12 = 12, R12D = 12, R12W = 12, R12B = 12,
        R13 = 13, R13D = 13, R13W = 13, R13B = 13,
        R14 = 14, R14D = 14, R14W = 14, R14B = 14,
        R15 = 15, R15D = 15, R15W = 15, R15B = 15,
        RIP = 16, EIP = 16, IP = 16,
        // versions note
        // versions are per https://en.wikipedia.org/wiki/X86-64#Microarchitecture_levels
        // with x86_64_v1 we have SSE and SSE2 so 16 128 bit XMM SIMD registers from SSE with double precision float support from SSE2
        // with x86_64_v2 we gave SSE3 and SSE4 which certainly add something, just have to check what, probably instructions
        // with x86_64_v3 we have AVX and AVX2 so XMM0-15 are extended with 256 bit YMM0-15 SIMD registers
        // with x86_64_v4 we have AVX512 extending YMM0-15 with 512 bit ZMM0-15 SIMD registers, and we also get XMM/YMM/ZMM 16-31
        // when APX is actually introduced it might be added as x86_64_v5 with 64 bit r16-31 general purpose registers
        ZMM0 = 17, YMM0 = 17, XMM0 = 17,
        ZMM1 = 18, YMM1 = 18, XMM1 = 18,
        ZMM2 = 19, YMM2 = 19, XMM2 = 19,
        ZMM3 = 20, YMM3 = 20, XMM3 = 20,
        ZMM4 = 21, YMM4 = 21, XMM4 = 21,
        ZMM5 = 22, YMM5 = 22, XMM5 = 22,
        ZMM6 = 23, YMM6 = 23, XMM6 = 23,
        ZMM7 = 24, YMM7 = 24, XMM7 = 24,
        ZMM8 = 25, YMM8 = 25, XMM8 = 25,
        ZMM9 = 26, YMM9 = 26, XMM9 = 26,
        ZMM10 = 27, YMM10 = 27, XMM10 = 27,
        ZMM11 = 28, YMM11 = 28, XMM11 = 28,
        ZMM12 = 29, YMM12 = 29, XMM12 = 29,
        ZMM13 = 30, YMM13 = 30, XMM13 = 30,
        ZMM14 = 31, YMM14 = 31, XMM14 = 31,
        ZMM15 = 32, YMM15 = 32, XMM15 = 32,
        ZMM16 = 33, YMM16 = 33, XMM16 = 33,
        ZMM17 = 34, YMM17 = 34, XMM17 = 34,
        ZMM18 = 35, YMM18 = 35, XMM18 = 35,
        ZMM19 = 36, YMM19 = 36, XMM19 = 36,
        ZMM20 = 37, YMM20 = 37, XMM20 = 37,
        ZMM21 = 38, YMM21 = 38, XMM21 = 38,
        ZMM22 = 39, YMM22 = 39, XMM22 = 39,
        ZMM23 = 40, YMM23 = 40, XMM23 = 40,
        ZMM24 = 41, YMM24 = 41, XMM24 = 41,
        ZMM25 = 42, YMM25 = 42, XMM25 = 42,
        ZMM26 = 43, YMM26 = 43, XMM26 = 43,
        ZMM27 = 44, YMM27 = 44, XMM27 = 44,
        ZMM28 = 45, YMM28 = 45, XMM28 = 45,
        ZMM29 = 46, YMM29 = 46, XMM29 = 46,
        ZMM30 = 47, YMM30 = 47, XMM30 = 47,
        ZMM31 = 48, YMM31 = 48, XMM31 = 48,
        R16 = 49, R16D = 49, R16W = 49, R16B = 49,
        R17 = 50, R17D = 50, R17W = 50, R17B = 50,
        R18 = 51, R18D = 51, R18W = 51, R18B = 51,
        R19 = 52, R19D = 52, R19W = 52, R19B = 52,
        R20 = 53, R20D = 53, R20W = 53, R20B = 53,
        R21 = 54, R21D = 54, R21W = 54, R21B = 54,
        R22 = 55, R22D = 55, R22W = 55, R22B = 55,
        R23 = 56, R23D = 56, R23W = 56, R23B = 56,
        R24 = 57, R24D = 57, R24W = 57, R24B = 57,
        R25 = 58, R25D = 58, R25W = 58, R25B = 58,
        R26 = 59, R26D = 59, R26W = 59, R26B = 59,
        R27 = 60, R27D = 60, R27W = 60, R27B = 60,
        R28 = 61, R28D = 61, R28W = 61, R28B = 61,
        R29 = 62, R29D = 62, R29W = 62, R29B = 62,
        R30 = 63, R30D = 63, R30W = 63, R30B = 63,
        R31 = 64, R31D = 64, R31W = 64, R31B = 64,
    };
}

#endif //X86_64ENUMS_HPP
