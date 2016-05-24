/* Working emulators "out-of-the-box" (various config settings NOT tested yet)
  - SNES
  - NES
  - NGP
  - GBC
  - SMS

Problematic Emulators
  - PCE (needs mouse scroll/offset setting impl!)

Unlisted emulators haven't been tested yet
*/
#ifndef __MDFN_NETPLAY_GRAFFITI_H
#define __MDFN_NETPLAY_GRAFFITI_H

#include <thread>

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

  // init video
  void SetScale(const scale_t& x, const scale_t& y);

  bool ConsoleParse(const char *msg);

  void Enable(bool e=true);
  void Disable();

  void ClearLocal();
  void ClearRemote();

  void Send(Command command);

  void RecvSync(const char *msg, uint32 len);

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
