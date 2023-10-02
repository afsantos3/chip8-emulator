#include <fstream>
#include <iostream>

#include "chip8.h"

#define DEFAULT 0
#define COSMAC_VIP 1 // TODO Review more arithmetic instructions
#define CHIP_48 2
#define SUPER_CHIP 3

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
  for(uint16_t i = 0; i < sizeof(fontset); ++i)
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
  op_table[0xBu] = &Chip8::OP_BNNN;
  op_table[0xCu] = &Chip8::OP_CXNN;
  op_table[0xDu] = &Chip8::OP_DXYN;
  op_table[0xEu] = &Chip8::TableE;
  op_table[0xFu] = &Chip8::TableF;

  std::fill_n(op_table0, sizeof(op_table0) / sizeof(op_table0[0]), &Chip8::OP_NULL);
  op_table0[0x0u] = &Chip8::OP_00E0;
  op_table0[0xEu] = &Chip8::OP_00EE;

  std::fill_n(op_table8, sizeof(op_table8) / sizeof(op_table8[0]), &Chip8::OP_NULL);
  op_table8[0x0u] = &Chip8::OP_8XY0;
  op_table8[0x1u] = &Chip8::OP_8XY1;
  op_table8[0x2u] = &Chip8::OP_8XY2;
  op_table8[0x3u] = &Chip8::OP_8XY3;
  op_table8[0x4u] = &Chip8::OP_8XY4;
  op_table8[0x5u] = &Chip8::OP_8XY5;
  op_table8[0x6u] = &Chip8::OP_8XY6;
  op_table8[0x7u] = &Chip8::OP_8XY7;
  op_table8[0xEu] = &Chip8::OP_8XYE;

  std::fill_n(op_tableE, sizeof(op_tableE) / sizeof(op_tableE[0]), &Chip8::OP_NULL);
  op_tableE[0x1u] = &Chip8::OP_EXA1;
  op_tableE[0xEu] = &Chip8::OP_EX9E;

  std::fill_n(op_tableF, sizeof(op_tableF) / sizeof(op_tableF[0]), &Chip8::OP_NULL);
  op_tableF[0x07u] = &Chip8::OP_FX07;
  op_tableF[0x15u] = &Chip8::OP_FX15;
  op_tableF[0x18u] = &Chip8::OP_FX18;
  op_tableF[0x1Eu] = &Chip8::OP_FX1E;
  op_tableF[0x33u] = &Chip8::OP_FX33;
  op_tableF[0x55u] = &Chip8::OP_FX55;
  op_tableF[0x65u] = &Chip8::OP_FX65;
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
    for(uint16_t i = 0; i < size; ++i)
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
  std::cout << "table F" << std::endl;
  (this->*op_tableF[opcode & 0x00FFu])();
}

// Clear Display
void Chip8::OP_00E0()
{
  memset(display, 0, sizeof(display));
}

// Return Subroutine
void Chip8::OP_00EE()
{
  --stack_pointer;
  program_counter = stack[stack_pointer];
}

// Execute Machine Language Routine
void Chip8::OP_0NNN()
{
  //TODO Compatability
}

// Jump
void Chip8::OP_1NNN()
{
  uint16_t address = opcode & 0x0FFFu;
  program_counter = address;
}

// Call Subroutine
void Chip8::OP_2NNN()
{
  uint16_t address = opcode & 0x0FFFu;
  stack[stack_pointer] = program_counter;
  ++stack_pointer;

  program_counter = address;
}

// Skip if VX == NN
void Chip8::OP_3XNN()
{
  uint8_t vx = VX();
  uint8_t byte = opcode & 0x00FFu;

  if(variable_registers[vx] == byte)
  {
    program_counter += 2;
  }
}

//Skip if VX != NN
void Chip8::OP_4XNN()
{
  uint8_t vx = VX();
  uint8_t byte = opcode & 0x00FFu;

  if(variable_registers[vx] != byte)
  {
    program_counter += 2;
  }
}

// Skip if VX == VY
void Chip8::OP_5XY0()
{
  uint8_t vx = VX();
  uint8_t vy = VY();

  if(variable_registers[vx] == variable_registers[vy])
  {
    program_counter += 2;
  }
}

// Set VX = NN
void Chip8::OP_6XNN()
{
  uint8_t vx = VX();
  uint8_t byte = opcode & 0x00FFu;

  variable_registers[vx] = byte;
}

// Set VY += NN
void Chip8::OP_7XNN()
{
  uint8_t vx = VX();
  uint8_t byte = opcode & 0x00FFu;

  variable_registers[vx] += byte;
}

// Set VX = VY
void Chip8::OP_8XY0()
{
  uint8_t vx = VX();
  uint8_t vy = VY();

  variable_registers[vx] = variable_registers[vy];
}

// Set VX |= VY
void Chip8::OP_8XY1()
{
  uint8_t vx = VX();
  uint8_t vy = VY();

  variable_registers[vx] |= variable_registers[vy];
}

// Set VX &= VY
void Chip8::OP_8XY2()
{
  uint8_t vx = VX();
  uint8_t vy = VY();

  variable_registers[vx] &= variable_registers[vy];
}

// Set VX ^= VY
void Chip8::OP_8XY3()
{
  uint8_t vx = VX();
  uint8_t vy = VY();

  variable_registers[vx] ^= variable_registers[vy];
}

// Set VX += VY with Carry VF
void Chip8::OP_8XY4()
{
  uint8_t vx = VX();
  uint8_t vy = VY();

  // Eval the variable register carry flag if overflow
  variable_registers[0x0Fu] = ((variable_registers[vx] + variable_registers[vy]) > 0xFF);

  // If overflow, will wrap
  variable_registers[vx] += variable_registers[vy];
}

// Set VX -= VY with Carry VF
void Chip8::OP_8XY5()
{
  uint8_t vx = VX();
  uint8_t vy = VY();

  // Eval the variable register carry flag if not underflow
  variable_registers[0x0Fu] = (variable_registers[vx] > variable_registers[vy]);
  variable_registers[vx] -= variable_registers[vy];
}

// Set VX >>= 1 with Carry VF
void Chip8::OP_8XY6()
{
  uint8_t vx = VX();
  
  // In the CHIP-8 interpreter for the original COSMAC VIP, this opcode also set VX to VY
  if(quirk == COSMAC_VIP)
  {
    uint8_t vy = VY();
    variable_registers[vx] = variable_registers[vy];
  }

  variable_registers[0xFu] = variable_registers[vx] & 0x1u;
  variable_registers[vx] >>= 0x1u;
}

// Set VX = VY - VX with Carry VF
void Chip8::OP_8XY7()
{
  uint8_t vx = VX();
  uint8_t vy = VY();

  // Eval the variable register carry flag if not underflow
  variable_registers[0x0Fu] = (variable_registers[vy] > variable_registers[vx]);
  variable_registers[vx] = (variable_registers[vy] - variable_registers[vx]);
}

// Set VX <<= with Carry VF
void Chip8::OP_8XYE()
{
  uint8_t vx = VX();
  
  // In the CHIP-8 interpreter for the original COSMAC VIP, this opcode also set VX to VY
  if(quirk == COSMAC_VIP)
  {
    uint8_t vy = VY();
    variable_registers[vx] = variable_registers[vy];
  }

  variable_registers[0xFu] = variable_registers[vx] >> 0x7u;
  variable_registers[vx] <<= 0x1u;
}

// Skip if VX != VY
void Chip8::OP_9XYO()
{
  uint8_t vx = VX();
  uint8_t vy = VY();

  if(variable_registers[vx] != variable_registers[vy])
  {
    program_counter += 2;
  }
}

// Set Index to NNN
void Chip8::OP_ANNN()
{
  index_register = opcode & 0x0FFFu;
}

// Jump with Offset
void Chip8::OP_BNNN()
{
  if(quirk == CHIP_48 || quirk == SUPER_CHIP)
  {
    uint8_t vx = VX();
    uint16_t address = (static_cast<uint16_t>(vx) << 8) | (opcode & 0x00FFu);

    program_counter = address + variable_registers[vx];
  }
  else
  {
    uint16_t address = opcode & 0x0FFFu;
    program_counter = address + variable_registers[0];
  }
}

// Random
void Chip8::OP_CXNN()
{
  uint8_t vx = VX();
  uint16_t rand = 0 & vx;
}

// Display
void Chip8::OP_DXYN()
{
  uint8_t x = VX();
  uint8_t y = VY();
  uint8_t height = opcode & 0x000Fu;

  uint8_t xCoord = variable_registers[x] % DISPLAY_WIDTH;
  uint8_t yCoord = variable_registers[y] % DISPLAY_HEIGHT;
  variable_registers[0xFu] = 0;

  // TODO Adjust Implementation for more clarity
  for(uint16_t i = 0; i < height; ++i)
  {
    uint8_t spriteByte = memory[index_register + i];
    for(uint16_t j = 0; j < 8; ++j)
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

void Chip8::OP_EX9E()
{
}

void Chip8::OP_EXA1()
{
}

void Chip8::OP_FX07()
{
}

void Chip8::OP_FX15()
{
}

void Chip8::OP_FX18()
{
}

// Check overflow?
void Chip8::OP_FX1E()
{
  uint8_t vx = VX();
  index_register += variable_registers[vx];
}

void Chip8::OP_FX33()
{
  uint8_t vx = VX();

  uint8_t value = variable_registers[vx];
  memory[index_register] = (value / 100) % 10;
  memory[index_register + 1] = (value / 10) % 10;
  memory[index_register + 2] = value % 10;
}

void Chip8::OP_FX55()
{
  uint8_t vx = VX();

  for(uint8_t i = 0; i <= vx; ++i)
  {
    memory[index_register + i] = variable_registers[i];
    //++index_register;
  }
}

void Chip8::OP_FX65()
{
  uint8_t vx = VX();

  for(uint8_t i = 0; i <= vx; ++i)
  {
    variable_registers[i] = memory[index_register + i];
  }
}

void Chip8::OP_NULL()
{
}

uint8_t Chip8::VX()
{
  return (opcode & 0x0F00u) >> 8u;
}

uint8_t Chip8::VY()
{
  return (opcode & 0x00F0u) >> 4u;
}