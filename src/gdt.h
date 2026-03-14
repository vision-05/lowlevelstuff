#ifndef GDT_H
#define GDT_H

#include <stdint.h>

void init_gdt();
extern void gdt_reload();

#endif
