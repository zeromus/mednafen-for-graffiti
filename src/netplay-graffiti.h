#ifndef __MDFN_NETPLAY_GRAFFITI_H
#define __MDFN_NETPLAY_GRAFFITI_H

#define ENABLE_GRAFFITI

#include "netplay-text.h"
#include "drivers/main.h"

class Graffiti : public TextCommand
{
  using scale_t = double;
public:
  Graffiti(MDFN_Surface *newcanvas);
  ~Graffiti();
  void Enable(bool e=true);
  void Disable();
  void Toggle();
  void SetScale(const scale_t& x, const scale_t& y);
  void Input_Event(const SDL_Event& event);
  void Clear();
  void Blit(MDFN_Surface *target);

  bool Broadcast(); // returns true if actually broadcasted
  bool will_broadcast {false};
private:
  bool Process(const char *nick, const char *msg, uint32 len, bool &display);
  void Paint(const int& x, const int& y);

  bool enabled {false};
  bool painting {false};

public:
  struct View {
    View(MDFN_Surface *newcanvas);
    ~View();
    MDFN_Surface *canvas {nullptr};
    uint8 red, green, blue;
    uint32 width {5}, height {5};
    scale_t xscale {1}, yscale {1};
  } view;
};

#endif
