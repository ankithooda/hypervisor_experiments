#include <stdint.h>
//#include <fonts.h>
#include <spline_font.h>

/* A basic output terminal */

struct terminal {
  uint32_t *fb_ptr;
  uint64_t width;
  uint64_t height;
  uint64_t pitch;
  uint32_t *cursor;
  uint64_t cursor_row;
  uint64_t cursor_col;
};

struct terminal term;

void initialize_terminal(struct limine_framebuffer *fb) {
  term.fb_ptr     = fb->address;
  term.width      = fb->width;  // Number of pixels in a line
  term.height     = fb->height; // Number of lines
  term.pitch      = fb->pitch;  // pitch is 4 * width, because each pixel is 32-bits in framebuffer
  term.cursor     = fb->address;
  term.cursor_row = 0;
  term.cursor_col = 0;
}

/* Interesting tidbit - If we use term.pitch instead of term.width
   kind of scales the font and drastically reduces number of charcters
   one can put on the screen.
 */
void putc(uint8_t c) {

  if ( c == '\n' || ( term.cursor_col + SPLINE_FONT_WIDTH > term.width ) ) {
    term.cursor_row += SPLINE_FONT_VPAD;
    term.cursor_col = 0;
    term.cursor = term.fb_ptr + (SPLINE_FONT_HEIGHT * term.width * term.cursor_row);
    return;
  }

  for ( uint8_t i = 0; i < SPLINE_FONT_HEIGHT; i++ ) {

    uint16_t line = spline_font[ i + ( c * SPLINE_FONT_HEIGHT ) ];

    for ( uint8_t j = 0; j < SPLINE_FONT_WIDTH; j++ ) {
      if ( ( line >> ( SPLINE_FONT_WIDTH - j ) ) & 0x01 ) {
        term.cursor[ j + (term.width * i) ] = 0xffffff;
      } else {
        term.cursor[ j + (term.width * i) ] = 0x0;
      }
    }
  }
  // Increment cursor to next char position or the next line
  term.cursor += SPLINE_FONT_WIDTH + SPLINE_FONT_HPAD;

  // Increment cursor_row
  term.cursor_col += SPLINE_FONT_WIDTH + SPLINE_FONT_HPAD;
}

void write_string(uint8_t *s) {

  while ( *s != '\0' ) {
    putc( *s );
    s++;
  }
}
