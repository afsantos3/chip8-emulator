#include <fstream>

#include "chip8.h"

const unsigned int STARTING_ADDRESS = 0x200;
const unsigned int FONTSET_STARTING_ADDRESS = 0x050;

uint8_t fontset[80] = {
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

Chip8::Chip8()
{
  program_counter = STARTING_ADDRESS;
  stack_pointer = 0;

  // Clear the memory
  memset(memory, 0, sizeof(memory));

  // Load the font into memory
  for(int i = 0; i < sizeof(fontset); ++i)
  {
    memory[FONTSET_STARTING_ADDRESS + i] = fontset[i];
  }

  // Function table
  op_table[0x0u] = &Chip8::Table0;
  op_table[0x1u] = &Chip8::OP_1NNN;
  op_table[0x2u] = &Chip8::OP_2NNN;
  op_table[0x3u] = &Chip8::OP_3XNN;
  op_table[0x4u] = &Chip8::OP_4XNN;
  op_table[0x5u] = &Chip8::OP_5XY0;
  op_table[0x6u] = &Chip8::OP_6XNN;
  op_table[0x7u] = &Chip8::OP_7XNN;
  op_table[0x8u] = &Chip8::Table8;
  op_table[0x9u] = &Chip8::OP_9XYO;
  op_table[0xAu] = &Chip8::OP_ANNN;
  op_table[0xDu] = &Chip8::OP_DXYN;

  std::fill_n(op_table0, sizeof(op_table0) / sizeof(op_table0[0]), &Chip8::OP_NULL);
  op_table0[0x0u] = &Chip8::OP_00E0;

  std::fill_n(op_table8, sizeof(op_table8) / sizeof(op_table8[0]), &Chip8::OP_NULL);
  op_table8[0x0u] = &Chip8::OP_8XY0;
  op_table8[0x1u] = &Chip8::OP_8XY1;
  op_table8[0x2u] = &Chip8::OP_8XY2;
  op_table8[0x3u] = &Chip8::OP_8XY3;
  op_table8[0x4u] = &Chip8::OP_8XY4;
}

void Chip8::Load(const char *f)
{
  // Open file with position at the end
  std::ifstream file(f, std::ios::binary | std::ios::ate);

  if(file.is_open())
  {
    // Get the size of the file, and create a buffer with that size
    std::streampos size = file.tellg();
    char* buffer = new char[size];
    
    // Return to the beginning of the file, and read contents into buffer
    file.seekg(0, std::ios::beg);
    file.read(buffer, size);
    file.close();

    // Write content of buffer into memory with the specified starting address
    for(int i = 0; i < size; i++)
    {
      memory[STARTING_ADDRESS + i] = buffer[i];
    }

    delete[] buffer;
  }
}

void Chip8::Run()
{
  // Fetch, one instruction is two bytes
  opcode =  (memory[program_counter] << 8) | memory[program_counter + 1];

  // Increment the program counter
  program_counter += 2;

  // Decode and Execute
  uint8_t highNibble = (opcode & 0xF000) >> 12;
  (this->*op_table[highNibble])();

  // Decrement the timers
  if(delay_timer > 0)
  {
    delay_timer--;
  }

  // TODO Put some Sound (System Commands)
  if(sound_timer > 0)
  {
    sound_timer--;
  }
}

void Chip8::Table0()
{
  (this->*op_table0[opcode & 0x000Fu])();
}

void Chip8::Table8()
{
  (this->*op_table8[opcode & 0x000Fu])();
}

void Chip8::TableE()
{
  (this->*op_tableE[opcode & 0x000Fu])();
}

void Chip8::TableF()
{
  (this->*op_tableF[opcode & 0x000Fu])();
}

// Clear Display
void Chip8::OP_00E0()
{
  memset(display, 0, sizeof(display));
}

void Chip8::OP_00EE()
{
  --stack_pointer;
  program_counter = stack[stack_pointer];
}

// Jump
void Chip8::OP_1NNN()
{
  uint16_t address = opcode & 0x0FFFu;
  program_counter = address;
}

void Chip8::OP_2NNN()
{
  uint16_t address = opcode & 0x0FFFu;
  stack[stack_pointer] = program_counter;
  ++stack_pointer;

  program_counter = address;
}

void Chip8::OP_3XNN()
{
  uint8_t i = (opcode & 0x0F00u) >> 8u;
  uint8_t byte = opcode & 0x00FFu;

  if(variable_registers[i] == byte)
  {
    program_counter += 2;
  }
}

void Chip8::OP_4XNN()
{
  uint8_t i = (opcode & 0x0F00u) >> 8u;
  uint8_t byte = opcode & 0x00FFu;

  if(variable_registers[i] != byte)
  {
    program_counter += 2;
  }
}

void Chip8::OP_5XY0()
{
  uint8_t i = (opcode & 0x0F00u) >> 8u;
  uint8_t j = (opcode & 0x00F0u) >> 4u;

  if(variable_registers[i] == variable_registers[j])
  {
    program_counter += 2;
  }
}

void Chip8::OP_6XNN()
{
  uint8_t i = (opcode & 0x0F00u) >> 8u;
  uint8_t byte = opcode & 0x00FFu;

  variable_registers[i] = byte;
}

void Chip8::OP_7XNN()
{
  uint8_t i = (opcode & 0x0F00u) >> 8u;
  uint8_t byte = opcode & 0x00FFu;

  variable_registers[i] += byte;
}

void Chip8::OP_8XY0()
{
  uint8_t i = (opcode & 0x0F00u) >> 8u;
  uint8_t j = (opcode & 0x00F0u) >> 4u;

  variable_registers[i] = variable_registers[j];
}

void Chip8::OP_8XY1()
{
  uint8_t i = (opcode & 0x0F00u) >> 8u;
  uint8_t j = (opcode & 0x00F0u) >> 4u;

  variable_registers[i] |= variable_registers[j];
}

void Chip8::OP_8XY2()
{
  uint8_t i = (opcode & 0x0F00u) >> 8u;
  uint8_t j = (opcode & 0x00F0u) >> 4u;

  variable_registers[i] &= variable_registers[j];
}

void Chip8::OP_8XY3()
{
  uint8_t i = (opcode & 0x0F00u) >> 8u;
  uint8_t j = (opcode & 0x00F0u) >> 4u;

  variable_registers[i] ^= variable_registers[j];
}

void Chip8::OP_8XY4()
{
  uint8_t i = (opcode & 0x0F00u) >> 8u;
  uint8_t j = (opcode & 0x00F0u) >> 4u;

  //Eval the variable register carry flag if overflow
  variable_registers[0x0Fu] = ((variable_registers[i] + variable_registers[j]) > 0xFF);

  //If overflow, will wrap (maybe..)
  uint16_t sum = variable_registers[i] + variable_registers[j];
  //variable_registers[i] += variable_registers[j];
  variable_registers[i] = sum & 0xFFu;
}

void Chip8::OP_8XY5()
{
  
}

void Chip8::OP_9XYO()
{
  uint8_t i = (opcode & 0x0F00u) >> 8u;
  uint8_t j = (opcode & 0x00F0u) >> 4u;

  if(variable_registers[i] != variable_registers[j])
  {
    program_counter += 2;
  }
}

void Chip8::OP_ANNN()
{
  index_register = opcode & 0x0FFFu;
}

void Chip8::OP_DXYN()
{
  uint8_t x = (opcode & 0x0F00u) >> 8u;
  uint8_t y = (opcode & 0x00F0u) >> 4u;
  uint8_t height = opcode & 0x000Fu;

  uint8_t xCoord = variable_registers[x] % DISPLAY_WIDTH;
  uint8_t yCoord = variable_registers[y] % DISPLAY_HEIGHT;
  variable_registers[0xFu] = 0;

  // TODO Adjust Implementation for more clarity
  for(int i = 0; i < height; ++i)
  {
    uint8_t spriteByte = memory[index_register + i];
    for(int j = 0; j < 8; ++j)
    {
      uint8_t spritePixel = spriteByte & (0x80u >> j);
			uint8_t* screenPixel = &display[(yCoord + i) * DISPLAY_WIDTH + (xCoord + j)];

			if (spritePixel)
			{
				if (*screenPixel == 0xFF)
				{
					variable_registers[0xFu] = 1;
				}

				*screenPixel ^= 0xFFFFFFFF;
      }
    }
  }
}

void Chip8::OP_NULL()
{
}
