#include <stdio.h>
#include <stdint.h>
#include <sys/timeb.h>
#include <SDL.h>
#include "cpu.h"
#include "display.h"

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		printf("Usage: %s <rom>\n", argv[0]);
		return -1;
	}

	FILE *rom = fopen(argv[1], "rb");
	if (!rom)
	{
		printf("Failed to open rom.\n");
		return 1;
	}

	char *title = malloc(sizeof(char) * (strlen(argv[1]) + 8));
	sprintf(title, "Chip8: %s", argv[1]);

	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window *window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1240, 640, 0);

	free(title);

	if (!window)
	{
		printf("Failed to create window.\n");
		return 1;
	}

	chip8_cpu_t *cpu = init_cpu();
	cpu->display->renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	SDL_RenderPresent(cpu->display->renderer);

	fseek(rom, 0L, SEEK_END);
	int rom_end = ftell(rom);
	fseek(rom, 0L, SEEK_SET);

	int rom_offset = 0x200;
	cpu->pc = rom_offset;

	fread(cpu->ram + rom_offset, 1, rom_end, rom);

	bool run = 1;
	while (run)
	{
		static struct timeb before, after;
		ftime(&before);

		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
				run = 0;

			if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP)
			{
				static int keys[] = { SDLK_x, SDLK_1, SDLK_2, SDLK_3, SDLK_q, SDLK_w, SDLK_e, SDLK_a, SDLK_s, SDLK_d, SDLK_z, SDLK_c, SDLK_4, SDLK_r, SDLK_f, SDLK_v };
				bool status = e.type == SDL_KEYDOWN;
				for (int i = 0; i < 0xF; i++)
				{
					if (e.key.keysym.sym == keys[i])
						cpu->keys ^= (-status ^ cpu->keys) & (1UL << i);
				}
			}
		}

		for (int i = 0; i < 9; i++)
		{
			if (!cpu->exec_state)
			{
				if (cpu->dt)
				{
					cpu->dt--;
					continue;
				}

				if (cpu->pc - rom_offset == rom_end)
					break;

				uint16_t instruction = cpu->ram[cpu->pc] << 8 | cpu->ram[cpu->pc + 1];

				cpu->pc += 2;
				perform_instruction(cpu, instruction);
			}
			else
			{
				if (cpu->keys)
					cpu->exec_state = 0;
			}
		}

		SDL_RenderPresent(cpu->display->renderer);

		ftime(&after);
		int16_t ms = (after.millitm) - (before.millitm);

		SDL_Delay((1000 / 60) - (ms > 0 ? ms : 0)); // it doesn't matter if we lose 1 millisecond but why not
	}

	dealloc_cpu(cpu);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}