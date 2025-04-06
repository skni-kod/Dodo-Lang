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
        // versions note
        // versions are per https://en.wikipedia.org/wiki/X86-64#Microarchitecture_levels
        // with x86_64_v1 we have SSE and SSE2 so 16 128 bit XMM SIMD registers from SSE with double precision float support from SSE2
        // with x86_64_v2 we gave SSE3 and SSE4 which certainly add something, just have to check what, probably instructions
        // with x86_64_v3 we have AVX and AVX2 so XMM0-15 are extended with 256 bit YMM0-15 SIMD registers
        // with x86_64_v4 we have AVX512 extending YMM0-15 with 512 bit ZMM0-15 SIMD registers, and we also get XMM/YMM/ZMM 16-31
        // when APX is actually introduced it might be added as x86_64_v5 with 64 bit r16-31 general purpose registers

        // x86-64_v1 registers

        RAX = 0, EAX = 0, AX = 0, AL = 0, // 8 - 64 bit - general purpose, Accumulator
        RBX = 1, EBX = 1, BX = 1, BL = 1, // 8 - 64 bit - general purpose, Base
        RCX = 2, ECX = 2, CX = 2, CL = 2, // 8 - 64 bit - general purpose, Counter
        RDX = 3, EDX = 3, DX = 3, DL = 3, // 8 - 64 bit - general purpose, Data, RAX extension for double width operations
        RSI = 4, ESI = 4, SI = 4, SIL = 4, // 8 - 64 bit - Source Index
        RDI = 5, EDi = 5, DI = 5, DIL = 5, // 8 - 64 bit - Destination Index
        RSP = 6, ESP = 6, SP = 6, SPL = 6, // 8 - 64 bit - Stack Pointer
        RBP = 7, EBP = 7, BP = 7, BPL = 7, // 8 - 64 bit - Base Pointer
        R8 = 8, R8D = 8, R8W = 8, R8B = 8, // 8 - 64 bit - general purpose
        R9 = 9, R9D = 9, R9W = 9, R9B = 9, // 8 - 64 bit - general purpose
        R10 = 10, R10D = 10, R10W = 10, R10B = 10, // 8 - 64 bit - general purpose
        R11 = 11, R11D = 11, R11W = 11, R11B = 11, // 8 - 64 bit - general purpose
        R12 = 12, R12D = 12, R12W = 12, R12B = 12, // 8 - 64 bit - general purpose
        R13 = 13, R13D = 13, R13W = 13, R13B = 13, // 8 - 64 bit - general purpose
        R14 = 14, R14D = 14, R14W = 14, R14B = 14, // 8 - 64 bit - general purpose
        R15 = 15, R15D = 15, R15W = 15, R15B = 15, // 8 - 64 bit - general purpose
        RIP = 16, EIP = 16, IP = 16, // 16 - 64 bit - Instruction Pointer
        CS = 17, // 16 bit - Code Segment
        DS = 18, // 16 bit - Data Segment
        SS = 19, // 16 bit - Stack Segment
        ES = 20, // 16 bit - Extra Segment, for string operations
        FS = 21, // 16 bit - general purpose Segment
        GS = 22, // 16 bit - general purpose Segment
        RFLAGS = 23, // 64 bit - Register FLAGS

        // registers like the control ones should not be needed, right?
        // registers introduced later, mixed with each other due to extensions over time

        ZMM0 = 24, YMM0 = 24, XMM0 = 24,
        ZMM1 = 25, YMM1 = 25, XMM1 = 25,
        ZMM2 = 26, YMM2 = 26, XMM2 = 26,
        ZMM3 = 27, YMM3 = 27, XMM3 = 27,
        ZMM4 = 28, YMM4 = 28, XMM4 = 28,
        ZMM5 = 29, YMM5 = 29, XMM5 = 29,
        ZMM6 = 30, YMM6 = 30, XMM6 = 30,
        ZMM7 = 31, YMM7 = 31, XMM7 = 31,
        ZMM8 = 32, YMM8 = 32, XMM8 = 32,
        ZMM9 = 33, YMM9 = 33, XMM9 = 33,
        ZMM10 = 34, YMM10 = 34, XMM10 = 34,
        ZMM11 = 35, YMM11 = 35, XMM11 = 35,
        ZMM12 = 36, YMM12 = 36, XMM12 = 36,
        ZMM13 = 37, YMM13 = 37, XMM13 = 37,
        ZMM14 = 38, YMM14 = 38, XMM14 = 38,
        ZMM15 = 39, YMM15 = 39, XMM15 = 39,
        ZMM16 = 40, YMM16 = 40, XMM16 = 40,
        ZMM17 = 41, YMM17 = 41, XMM17 = 41,
        ZMM18 = 42, YMM18 = 42, XMM18 = 42,
        ZMM19 = 43, YMM19 = 43, XMM19 = 43,
        ZMM20 = 44, YMM20 = 44, XMM20 = 44,
        ZMM21 = 45, YMM21 = 45, XMM21 = 45,
        ZMM22 = 46, YMM22 = 46, XMM22 = 46,
        ZMM23 = 47, YMM23 = 47, XMM23 = 47,
        ZMM24 = 48, YMM24 = 48, XMM24 = 48,
        ZMM25 = 49, YMM25 = 49, XMM25 = 49,
        ZMM26 = 50, YMM26 = 50, XMM26 = 50,
        ZMM27 = 51, YMM27 = 51, XMM27 = 51,
        ZMM28 = 52, YMM28 = 52, XMM28 = 52,
        ZMM29 = 53, YMM29 = 53, XMM29 = 53,
        ZMM30 = 54, YMM30 = 54, XMM30 = 54,
        ZMM31 = 55, YMM31 = 55, XMM31 = 55,

        // APX additions, probably won't be added before this project dies of old age

        R16 = 56, R16D = 56, R16W = 56, R16B = 56,
        R17 = 57, R17D = 57, R17W = 57, R17B = 57,
        R18 = 58, R18D = 58, R18W = 58, R18B = 58,
        R19 = 59, R19D = 59, R19W = 59, R19B = 59,
        R20 = 60, R20D = 60, R20W = 60, R20B = 60,
        R21 = 61, R21D = 61, R21W = 61, R21B = 61,
        R22 = 62, R22D = 62, R22W = 62, R22B = 62,
        R23 = 63, R23D = 63, R23W = 63, R23B = 63,
        R24 = 64, R24D = 64, R24W = 64, R24B = 64,
        R25 = 65, R25D = 65, R25W = 65, R25B = 65,
        R26 = 66, R26D = 66, R26W = 66, R26B = 66,
        R27 = 67, R27D = 67, R27W = 67, R27B = 67,
        R28 = 68, R28D = 68, R28W = 68, R28B = 68,
        R29 = 69, R29D = 69, R29W = 69, R29B = 69,
        R30 = 70, R30D = 70, R30W = 70, R30B = 70,
        R31 = 71, R31D = 71, R31W = 71, R31B = 71,
    };

    // numbers of bits at which RFLAGS flags start (IOPL has 2 bits!)
    enum RFLAGSBits {
        BitCF   = 0,  // Carry Flag
        BitPF   = 2,  // Parity Flag
        BitAF   = 4,  // Auxiliary carry Flag
        BitZF   = 6,  // Zero Flag
        BitSF   = 7,  // Sign Flag
        BitTF   = 8,  // Trap Flag
        BitIF   = 9,  // Interrupt enable Flag
        BitDF   = 10, // Direction Flag
        BitOF   = 11, // Overflow Flag
        BitIOPL = 12, // I/O Privilege Level flag
        BitNT   = 14, // Nested Task flag
        BitRF   = 16, // Resume Flag
        BitVM   = 17, // Virtual-8086 Mode flag
        BitAC   = 18, // Alignment Check / Access Control flag
        BitVIF  = 19, // Virtual Interrupt Flag
        BitVIP  = 20, // Virtual Interrupt Pending flag
        BitID   = 21  // ID flag
    };

    // Masks that can be used to get flags in RFLAGS
    enum RFLAGSMasks {
        MaskCF   = 0x0000000000000001, // Carry Flag
        MaskPF   = 0x0000000000000004, // Parity Flag
        MaskAF   = 0x0000000000000010, // Auxiliary carry Flag
        MaskZF   = 0x0000000000000040, // Zero Flag
        MaskSF   = 0x0000000000000080, // Sign Flag
        MaskTF   = 0x0000000000000100, // Trap Flag
        MaskIF   = 0x0000000000000200, // Interrupt enable Flag
        MaskDF   = 0x0000000000000400, // Direction Flag
        MaskOF   = 0x0000000000000800, // Overflow Flag
        MaskIOPL = 0x0000000000003000, // I/O Privilege Level flag
        MaskNT   = 0x0000000000004000, // Nested Task flag
        MaskRF   = 0x0000000000010000, // Resume Flag
        MaskVM   = 0x0000000000020000, // Virtual-8086 Mode flag
        MaskAC   = 0x0000000000040000, // Alignment Check / Access Control flag
        MaskVIF  = 0x0000000000080000, // Virtual Interrupt Flag
        MaskVIP  = 0x0000000000100000, // Virtual Interrupt Pending flag
        MaskID   = 0x0000000000200000  // ID flag
    };
}

#endif //X86_64ENUMS_HPP
