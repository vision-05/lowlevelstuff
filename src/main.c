#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include "font.h"

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

void kprintf(char* str, psf2_header_t *font, struct limine_framebuffer *framebuffer, uint8_t *x, uint8_t *y) {
	char* it = str;
	while(*it) {
		if(*it == '\n') {
			*x = 0;
			*y += font->height;
		}
		if(*it == '\r') {
			continue;
		}
		draw_char(font, *it, *x, *y, 0x0000ff, 0x00ff00, framebuffer);
		(*x) += font->width;
		if (*x > 100) {
			*x = 0;
			(*y) += font->height;
		}
		it++;
	}
}

void kmain(void) {
	uint8_t x = 0;
	uint8_t y = 0;
	if(LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
		hcf();
	}

	if(framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
		hcf();
	}

	struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

	psf2_header_t *font = init_font();

	for(int i = 0; i < 10; ++i) {

		kprintf("Hello world!\n", font, framebuffer, &x, &y);
	}

	hcf();
}
