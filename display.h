#ifndef display_h
#define display_h

#include <stdint.h>
#include <stdbool.h>
#include <SDL.h>

typedef struct {
	uint64_t frame_buffer[32];
	SDL_Renderer *renderer;
} chip8_display_t;

void clear_display(chip8_display_t *display);
bool toggle_pixel(chip8_display_t* display, uint8_t x, uint8_t y);

#endif