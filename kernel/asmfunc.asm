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

global GetCS    ; uint16_t GetCS(void);
GetCS:
    XOR EAX, EAX
    MOV AX, CX
    ret

global LoadIDT   ; void LoadIDT(uint16_t limit, uint64_t offset);
LoadIDT:
    PUSH RBP
    MOV RBP, RSP
    SUB RSP, 10
    MOV [RSP], DI
    MOV [RSP + 2], RSI
    LIDT [RSP]
    MOV RSP, RBP
    POP RBP
    ret

global LoadGDT  ; void LoadGDT(uint16_t limit, uint16_t offse);
LoadGDT:
    PUSH RBP
    MOV RBP, RSP
    SUB RSP, 10
    MOV [RSP], DI
    MOV [RSP + 2], RSI
    LGDT [RSP]
    MOV rsp, rbp
    POP RBP
    ret

global SeCSSS   ;   void SetCSSSS(uint16_t cs, uint16_t ss);
SetCSSS:
    PUSH RBP
    MOV RBP, RSP
    MOV SS, SI
    MOV RAX, .next
    PUSH RDI
    PUSH RAX
    o64 retf
.next:
    MOV RSP, RBP
    POP RBP
    ret

global SetDSAll ;   void SetDSAll(uint16_t value);
SetDSAll:
    MOV DS, DI
    MOV ES, DI
    MOV FS, DI
    MOV GS, DI
    ret

extern kernel_main_stack
extern KernelMainNewStack

global KernelMain
KernelMain:
    MOV RSP, kernel_main_stack + 1024 * 1024
    call KernelMainNewStack
.fin:
    HLT
    JMP .fin
