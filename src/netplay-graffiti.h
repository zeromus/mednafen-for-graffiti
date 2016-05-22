#ifndef __MDFN_NETPLAY_GRAFFITI_H
#define __MDFN_NETPLAY_GRAFFITI_H
#include "netplay-text.h"
#include "drivers/main.h"

class Graffiti : public TextCommand
{
  using scale_t = double;
public:
  Graffiti(MDFN_Surface *newcanvas);
  ~Graffiti() { delete canvas; canvas = nullptr; }
  void Enable(bool e=true);
  void Disable();
  void Toggle();
  void SetScale(const scale_t& x, const scale_t& y) { xscale = x; yscale = y; }
  void Input_Event(const SDL_Event &event);
  void Clear();
  void Blit(MDFN_Surface *target, const int xpos, const int ypos);
private:
  void Paint(int x, int y, uint8 red, uint8 green, uint8 blue);
  bool Process(const char *nick, const char *msg, uint32 len, bool &display);
  MDFN_Surface *canvas {nullptr};
  bool painting {false};
  uint8 red, green, blue;
  scale_t xscale{1}, yscale{1};
  bool enabled {false};
};

#endif
