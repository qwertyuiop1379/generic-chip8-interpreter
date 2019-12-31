#include <stdlib.h>
#include <time.h>
#include "cpu.h"

static uint8_t hex_digits[16][5] =
{
	{0xF0, 0x90, 0x90, 0x90, 0xF0},
	{0x20, 0x60, 0x20, 0x20, 0x70},
	{0xF0, 0x10, 0xF0, 0x80, 0xF0},
	{0xF0, 0x10, 0xF0, 0x10, 0xF0},
	{0x90, 0x90, 0xF0, 0x10, 0x10},
	{0xF0, 0x80, 0xF0, 0x10, 0xF0},
	{0xF0, 0x80, 0xF0, 0x90, 0xF0},
	{0xF0, 0x10, 0x20, 0x40, 0x40},
	{0xF0, 0x90, 0xF0, 0x90, 0xF0},
	{0xF0, 0x90, 0xF0, 0x10, 0xF0},
	{0xF0, 0x90, 0xF0, 0x90, 0x90},
	{0xE0, 0x90, 0xE0, 0x90, 0xE0},
	{0xF0, 0x80, 0x80, 0x80, 0xF0},
	{0xE0, 0x90, 0x90, 0x90, 0xE0},
	{0xF0, 0x80, 0xF0, 0x80, 0xF0},
	{0xF0, 0x80, 0xF0, 0x80, 0x80},
};

chip8_cpu_t *init_cpu()
{
	chip8_cpu_t *self = calloc(1, sizeof(chip8_cpu_t));
	self->display = calloc(1, sizeof(chip8_display_t));

	memcpy(self->ram, hex_digits, sizeof(hex_digits));

	return self;
}

void dealloc_cpu(chip8_cpu_t *cpu)
{
	free(cpu->display);
	free(cpu);
}

void perform_instruction(chip8_cpu_t *cpu, uint16_t inst)
{
	uint8_t n0 = inst >> 0xc;
	uint8_t n1 = (inst >> 0x8) & 0xf;
	uint8_t n2 = (inst >> 0x4) & 0xf;
	uint8_t n3 = inst & 0xf;
	
	uint16_t val0 = inst & 0xfff;
	uint8_t val1 = inst & 0xff;

	switch (n0)
	{
		case 0x0:
		{
			if (val0 == 0xe0)
				clear_display(cpu->display);
			else if (val0 == 0xee)
				cpu->pc = cpu->stack[--cpu->sp];

			// ignore sys
			break;
		}

		case 0x1:
		{
			cpu->pc = val0;
			break;
		}

		case 0x2:
		{
			cpu->stack[cpu->sp] = cpu->pc;
			cpu->sp++;

			cpu->pc = val0;
			break;
		}

		case 0x3:
		{
			if (cpu->reg[n1] == val1)
				cpu->pc += 2;

			break;
		}

		case 0x4:
		{
			if (cpu->reg[n1] != val1)
				cpu->pc += 2;

			break;
		}

		case 0x5:
		{
			if (n3)
				break;

			if (cpu->reg[n1] == cpu->reg[n2])
				cpu->pc += 2;

			break;
		}

		case 0x6:
		{
			cpu->reg[n1] = val1;
			break;
		}

		case 0x7:
		{
			cpu->reg[n1] += val1;
			break;
		}

		case 0x8:
		{
			switch (n3)
			{
				case 0x0:
				{
					cpu->reg[n1] = cpu->reg[n2];
					break;
				}

				case 0x1:
				{
					cpu->reg[n1] |= cpu->reg[n2];
					break;
				}

				case 0x2:
				{
					cpu->reg[n1] &= cpu->reg[n2];
					break;
				}

				case 0x3:
				{
					cpu->reg[n1] ^= cpu->reg[n2];
					break;
				}

				case 0x4:
				{
					uint16_t val = cpu->reg[n1] + cpu->reg[n2];
					cpu->reg[n1] = val;
					cpu->reg[15] = val < 255;
					break;
				}

				case 0x5:
				{
					cpu->reg[15] = cpu->reg[n1] > cpu->reg[n2];
					cpu->reg[n1] -= cpu->reg[n2];
					break;
				}

				case 0x6:
				{
					cpu->reg[15] = cpu->reg[n1] & 1;
					cpu->reg[n1] /= 2;
					break;
				}

				case 0x7:
				{
					int16_t val = cpu->reg[n2] - cpu->reg[n1];
					cpu->reg[n2] = val;
					cpu->reg[15] = val > 0;
					break;
				}

				case 0xe:
				{
					cpu->reg[15] = (cpu->reg[n1] & 128) >> 7;
					cpu->reg[n1] *= 2;
					break;
				}
			}

			break;
		}

		case 0x9:
		{
			if (n3)
				break;

			if (cpu->reg[n1] != cpu->reg[n2])
				cpu->pc += 2;

			break;
		}

		case 0xa:
		{
			cpu->i = val0;
			break;
		}

		case 0xb:
		{
			cpu->pc = cpu->reg[0] + val0;
			break;
		}

		case 0xc:
		{
			srand(time(NULL));
			cpu->reg[n1] = (rand() % 255) | val0;
			break;
		}

		case 0xd:
		{
			bool erase = 0;

			for (int y = 0; y < n3; y++)
			{
				uint8_t byte = cpu->ram[cpu->i + y];

				for (int x = 0; x < 8; x++)
				{
					if (byte & (1 << x))
					{
						if (toggle_pixel(cpu->display, 7 - x + cpu->reg[n1], y + cpu->reg[n2]))
							erase = 1;
					}
				}
			}

			cpu->reg[15] = erase;
			break;
		}

		case 0xe:
		{
			uint16_t keycode = pow(2, (1 << cpu->reg[n1]));

			if (val1 == 0x9e)
			{

				if (cpu->keys >> cpu->reg[n1] & 1)
					cpu->pc += 2;
			}
			else if (val1 == 0xa1)
			{
				if (!(cpu->keys >> cpu->reg[n1] & 1))
					cpu->pc += 2;
			}

			break;
		}

		case 0xf:
		{
			switch (val1)
			{
				case 0x07:
				{
					cpu->reg[n1] = cpu->dt;
					break;
				}

				case 0x0a:
				{
					cpu->exec_state = n1;
					break;
				}

				case 0x15:
				{
					cpu->dt = cpu->reg[n1];
					break;
				}

				case 0x18:
				{
					cpu->st = cpu->reg[n1];
					break;
				}

				case 0x1e:
				{
					cpu->i += cpu->reg[n1];
					break;
				}

				case 0x29:
				{
					cpu->i = cpu->reg[n1] * sizeof(uint8_t) * 5;
					break;
				}

				case 0x33:
				{
					int v = cpu->reg[n1];
					cpu->ram[cpu->i] = v / 100;
					v = v % 100;
					cpu->ram[cpu->i + 1] = v / 10;
					cpu->ram[cpu->i + 2] = v % 10;

					break;
				}

				case 0x55:
				{
					for (int v = 0; v <= n1; v++)
						cpu->ram[cpu->i + v] = cpu->reg[v];

					break;
				}

				case 0x65:
				{
					for (int v = 0; v <= n1; v++)
						cpu->reg[v] = cpu->ram[cpu->i + v];

					break;
				}
			}

			break;
		}
	}
}