#ifndef cpu_h
#define cpu_h

#include <stdint.h>
#include "display.h"

typedef struct {
	uint8_t ram[4096];
	uint8_t reg[16];
	uint8_t dt;
	uint8_t st;
	uint8_t exec_state;
	uint16_t pc;
	uint16_t sp;
	uint16_t i;
	uint16_t keys;
	uint16_t stack[24];
	chip8_display_t *display;
} chip8_cpu_t;

chip8_cpu_t *init_cpu();
void dealloc_cpu(chip8_cpu_t *cpu);
void perform_instruction(chip8_cpu_t *cpu, uint16_t inst);

#endif