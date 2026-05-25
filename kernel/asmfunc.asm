; asmfunc.asm
; System V AMD64 Calling Convention
; Registers; RDI, RSI, RDX, RCX, R8, R9

bits 64
section .text

global IoOut32  ; void IoOut32(uint16_t addr, iom32_t data);
IoOut32:
    MOV DX, DI      ; DX = addr
    MOV EAX, ESI    ; EAX = data
    OUT DX, EAX
    ret

global IoIn32   ; uint32_t IoIn32(uint16_t addr);
IoIn32:
    MOV DX, DI      ; DX = addr
    IN EAX ,DX
    ret

global ReadCR3  ; uintptr_t ReadCR3();
ReadCR3:
    MOV RAX, CR3
    ret

global SetCR3   ; void SetCR3(uint64_t value);
SetCR3:
    MOV CR3, RDI
    ret
