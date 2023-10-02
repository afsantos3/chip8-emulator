#include <iostream>
#include <chrono>

#include "chip8.h"
#include "display.h"

int main(int argc, char* argv[])
{
  
  if(argc == 2 && strcmp(argv[1], "quirks") == 0)
  {
    std::cout << "Default 0, COSMAC_VIP 1";
    return 0;
  }
  if(argc != 3)
  {
    std::cerr << "Usage: " << argv[0] << "<Delay> <ROM> Optional:<Quirk>\n";
		std::exit(EXIT_FAILURE);
  }

  const int delay = std::stoi(argv[1]);
  const char* rom = argv[2];
  int quirk = 0; // Default

  if(argc == 4)
  {
    quirk = std::stoi(argv[3]);
  }

  Display display;

  Chip8 chip8;
  chip8.quirk = quirk;
  chip8.Load(rom);

  //TODO Change time stuff
  auto lastCycleTime = std::chrono::high_resolution_clock::now();

  while(true)
  {
    auto currentTime = std::chrono::high_resolution_clock::now();
		float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastCycleTime).count();

    if(dt > delay)
    {
      lastCycleTime = currentTime;
      
      chip8.Run();
      display.Update(chip8.display);
    }
  }

  return 0;
}