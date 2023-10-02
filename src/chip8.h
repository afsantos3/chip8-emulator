#include <stdint.h>

const unsigned int DISPLAY_WIDTH = 64;
const unsigned int DISPLAY_HEIGHT = 32;

class Chip8
{
public:
  Chip8();
  void Load(const char *f);
  void Run();

  uint8_t quirk;
  uint8_t keypad[16];
  uint8_t display[DISPLAY_WIDTH * DISPLAY_HEIGHT];

private:
  uint8_t memory[4096]; // RAM and Writeable
  uint8_t variable_registers[16];
  
  uint16_t stack[16];
  uint8_t stack_pointer;
  
  uint16_t opcode;
  uint16_t program_counter;
  uint16_t index_register;

  uint8_t delay_timer;
  uint8_t sound_timer;

  void Table0();
  void Table8();
  void TableE();
  void TableF();

  void OP_00E0();
  void OP_00EE();
  void OP_0NNN();
  void OP_1NNN();
  void OP_2NNN();
  void OP_3XNN();
  void OP_4XNN();
  void OP_5XY0();
  void OP_6XNN();
  void OP_7XNN();
  void OP_8XY0();
  void OP_8XY1();
  void OP_8XY2();
  void OP_8XY3();
  void OP_8XY4();
  void OP_8XY5();
  void OP_8XY6();
  void OP_8XY7();
  void OP_8XYE();
  void OP_9XYO();
  void OP_ANNN();
  void OP_BNNN();
  void OP_CXNN();
  void OP_DXYN();
  void OP_EX9E();
  void OP_EXA1();
  void OP_FX07();
  void OP_FX15();
  void OP_FX18();
  void OP_FX1E();
  void OP_FX33();
  void OP_FX55();
  void OP_FX65();

  void OP_NULL();

  typedef void (Chip8::*OpcodeFunction)();
	OpcodeFunction op_table[0x10u];
	OpcodeFunction op_table0[0xFu];
	OpcodeFunction op_table8[0xFu];
	OpcodeFunction op_tableE[0xFu];
	OpcodeFunction op_tableF[0x66u];

  uint8_t VX();
  uint8_t VY();
};