#ifndef __MDFN_NETPLAY_GRAFFITI_H
#define __MDFN_NETPLAY_GRAFFITI_H

#define ENABLE_GRAFFITI

#include "netplay-text.h"
#include "drivers/main.h"

class Graffiti : public TextCommand
{
  using scale_t = double;
  using cmd_t = uint8;
  enum Command : cmd_t;

public:
  Graffiti(MDFN_Surface *newcanvas);
  ~Graffiti();

  // most public-facing functions - should not operate if plugin disabled
  void Input_Event(const SDL_Event& event);
  void Blit(MDFN_Surface *target);
  bool Broadcast(); // returns true if actually broadcasted
  bool will_broadcast {false};

  void Enable(bool e=true);
  void Disable();

  bool ConsoleParse(const char *msg);
  void SetScale(const scale_t& x, const scale_t& y);

  void Send(Command command);

private:
  bool Process(const char *nick, const char *msg, uint32 len, bool &display);
  void Paint(const int& x, const int& y);

  bool painting {false};

  enum Command : cmd_t { paint, sync, clear };

  struct View {
    View(MDFN_Surface *newcanvas);
    ~View();
    void Clear();
    MDFN_Surface *canvas {nullptr};
    uint8 red, green, blue;
    uint32 width {5}, height {5};
    scale_t xscale {1}, yscale {1};
  } view;
};

#endif
