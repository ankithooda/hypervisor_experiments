#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <fonts.h>

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

void putc(uint8_t c) {

  for ( uint8_t i = 0; i < ARMSCII_HEIGHT; i++ ) {

    uint8_t line = armscii8[ i + ( c * ARMSCII_HEIGHT ) ];

    for ( uint8_t j = 0; j < ARMSCII_WIDTH; j++ ) {
      if ( ( line >> ( ARMSCII_WIDTH - j ) ) & 0x01 ) {
        term.cursor[ j + (term.pitch * i) ] = 0xffffff;
      } else {
        term.cursor[ j + (term.pitch * i) ] = 0x0;
      }
    }
  }
  // Increment cursor to next char position or the next line
  term.cursor += (ARMSCII_WIDTH + ARMSCII_PAD);
}

void write_string(uint8_t *s) {
  for (uint8_t i = 0; i < 20; i++) {
    if (s[i] == '\0')
      break;
    putc(s[i]);
  }
}

// The following will be our kernel's entry point.
// If renaming _start() to something else, make sure to change the
// linker script accordingly.
void _start(void) {

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
  // Note: we assume the framebuffer model is RGB with 32-bit pixels.
  struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

  initialize_terminal(framebuffer);

  write_string((uint8_t *)" Hello World !!! ");

  uint64_t cr4 = getcr4();
  cr4 |= 1 << 13;
  setcr4(cr4);

  // We're done, just hang...
  hcf();
}
