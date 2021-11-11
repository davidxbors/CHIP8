/*
 * Author: David Bors <daviddvd267@gmail.com>
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

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
/* instruction set currently used (SUPER / COSMAC, by default SUPER) */
enum instruction_set {SUPER_CHIP, COSMAC_VIP} is;
/* running mode: NORMAL/DEBUG */
enum run_mode {NORMAL, DEBUG} mode;

/* initial machine prepare functions */
void __init_chip8_emulator (void);
int __load_chip8_emulator (const char *);

/* drawing function */
void __c8_draw (uint8_t, uint8_t, unsigned);
/* basic display functionality */
void __c8_dump_display (void);

/* debugging functions */
void __c8_dump_op (uint16_t, unsigned, unsigned, unsigned, unsigned, unsigned);
void __c8_dump_registers ();
void __c8_dump_memory ();
void __c8_dump_special_registers ();
void __c8_dump_stack ();

/* glues together all the other function to emulate the machine */
/* it also handles debugging ops */
void chip8_emulator (char *rom_fp);

/* emulates one cycle of the cpu */
void __c8_cycle (void);

