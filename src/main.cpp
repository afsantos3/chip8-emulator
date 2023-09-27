#include <iostream>
#include <chrono>

#include "chip8.h"
#include "display.h"

int main(int argc, char* argv[])
{
  if(argc != 3)
  {
    std::cerr << "Usage: " << argv[0] << " <Delay> <ROM>\n";
		std::exit(EXIT_FAILURE);
  }

  const int delay = std::stoi(argv[1]);
  const char* rom = argv[2];

  Display display;

  Chip8 chip8;
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