/*
 * Author: David Bors <daviddvd267@gmail.com>
 * Usage : ./chip8_emulator help
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))
#define _CHIP_8_

/* memory and registers */

/* memory -> 4 kb of ram */
uint8_t memory[4096];
/* display 32 x 64 pixels monochrome */
int display[32][64];
/* program counter */
uint16_t PC;
/* 16-bit index register I */
uint16_t I;
/* stack for 16 16-bit addresses */
uint16_t stack[16];
/* 8 bit stack pointer */
uint8_t SP;
/* 8 bit delay timer */
uint8_t delay_timer;
/* 8 bit sound timer */
uint8_t sound_timer;
/* 16 8 bit general purpose registers V0 -> VF */
uint8_t V[17];
/* vf is also used as a flag register -> NOT TO BE USED BY ANY PROGRAM */
uint8_t *VF = (V + 0xF);

/* keyboard */
uint8_t key[16];

/* font */
unsigned char chip8_fontset[80] =
{
    0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};

/* current opcode processed */
uint16_t opcode;

/* init the emulator */
void __init_chip8_emulator (void)
{
	PC          = 0x200; /* start of most Chip-8 programs */
	I           = 0x0;
	SP          = 0x0;
	opcode      = 0x0;
	delay_timer = 0x0;
	sound_timer = 0x0;
	/* clear stack, mem, keyapd, registers, display */
	memset(stack, 0, sizeof(stack));
	memset(memory, 0, sizeof(memory));
	memset(key, 0, sizeof(key));
	memset(V, 0, sizeof(V));
	memset(display, 0, sizeof(display));
	/* load font into memory */	
	int i = 0;
	for(; i < 80; ++i)
		memory[i] = chip8_fontset[i];
}

/* load the rom file into the emulator */
int __load_chip8_emulator (const char *filepath)
{
	printf("Loading ROM: %s\n", filepath);

	FILE *rom = fopen(filepath, "rb");
	if (NULL == rom) {
		fprintf(stderr, "Failed to open ROM\n");
		return -1;
	}

	/* get the size of the rom file */
	fseek(rom, 0, SEEK_END);
	size_t size = ftell(rom);
	rewind(rom);
	if ((4096 - 512) < size) {
		fprintf(stderr, "ROM too big\n");
		goto error1;
	}
	
	/* alloc mem to store rom */
	char *rom_buffer = (char*) malloc(sizeof(char) * size + 1);
	if (NULL == rom_buffer) {
		fprintf(stderr, "Failed to alloc mem for ROM\n");
		goto error1;
	}

	/* copy the rom into a buffer */
	size_t result = fread(rom_buffer, sizeof(char), size, rom);
	if (result != size) {
		fprintf(stderr, "Failed to read ROM\n");
		goto error2;
	}

	/* finally, copy the rom buffer into the memory */
	/* TODO review */
	memcpy(memory+512, (uint8_t*)rom_buffer, size+1);

	/* clean up */
	fclose(rom);
	free(rom_buffer);
	printf("Loaded ROM.\n");
	return 0;

error2:
	free(rom_buffer);
error1:
	fclose(rom);	
	return -1;
}

 void __c8_draw (uint8_t vx, uint8_t vy, unsigned n) 
{
	printf("Drawing...\n");
	int x = vx % 64;
	int y = vy % 32;
	V[0xF] = 0;
	int aux = x;
	for (int i = 0; i < n; ++i) {
		// printf("%d, %d\n", y, x);
		uint8_t n_byte = memory[I + i];
		x = aux;
		for (int j = 7; j >= 0; --j) {
			if ( CHECK_BIT(n_byte, j) && display[y][x] ) {
				V[0xF] = 1;
			//	printf("%d, %d\n", y, x);
				display[y][x] = 0;
			} else if ( CHECK_BIT(n_byte, j) && !display[y][x] ) {
			//	printf("%d, %d\n", y, x);
				display[y][x] = 1;
			}/* else {
				printf("%d, %d, nothing found boss\n", y, x);
			}*/
			x++;
		}
		y++;
	}
}

void __c8_dump_display (void)
{
	system("clear");
	for(int i = 0; i < 32; i++) {
		for(int j = 0; j < 64; j++)
			printf(display[i][j] ? "X " : "  ");
		printf("\n");
	}
}

void __c8_cycle (void)
{
	/* FETCH => get opcode, which is 2 bytes */
	opcode = memory[PC] << 8 | memory[PC+1];
	PC += 2;

	/* DT and ST manage */	
	if (delay_timer > 0)
		--delay_timer;

	if (sound_timer > 0) {
		/* TODO implement sound */
		--sound_timer;
	}

	/* DECODE & EXECUTE*/
	unsigned nnn = opcode & 0xFFF; 
	unsigned n   = (opcode >> 12) & 0xF;
	unsigned x   = (opcode >> 8) & 0xF;
	unsigned y   = (opcode >> 4) & 0xF;
	unsigned kk  = opcode & 0xFF;

	//printf("0x%x\n", opcode & 0xF000);

	switch ((opcode & 0xF000) >> 12 & 0xF) {
	case 0x0:	
		if (0x00E0 == opcode) { /* clear the display */
			memset(display, 0, sizeof(display));
		} else if (0x00EE == opcode) { /* return from subroutine */
			PC = stack[SP--];
		} else {
			fprintf(stderr, "Unknown instruction: 0x%x", opcode);
			exit(1);
		}
		break;
	case 0x1:	/* JP addr */
		PC = nnn;
		break;
	case 0x2:	/* CALL addr */
		stack[SP++] = PC;
		PC = nnn;
		break;
	case 0x3:	/* SE Vx, byte */
		PC += (V[x] == kk) ? 2 : 0;
		break;
	case 0x4:	/* SNE Vx, byte */
		PC += (V[x] != kk) ? 2 : 0;
		break;
	case 0x5:	/* SE Vx, Vy */
		PC += (V[x] == V[y]) ? 2 : 0;
		break;
	case 0x6:	/* LD Vx, byte */
		V[x] = kk;
		break;
	case 0x7:	/* ADD Vx, byte */
		V[x] += kk;
		break;
	case 0x8:
		if ((opcode & 0x000F) == 0) {	/* LD Vx, Vy */
			V[x] = V[y];
			break;
		} else if ((opcode & 0x000F) == 0x1) {	/* OR Vx, Vy */
			V[x] |= V[y];
			break;
		} else if ((opcode & 0x000F) == 0x2) {	/* AND Vx, Vy */
			V[x] &= V[y];
			break;
		} else if ((opcode & 0x000F) == 0x3) {	/* XOR Vx, Vy */
			V[x] ^= V[y];
			break;
		} else if ((opcode & 0x000F) == 0x4) {	/* ADD Vx, Vy */
			int aux = (int)V[x] + (int)V[y];
			V[0xF] = (aux > 255);
			V[x] = aux;
			break;
		} else if ((opcode & 0x000F) == 0x5) {	/* SUB Vx, Vy */
			V[0xF] = (V[x] > V[y]);
			V[x] -= V[y];
			break;
		} else if ((opcode & 0x000F) == 0x6) {	/* SHR Vx {, Vy{ */
			//V[x] = V[y];
			//V[0xF] = CHECK_BIT(V[x], 7);
			//V[x] >>= 1;
			V[0xF] = CHECK_BIT(V[x], 7);
			V[x] >>= 1;
		} else if ((opcode & 0x000F) == 0x7) { 	/* SUBN Vx, Vy */
			V[0xF] = (V[x] < V[y]);
			V[x] = V[y] - V[x];
		} else if ((opcode & 0x000F) == 0xE) {	/* SHL Vx {, Vy} */
			V[0xF] = CHECK_BIT(V[x], 0);
			V[x] <<= 1;
		} else { 
			fprintf(stderr, "Unknown instruction: 0x%x\n", opcode);
			exit(1);
		}
		break;

	case 0x9:	/* SNE Vx, Vy */
		PC += (V[x] != V[y]) ? 2 : 0;
		break;

	case 0xA:	/* LD I, addr */
		I = nnn;
		break;
	case 0xB:	/* JP V0, addr */
		PC = nnn + V[0x0];
		break;
	case 0xC:	/* RND Vx, byte */
		/* TODO gen random */
		V[x] = 0x2 & kk;
	case 0xD:	/* DRW Vx, Vy, nibble */
		__c8_draw(V[x], V[y], n);
		__c8_dump_display();
		break;
	case 0xE:
		if ((opcode & 0x00FF) == 0x93) {	/* SKP Vx */
			if (V[x] < 17)
				PC += (key[V[x]]) ? 2 : 0;
		} else if ((opcode & 0x00FF) == 0xA1) {	/* SKNP Vx */
			if (V[x] < 17)
				PC += (key[V[x]]) ? 0 : 2;
		} else {
			fprintf(stderr, "Unknown instruction: 0x%x\n", opcode);
			exit(1);
		}
		break;
	case 0xF:
		if ((opcode & 0x00FF) == 0x07) {	/* LD Vx, DT */
			V[x] = delay_timer;
			break;
		} else if ((opcode & 0x00FF) == 0x0A) {  /* LD Vx, K */
			fprintf(stderr, "Not implemented yet!\n");
			break;
		} else if ((opcode & 0x00FF) == 0x15) {  /* LD DT, Vx */
			delay_timer = V[x];
			break;
		} else if ((opcode & 0x00FF) == 0x18) {  /* LD ST, Vx */
			sound_timer = V[x];
			break;
		} else if ((opcode & 0x00FF) == 0x1E) {  /* ADD I, Vx */
			I += V[x];
			break;
		} else if ((opcode & 0x00FF) == 0x29) {  /* LD F, Vx */
			I = (V[x] & 0x0F) * 5;
			break;
		} else if ((opcode & 0x00FF) == 0x33) {  /* LD B, Vx */
			memory[I]   = V[x] / 100;
			memory[I+1] = V[x] / 10 % 10;
			memory[I+2] = V[x] % 20;
			break;
		} else if ((opcode & 0x00FF) == 0x55) {  /* LD [I], Vx */
			int i = 0;
			int idx = I;
			for(; i <= 0xF; i++)
				memory[idx++] = V[i]; 	
			break;
		} else if ((opcode & 0x00FF) == 0x65) {  /* LD Vx, [I] */
			int i = 0;
			int idx = I;
			for(; i <= 0xF; i++)
				V[i] = memory[idx++];
			break;
		} else {
			fprintf(stderr, "Unknown instruction: 0x%x\n", opcode);
			exit(1);
		}
		break;
	default:
		fprintf(stderr, "Unknown instruction: 0x%x\n", opcode);
		exit(1);
		break;
	}	
}

void chip8_emulator (char *rom_fp)
{
	__init_chip8_emulator();
	__load_chip8_emulator(rom_fp);

	while (1) {
		__c8_cycle();
	}
}

void __usage ()
{
	printf("Chip8 emulator usage: ./chip8_emulator <ROM_FILEPATH>\n");
}

int main (int argc, char *argv[])
{
	if (argc == 2 && strcmp(argv[1], "help"))
		chip8_emulator(argv[1]);
	else    __usage();
	return 0;
}






