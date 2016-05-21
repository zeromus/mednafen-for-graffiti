#pragma once
#include "netplay-text.h"
#include "drivers/main.h"

class Graffiti : public TextCommand
{
public:
  Graffiti(MDFN_Surface *newcanvas);
  ~Graffiti() { delete canvas; canvas = nullptr; }
  //void Send(const std::string &msg = "");
  void Input_Event(const SDL_Event &event);
  void Clear() { if (canvas) canvas->Fill(0,0,0,0); }
  void Draw(MDFN_Surface *target, const int xpos, const int ypos);

private:
  void Paint(int x, int y, Uint8 red, Uint8 green, Uint8 blue);
  bool Process(const char *nick, const char *msg, uint32 len, bool &display);
  MDFN_Surface *canvas = nullptr;  // I think that's the right datatype..
  bool painting = false;
  uint8_t red, green, blue;
  int x=0, y=0;
};


