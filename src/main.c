#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include "font.h"
#include "gdt.h"
#include "idt.h"

__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(5);

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
	.id = LIMINE_FRAMEBUFFER_REQUEST_ID,
	.revision = 0
};

__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_reqeuests_end")))
static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

void *memcpy(void *restrict dest, const void *restrict src, size_t n) {
	uint8_t *restrict pdest = (uint8_t *restrict)dest;
	const uint8_t *restrict psrc = (const uint8_t *restrict)src;

	for(size_t i = 0; i < n; ++i) {
		pdest[i] = psrc[i];
	}

	return dest;
}

extern uint8_t _binary_fonts_lat9w_16_psfu_start[];
extern uint8_t _binary_fonts_lat9w_16_psfu_end[];

void *memset(void *s, int c, size_t n) {
	uint8_t *p = (uint8_t *)s;

	for(size_t i = 0; i < n; ++i) {
		p[i] = (uint8_t)c;
	}

	return s;
}

void *memmove(void *dest, const void* src, size_t n) {
	uint8_t *pdest = (uint8_t*)dest;
	const uint8_t *psrc = (const uint8_t*)src;
	if((uintptr_t)src > (uintptr_t)dest) {
		for(size_t i = 0; i < n; ++i) {
			pdest[i] = psrc[i];
		}
	} else if((uintptr_t)src < (uintptr_t)dest) {
		for(size_t i = n; i > 0; --i) {
			pdest[i-1] = psrc[i-1];
		}
	}

	return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
	const uint8_t *ps1 = (const uint8_t*)s1;
	const uint8_t *ps2 = (const uint8_t*)s2;

	for(size_t i = 0; i < n; ++i) {
		if(ps1[i] != ps2[i]) {
			return ps1[i] < ps2[i] ? -1 : 1;
		}
	}

	return 0;
}

static void hcf(void) {
	for (;;) {
		asm("hlt");
	}
}

psf2_header_t *init_font() {
	psf2_header_t *font_header = (psf2_header_t*)_binary_fonts_lat9w_16_psfu_start;
	if(font_header->magic != PSF2_MAGIC) {
		hcf();
	}

	return font_header;
}

void draw_char(psf2_header_t *font, char c, int x, int y, uint32_t fg, uint32_t bg, struct limine_framebuffer *framebuffer) {
	uint8_t* glyph = (uint8_t*)font + font->header_size + (c * font->glyph_size);
	int bytes_per_line = (font->width + 7)/8;


	for(uint32_t cy = 0; cy < font->height; ++cy) {
		for(uint32_t cx = 0; cx < font->width; ++cx) {
			uint8_t mask = 1 << (font->width - 1 - cx); //get the bit at the current x position
			uint32_t color = (mask & glyph[cy * bytes_per_line]) ? fg : bg;
			volatile uint32_t *pixel = (uint32_t*)((uint8_t*)framebuffer->address + (y+cy)*framebuffer->pitch + (x+cx)*4);
			*pixel = color;
		}
	}
}

void print_cursor(psf2_header_t *font, struct limine_framebuffer *framebuffer, int *x, int *y) {
	for(uint32_t cy = font->height - 4; cy < font->height; ++cy) {
		for(uint32_t cx = 0; cx < font->width; ++cx) {
			volatile uint32_t *pixel = (uint32_t*)((uint8_t*)framebuffer->address + (*y+cy)*framebuffer->pitch + (*x+cx)*4);
			*pixel = 0xffffff;
		}
	}
}

void kprintf(char* str, psf2_header_t *font, struct limine_framebuffer *framebuffer, int *x, int *y) {
	char* it = str;
	while(*it) {
		if(*it == '\n') {
			*it = ' ';
			draw_char(font, *it, *x, *y, 0xffffff, 0x000000, framebuffer); 
			*x = 0;
			*y += font->height;
			it++;
			continue;
		}
		if(*it == '\r') {
			it++;
			continue;
		}
		if(*it == '\b') {
			*it = ' ';
			draw_char(font, *it, *x, *y, 0xffffff, 0x000000, framebuffer);
			if(*x == 0) {
				*x = 1000;
				*y -= font->height;
			}
			else {	
				*x -= font->width;
			}
			draw_char(font, *it, *x, *y, 0xffffff, 0x000000, framebuffer);
			print_cursor(font, framebuffer, x, y);
			it++;
			continue;
		}
		if(*it == 0) {
			
			break;
		}
		draw_char(font, *it, *x, *y, 0xffffff, 0x000000, framebuffer);
		(*x) += font->width;
		if (*x > 1000) {
			*x = 0;
			(*y) += font->height;
			print_cursor(font, framebuffer, x, y);
		}
		it++;
	}
}

static inline uint8_t inb(uint16_t port) {
	uint8_t ret;
	asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

unsigned char kbd_us[128] = {
	0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
	'\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
	0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
	'\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ',
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, '-', 0, 0, 0, '+', 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0
};

void kmain(void) {
	init_gdt();
	gdt_reload();
	init_idt();
	asm volatile("int $0x0D");
	int x = 0;
	int y = 0;
	if(LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
		hcf();
	}

	if(framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
		hcf();
	}

	struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

	psf2_header_t *font = init_font();

	while(1) {
		if (inb(0x64) & 1) {
			uint8_t scancode = inb(0x60);
			if(!(scancode & 0x80)) {
				char c = kbd_us[scancode];
				if (c>0) {
					char str[2] = {c, '\0'};

					kprintf(str, font, framebuffer, &x, &y);
					print_cursor(font, framebuffer, &x, &y);
				}
			}
		}
	}

	hcf();
}
