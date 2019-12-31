#include "display.h"

void clear_display(chip8_display_t *display)
{
	for (int y = 0; y < 32; y++)
		display->frame_buffer[y] = 0;
}

bool toggle_pixel(chip8_display_t *display, uint8_t x, uint8_t y)
{
	x %= 64;
	y %= 32;

	uint64_t mask = (1ui64 << (64 - x));

	bool before = display->frame_buffer[y] & mask;
	display->frame_buffer[y] ^= mask;
	bool on = display->frame_buffer[y] & mask;

	static SDL_Rect pixel;
	pixel.x = (x - 2) * 20;
	pixel.y = y * 20;
	pixel.w = 20;
	pixel.h = 20;

	uint8_t color = on * 255;
	SDL_SetRenderDrawColor(display->renderer, color, color, color, 255);
	SDL_RenderFillRect(display->renderer, &pixel);

	return before == on;
}