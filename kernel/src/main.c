#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

// Set the base revision to 2, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.

__attribute__((used, section(".requests")))
static volatile LIMINE_BASE_REVISION(2);

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.

__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
  .id = LIMINE_FRAMEBUFFER_REQUEST,
  .revision = 0
};

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.

__attribute__((used, section(".requests_start_marker")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests_end_marker")))
static volatile LIMINE_REQUESTS_END_MARKER;

// GCC and Clang reserve the right to generate calls to the following
// 4 functions even if they are not directly called.
// Implement them as the C specification mandates.
// DO NOT remove or rename these functions, or stuff will eventually break!
// They CAN be moved to a different .c file.

void *memcpy(void *dest, const void *src, size_t n) {
  uint8_t *pdest = (uint8_t *)dest;
  const uint8_t *psrc = (const uint8_t *)src;

  for (size_t i = 0; i < n; i++) {
    pdest[i] = psrc[i];
  }

  return dest;
}

void *memset(void *s, int c, size_t n) {
  uint8_t *p = (uint8_t *)s;

  for (size_t i = 0; i < n; i++) {
    p[i] = (uint8_t)c;
  }

  return s;
}

void *memmove(void *dest, const void *src, size_t n) {
  uint8_t *pdest = (uint8_t *)dest;
  const uint8_t *psrc = (const uint8_t *)src;

  if (src > dest) {
    for (size_t i = 0; i < n; i++) {
      pdest[i] = psrc[i];
    }
  } else if (src < dest) {
    for (size_t i = n; i > 0; i--) {
      pdest[i-1] = psrc[i-1];
    }
  }

  return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const uint8_t *p1 = (const uint8_t *)s1;
  const uint8_t *p2 = (const uint8_t *)s2;

  for (size_t i = 0; i < n; i++) {
    if (p1[i] != p2[i]) {
      return p1[i] < p2[i] ? -1 : 1;
    }
  }

  return 0;
}

// Halt and catch fire function.
static void hcf(void) {
  asm ("cli");
  for (;;) {
    asm ("hlt");
  }
}


// VT-x //

static inline uint64_t getcr4(void) {
  register uint64_t ret = 0;

  asm volatile (
                "movq %%cr4, %0\n"
                :"=r"(ret)
                );

  return ret;
}

static inline void setcr4(register uint64_t val) {
  asm volatile (
                "movq %0, %%cr4\n"
                :
                :"r"(val)
                );
}

/* A basic output terminal */

struct terminal {
  uint32_t *fb_ptr;
  uint64_t width;
  uint64_t height;
  uint64_t pitch;
  uint32_t *cursor;
};

struct terminal term;

void initialize_terminal(struct limine_framebuffer *fb) {
  term.fb_ptr = fb->address;
  term.width  = fb->width;
  term.height = fb->height;
  term.pitch  = fb->pitch;
  term.cursor = fb->address;
}

void putc(uint8_t *font_char) {

  for ( uint8_t i = 0; i < 16; i++ ) {
    uint8_t line = font_char[i];
    for ( uint8_t j = 0; j < 8; j++ ) {
      if ((line >> (8-j)) & 0x01) {
        term.cursor[j + (term.pitch * i)] = 0xffffff;
      } else {
        term.cursor[j + (term.pitch * i)] = 0x0;
      }
    }
  }
  // Increment cursor to next char position or the next line
  term.cursor += 8;
}

// The following will be our kernel's entry point.
// If renaming _start() to something else, make sure to change the
// linker script accordingly.
void _start(void) {

  uint8_t exclam[16] = {
    0x00, 0x18,
    0x3c, 0x3c,
    0x3c, 0x3c,
    0x18, 0x18,
    0x18, 0x0,
    0x18, 0x18,
    0x0,  0x0,
    0x0,  0x0
  };

  uint8_t ascii_c[16] = {
    //007c c6c6 c0c0 c0c0 c0c6 c67c 0000 0000
    0x00, 0x7c,
    0xc6, 0xc6,
    0xc0, 0xc0,
    0xc0, 0xc0,
    0xc0, 0xc6,
    0xc6, 0x7c,
    0x0,  0x0,
    0x0,  0x0
  };
  // Ensure the bootloader actually understands our base revision (see spec).
  if (LIMINE_BASE_REVISION_SUPPORTED == false) {
    hcf();
  }

  // Ensure we got a framebuffer.
  if (framebuffer_request.response == NULL
      || framebuffer_request.response->framebuffer_count < 1) {
    hcf();
  }

  // Fetch the first framebuffer.
  struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

  initialize_terminal(framebuffer);
  putc(ascii_c);
  putc(exclam);

  // Note: we assume the framebuffer model is RGB with 32-bit pixels.
  /* for (size_t i = 0; i < 100; i++) { */
  /*   volatile uint32_t *fb_ptr = framebuffer->address; */
  /*   fb_ptr[i * (framebuffer->pitch / 4) + i] = 0xffffff; */
  /* } */

  /* /\* Initialize terminal interface *\/ */
  /* terminal_initialize(); */

  /* /\* Newline support is left as an exercise. *\/ */
  /* terminal_writestring("Hello, kernel World!\n"); */

  // We're done, just hang...

  uint64_t cr4 = getcr4();

  cr4 |= 1 << 13;

  setcr4(cr4);

  hcf();
}
