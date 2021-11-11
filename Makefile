build: chip8_emulator

chip8_emulator:
	gcc ./src/c8.c -O3 -o chip8_emulator -std=c99

clean:
	rm chip8_emulator

check_mem: chip8_emulator
	valgrind --leak-check=full --show-leak-kinds=all --track-orgins=yes ./chip8_emulator

