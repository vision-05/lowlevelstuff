#include "idt.h"

struct idt_entry idt[256];
struct idt_ptr idtp;

extern void idt_load(uint64_t ptr);
extern void error_code_isr();


void idt_set_gate(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags) {
	idt[num].offset_low = (base & 0xFFFF);
	idt[num].offset_mid = (base >> 16) & 0xFFFF;
	idt[num].offset_high = (base >> 32) & 0xFFFFFFFF;

	idt[num].selector = sel;
	idt[num].ist = 0;
	idt[num].attributes = flags;
	idt[num].reserved = 0;
}

void init_idt() {
	idtp.limit = (sizeof(struct idt_entry) * 256)-1;
	idtp.base = (uintptr_t)&idt;

	for(int i = 0; i < 256; ++i) {
		idt_set_gate(i, 0, 0, 0);
	}

	idt_set_gate(13, (uint64_t)error_code_isr, 0x08, 0x8E);
	idt_set_gate(8, (uint64_t)error_code_isr, 0x08, 0x8E);

	idt_load((uintptr_t)&idtp);
}

void exception_handler(uint64_t stack_pointer) {
	while(1) {
		asm("hlt");
	}
}
