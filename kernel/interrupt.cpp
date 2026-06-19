#include "interrupt.hpp"

std::array<InterruptDescriptor, 256> idt;

void SetIDTEntry(InterruptDescriptor &desc, InterruptDescriptorAttribute attr, uint64_t offset,
                 uint16_t segment_selector) {
    desc.attr = attr;
    desc.offset_low = offset & 0xffffu;
    desc.offset_middle = (offset >> 16) & 0xffffu;
    desc.offset_high = offset >> 32;
    desc.segment_selector = segment_selector;
}

void NotifyEndOfInterrupt() {
    volatile auto end_of_interrupt = reinterpret_cast<uint32_t *>(0xfee000b0);
    *end_of_interrupt = 0;
}


void SetupBasicExceptionHandler(InterruptDescriptorAttribute attr, uint64_t WithError, uint64_t NoError, uint16_t cs) {
    SetIDTEntry(idt[0], attr, NoError, cs); // Divide Error Exception (#DE)
    SetIDTEntry(idt[1], attr, NoError, cs); // Debug Exception (#DB)
    SetIDTEntry(idt[2], attr, NoError, cs); // NMI Interrupt
    SetIDTEntry(idt[3], attr, NoError, cs); // Break Point Exception (#BP)
    SetIDTEntry(idt[4], attr, NoError, cs); // Overflow Exception (#OF)
    SetIDTEntry(idt[5], attr, NoError, cs); // BOUND Range Exceeded Exception (#BR)
    SetIDTEntry(idt[6], attr, NoError, cs); // Invalid Opcode Exception (#UD)
    SetIDTEntry(idt[7], attr, NoError, cs); // Device Not Available Exception (#NM)
    SetIDTEntry(idt[8], attr, WithError, cs); // Double Fault Exception (#DF)
    SetIDTEntry(idt[9], attr, NoError, cs); // Coprocessor Segment Overrun
    SetIDTEntry(idt[10], attr, WithError, cs); // Invalid TSS Exception (#TS)
    SetIDTEntry(idt[11], attr, WithError, cs); // Segment Not Present (#NP)
    SetIDTEntry(idt[12], attr, WithError, cs); // Stack Fault Exception (#SS)
    SetIDTEntry(idt[13], attr, WithError, cs); // General Protection Exception (#GP)
    SetIDTEntry(idt[14], attr, WithError, cs); // Page-Fault Exception (#PF)
    SetIDTEntry(idt[16], attr, WithError, cs); // x87 FPU Floating-Point Error (#MF)
    SetIDTEntry(idt[17], attr, WithError, cs); // Alignment Check Exception (#AC)
    SetIDTEntry(idt[18], attr, NoError, cs); // Machine Check Exception (#MC)
    SetIDTEntry(idt[19], attr, NoError, cs); // SIMD Floating-Point Exception (#XM)
    SetIDTEntry(idt[20], attr, NoError, cs); // Virtualization Exception (#VE)
    SetIDTEntry(idt[21], attr, NoError, cs); // Control Protection Exception (#CP)
}
