#include "X86_64Config.hpp"

#include <GenerateCode.hpp>

#include "X86_64Enums.hpp"

namespace x86_64 {
    void PrepareProcessor(Processor& proc) {

        switch (Options::architectureVersion) {
            case Options::AMD64_v1:
                proc.registers = {
                    {x86_64::RAX, true, true, true, false, false, false, false, true, true, true, true, false, false, false, false, 8, true, true, true, true, true, true, false, true, true, false, false, false},
                    {x86_64::RBX, true, true, true, false, false, false, false, true, true, true, true, false, false, false, false, 8, true, true, true, true, true, true, false, true, true, false, false, false},
                    {x86_64::RCX, true, true, true, false, false, false, false, true, true, true, true, false, false, false, false, 8, true, true, true, true, true, true, false, true, true, false, false, false},
                    {x86_64::RDX, true, true, true, false, false, false, false, true, true, true, true, false, false, false, false, 8, true, true, true, true, true, true, false, true, true, false, false, false},
                    {x86_64::RSI, true, true, false, false, false, false, false, true, true, true, true, false, false, false, false, 8, false, false, false, true, false, false, false, true, true, false, false, false},
                    {x86_64::RDI, true, true, false, false, false, false, false, true, true, true, true, false, false, false, false, 8, false, false, false, true, false, false, false, true, true, false, false, false},
                    {x86_64::RSP, true, true, false, false, false, false, false, true, true, true, true, false, false, false, false, 8, false, false, false, true, false, false, false, false, true, false, false, false},
                    {x86_64::RBP, true, true, false, false, false, false, false, true, true, true, true, false, false, false, false, 8, false, false, false, true, false, false, false, false, true, false, false, false},
                    {x86_64::R9, false, true, true, false, false, false, false, true, true, true, true, false, false, false, false, 8, true, true, true, true, true, true, false, true, true, false, false, false},
                    {x86_64::R10, false, true, true, false, false, false, false, true, true, true, true, false, false, false, false, 8, true, true, true, true, true, true, false, true, true, false, false, false},
                    {x86_64::R11, false, true, true, false, false, false, false, true, true, true, true, false, false, false, false, 8, true, true, true, true, true, true, false, true, true, false, false, false},
                    {x86_64::R12, false, true, true, false, false, false, false, true, true, true, true, false, false, false, false, 8, true, true, true, true, true, true, false, true, true, false, false, false},
                    {x86_64::R13, false, true, true, false, false, false, false, true, true, true, true, false, false, false, false, 8, true, true, true, true, true, true, false, true, true, false, false, false},
                    {x86_64::R14, false, true, true, false, false, false, false, true, true, true, true, false, false, false, false, 8, true, true, true, true, true, true, false, true, true, false, false, false},
                    {x86_64::R15, false, true, true, false, false, false, false, true, true, true, true, false, false, false, false, 8, true, true, true, true, true, true, false, true, true, false, false, false},
                    {x86_64::RIP, true, true, false, true, false, false, false, false, true, true, true, false, false, false, false, 8, false, false, false, true, false, false, false, false, true, false, false, false},
                    {x86_64::CS, true, true, false, false, false, true, false, false, true, false, false, false, false, false, false, 2, true, false, false, false, true, false, false, false, false, false, false, false},
                    {x86_64::DS, true, true, false, false, false, true, false, false, true, false, false, false, false, false, false, 2, true, false, false, false, true, false, false, false, false, false, false, false},
                    {x86_64::SS, true, true, false, false, false, true, false, false, true, false, false, false, false, false, false, 2, true, false, false, false, true, false, false, false, false, false, false, false},
                    {x86_64::ES, true, true, false, false, false, true, false, false, true, false, false, false, false, false, false, 2, true, false, false, false, true, false, false, false, false, false, false, false},
                    {x86_64::FS, true, true, true, false, false, true, false, false, true, false, false, false, false, false, false, 2, true, false, false, false, true, false, false, false, false, false, false, false},
                    {x86_64::GS, true, true, true, false, false, true, false, false, true, false, false, false, false, false, false, 2, true, false, false, false, true, false, false, false, false, false, false, false},
                    {x86_64::RFLAGS, true, true, false, false, false, false, true, false, false, false, true, false, false, false, false, 8, true, false, false, false, false, false, false, false, false, false, false, false},
                    {x86_64::XMM0, true, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 16, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::XMM1, true, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 16, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::XMM2, true, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 16, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::XMM3, true, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 16, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::XMM4, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 16, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::XMM5, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 16, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::XMM6, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 16, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::XMM7, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 16, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::XMM8, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 16, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::XMM9, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 16, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::XMM10, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 16, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::XMM11, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 16, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::XMM12, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 16, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::XMM13, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 16, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::XMM14, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 16, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::XMM15, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 16, true, true, true, true, true, true, true, false, false, false, true, true}

                };
                break;
            case Options::AMD64_v2:
            case Options::AMD64_v3:
                // they seem to have the same registers and only differ in instructions
                proc.registers = {
                    {x86_64::RAX, true, true, true, false, false, false, false, true, true, true, true, false, false, false, false, 8, true, true, true, true, true, true, false, true, true, false, false, false},
                    {x86_64::RBX, true, true, true, false, false, false, false, true, true, true, true, false, false, false, false, 8, true, true, true, true, true, true, false, true, true, false, false, false},
                    {x86_64::RCX, true, true, true, false, false, false, false, true, true, true, true, false, false, false, false, 8, true, true, true, true, true, true, false, true, true, false, false, false},
                    {x86_64::RDX, true, true, true, false, false, false, false, true, true, true, true, false, false, false, false, 8, true, true, true, true, true, true, false, true, true, false, false, false},
                    {x86_64::RSI, true, true, false, false, false, false, false, true, true, true, true, false, false, false, false, 8, false, false, false, true, false, false, false, true, true, false, false, false},
                    {x86_64::RDI, true, true, false, false, false, false, false, true, true, true, true, false, false, false, false, 8, false, false, false, true, false, false, false, true, true, false, false, false},
                    {x86_64::RSP, true, true, false, false, false, false, false, true, true, true, true, false, false, false, false, 8, false, false, false, true, false, false, false, false, true, false, false, false},
                    {x86_64::RBP, true, true, false, false, false, false, false, true, true, true, true, false, false, false, false, 8, false, false, false, true, false, false, false, false, true, false, false, false},
                    {x86_64::R9, false, true, true, false, false, false, false, true, true, true, true, false, false, false, false, 8, true, true, true, true, true, true, false, true, true, false, false, false},
                    {x86_64::R10, false, true, true, false, false, false, false, true, true, true, true, false, false, false, false, 8, true, true, true, true, true, true, false, true, true, false, false, false},
                    {x86_64::R11, false, true, true, false, false, false, false, true, true, true, true, false, false, false, false, 8, true, true, true, true, true, true, false, true, true, false, false, false},
                    {x86_64::R12, false, true, true, false, false, false, false, true, true, true, true, false, false, false, false, 8, true, true, true, true, true, true, false, true, true, false, false, false},
                    {x86_64::R13, false, true, true, false, false, false, false, true, true, true, true, false, false, false, false, 8, true, true, true, true, true, true, false, true, true, false, false, false},
                    {x86_64::R14, false, true, true, false, false, false, false, true, true, true, true, false, false, false, false, 8, true, true, true, true, true, true, false, true, true, false, false, false},
                    {x86_64::R15, false, true, true, false, false, false, false, true, true, true, true, false, false, false, false, 8, true, true, true, true, true, true, false, true, true, false, false, false},
                    {x86_64::RIP, true, true, false, true, false, false, false, false, true, true, true, false, false, false, false, 8, false, false, false, true, false, false, false, false, true, false, false, false},
                    {x86_64::CS, true, true, false, false, false, true, false, false, true, false, false, false, false, false, false, 2, true, false, false, false, true, false, false, false, false, false, false, false},
                    {x86_64::DS, true, true, false, false, false, true, false, false, true, false, false, false, false, false, false, 2, true, false, false, false, true, false, false, false, false, false, false, false},
                    {x86_64::SS, true, true, false, false, false, true, false, false, true, false, false, false, false, false, false, 2, true, false, false, false, true, false, false, false, false, false, false, false},
                    {x86_64::ES, true, true, false, false, false, true, false, false, true, false, false, false, false, false, false, 2, true, false, false, false, true, false, false, false, false, false, false, false},
                    {x86_64::FS, true, true, true, false, false, true, false, false, true, false, false, false, false, false, false, 2, true, false, false, false, true, false, false, false, false, false, false, false},
                    {x86_64::GS, true, true, true, false, false, true, false, false, true, false, false, false, false, false, false, 2, true, false, false, false, true, false, false, false, false, false, false, false},
                    {x86_64::RFLAGS, true, true, false, false, false, false, true, false, false, false, true, false, false, false, false, 8, true, false, false, false, false, false, false, false, false, false, false, false},
                    {x86_64::XMM0, true, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 32, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::XMM1, true, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 32, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::XMM2, true, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 32, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::XMM3, true, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 32, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::XMM4, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 32, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::XMM5, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 32, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::XMM6, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 32, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::XMM7, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 32, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::XMM8, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 32, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::XMM9, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 32, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::XMM10, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 32, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::XMM11, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 32, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::XMM12, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 32, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::XMM13, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 32, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::XMM14, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 32, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::XMM15, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 32, true, true, true, true, true, true, true, false, false, false, true, true}
                };
                break;
            case Options::AMD64_v4:
                // they seem to have the same registers and only differ in instructions
                proc.registers = {
                    {x86_64::RAX, true, true, true, false, false, false, false, true, true, true, true, false, false, false, false, 8, true, true, true, true, true, true, false, true, true, false, false, false},
                    {x86_64::RBX, true, true, true, false, false, false, false, true, true, true, true, false, false, false, false, 8, true, true, true, true, true, true, false, true, true, false, false, false},
                    {x86_64::RCX, true, true, true, false, false, false, false, true, true, true, true, false, false, false, false, 8, true, true, true, true, true, true, false, true, true, false, false, false},
                    {x86_64::RDX, true, true, true, false, false, false, false, true, true, true, true, false, false, false, false, 8, true, true, true, true, true, true, false, true, true, false, false, false},
                    {x86_64::RSI, true, true, false, false, false, false, false, true, true, true, true, false, false, false, false, 8, false, false, false, true, false, false, false, true, true, false, false, false},
                    {x86_64::RDI, true, true, false, false, false, false, false, true, true, true, true, false, false, false, false, 8, false, false, false, true, false, false, false, true, true, false, false, false},
                    {x86_64::RSP, true, true, false, false, false, false, false, true, true, true, true, false, false, false, false, 8, false, false, false, true, false, false, false, false, true, false, false, false},
                    {x86_64::RBP, true, true, false, false, false, false, false, true, true, true, true, false, false, false, false, 8, false, false, false, true, false, false, false, false, true, false, false, false},
                    {x86_64::R9, false, true, true, false, false, false, false, true, true, true, true, false, false, false, false, 8, true, true, true, true, true, true, false, true, true, false, false, false},
                    {x86_64::R10, false, true, true, false, false, false, false, true, true, true, true, false, false, false, false, 8, true, true, true, true, true, true, false, true, true, false, false, false},
                    {x86_64::R11, false, true, true, false, false, false, false, true, true, true, true, false, false, false, false, 8, true, true, true, true, true, true, false, true, true, false, false, false},
                    {x86_64::R12, false, true, true, false, false, false, false, true, true, true, true, false, false, false, false, 8, true, true, true, true, true, true, false, true, true, false, false, false},
                    {x86_64::R13, false, true, true, false, false, false, false, true, true, true, true, false, false, false, false, 8, true, true, true, true, true, true, false, true, true, false, false, false},
                    {x86_64::R14, false, true, true, false, false, false, false, true, true, true, true, false, false, false, false, 8, true, true, true, true, true, true, false, true, true, false, false, false},
                    {x86_64::R15, false, true, true, false, false, false, false, true, true, true, true, false, false, false, false, 8, true, true, true, true, true, true, false, true, true, false, false, false},
                    {x86_64::RIP, true, true, false, true, false, false, false, false, true, true, true, false, false, false, false, 8, false, false, false, true, false, false, false, false, true, false, false, false},
                    {x86_64::CS, true, true, false, false, false, true, false, false, true, false, false, false, false, false, false, 2, true, false, false, false, true, false, false, false, false, false, false, false},
                    {x86_64::DS, true, true, false, false, false, true, false, false, true, false, false, false, false, false, false, 2, true, false, false, false, true, false, false, false, false, false, false, false},
                    {x86_64::SS, true, true, false, false, false, true, false, false, true, false, false, false, false, false, false, 2, true, false, false, false, true, false, false, false, false, false, false, false},
                    {x86_64::ES, true, true, false, false, false, true, false, false, true, false, false, false, false, false, false, 2, true, false, false, false, true, false, false, false, false, false, false, false},
                    {x86_64::FS, true, true, true, false, false, true, false, false, true, false, false, false, false, false, false, 2, true, false, false, false, true, false, false, false, false, false, false, false},
                    {x86_64::GS, true, true, true, false, false, true, false, false, true, false, false, false, false, false, false, 2, true, false, false, false, true, false, false, false, false, false, false, false},
                    {x86_64::RFLAGS, true, true, false, false, false, false, true, false, false, false, true, false, false, false, false, 8, true, false, false, false, false, false, false, false, false, false, false, false},
                    {x86_64::ZMM0, true, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 64, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::ZMM1, true, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 64, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::ZMM2, true, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 64, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::ZMM3, true, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 64, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::ZMM4, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 64, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::ZMM5, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 64, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::ZMM6, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 64, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::ZMM7, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 64, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::ZMM8, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 64, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::ZMM9, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 64, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::ZMM10, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 64, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::ZMM11, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 64, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::ZMM12, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 64, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::ZMM13, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 64, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::ZMM14, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 64, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::ZMM15, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 64, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::ZMM16, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 64, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::ZMM17, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 64, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::ZMM18, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 64, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::ZMM19, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 64, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::ZMM20, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 64, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::ZMM21, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 64, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::ZMM22, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 64, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::ZMM23, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 64, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::ZMM24, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 64, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::ZMM25, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 64, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::ZMM26, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 64, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::ZMM27, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 64, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::ZMM28, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 64, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::ZMM29, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 64, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::ZMM30, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 64, true, true, true, true, true, true, true, false, false, false, true, true},
                    {x86_64::ZMM31, false, true, true, false, false, false, false, false, true, true, true, false, false, false, true, 64, true, true, true, true, true, true, true, false, false, false, true, true},
                };
                break;
            default:
                CodeGeneratorError("Invalid architecture version!");
        }
    }
}