# Chip8 Emulator

Yes, yet another Chip8 Emulator on the internet.(yes, I know it's actually an interpreter)

## Short description

CHIP-8 was created by engineer Joe Weisbecker in 1977 for the COSMAC VIP microcomputer. It was intended as a simpler way to make small programs and games for the computer. Instead of using machine language, you could type in simple hexadecimal instructions that resembled machine code, but which was actually interpreted on the fly by a small program.

Nowadays, if you want to write your custom ROM for a CHIP-8 interpreter you can simply do it by manually writing the hex opcodes and loading the ROM into the emulator, or you can use [Octo](http://johnearnest.github.io/Octo/) for a more user-friendly experience.

This emulator is far from perfect, and it's intended to be a very light-weight, straight-forward, easy to use and easy to learn from (I tried to keep the source code as simple as possible). It's going to run directly in your terminal so that's a nice thing too I guess.(if you see the full half of the glass, like me)

For any questions you might have feel free to [contact me](mailto:daviddvd267@gmail.com), or ask away on the [r/EmuDev](https://old.reddit.com/r/EmuDev/) forum.

## Usage

```bash Normal Running 
# first build the executable
make
# now run the emulator with any chip8 ROM you may want
./chip8_emulator <ROM_filepath>
# by default, the instruction set will be set to follow
# SUPER-CHIP rules, if you want original COSMAC VIP rules
./chip8_emulator <ROM_filepath> NORMAL COSMAC
# if you want to delete the executable
make clean 
```
```bash Debug Mode !TODO
# building and cleaning are done in the same way
# to run the emulator with a ROM in debug mode you do:
./chip8_emulator <ROM_filepath> DEBUG
# for COSMAC VIP running
./chip8_emulator <ROM_filepath> DEBUG COSMAC
```

## References

http://devernay.free.fr/hacks/chip8/C8TECH10.HTM

