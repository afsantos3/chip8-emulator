#include <stdint.h>

const unsigned int DISPLAY_WIDTH = 64;
const unsigned int DISPLAY_HEIGHT = 32;

class Chip8
{
public:
  Chip8();
  void Load(const char *f);
  void Run();

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
  void OP_9XYO();
  void OP_ANNN();
  void OP_DXYN();

  void OP_NULL();

  typedef void (Chip8::*OpcodeFunction)();
	OpcodeFunction op_table[16];
	OpcodeFunction op_table0[15];
	OpcodeFunction op_table8[15];
	OpcodeFunction op_tableE[15];
	OpcodeFunction op_tableF[66];
};