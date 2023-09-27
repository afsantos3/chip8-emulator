#include <iostream>
#include <cstdlib>
#include "display.h"

const unsigned int DISPLAY_WIDTH = 64;
const unsigned int DISPLAY_HEIGHT = 32;

const char PIXEL_ON = '#';
const char PIXEL_OFF = ' ';

Display::Display(){}

void Display::Update(const uint8_t* buffer)
{
  Clear();
  Print(buffer);
}

void Display::Print(const uint8_t* buffer)
{
  for(int i = 0; i < DISPLAY_HEIGHT; i++)
  {
    for(int j = 0; j < DISPLAY_WIDTH; j++)
    {
      if(buffer[(i * DISPLAY_WIDTH) + j]) 
        std::cout << PIXEL_ON; // if pixel is ON
      else 
        std::cout << PIXEL_OFF; // if pixel is OFF
    
    }
    std::cout << std::endl;
  }
}

void Display::Clear()
{
  std::cout << "\x1B[2J\x1B[3J\x1B[H";
}