// netplay-graffiti.h: It's FREAKIN GRAFFITI OVER NETPLAY
/*
Graffiti allows you to draw on the surface of your game. It's intended to be a fun
addition to the netplay experience.

History: Zeromus gave birth to this idea when I was playing a round of Kirby's
Dream Course with him over net play. I quote:
<zeromus> you know what would be fun is if you could draw graffiti on a netplay session

I felt like I could accomplish it, but I was very busy at the time. So I filed
it as a TODO item on my mednafen Github repository and in a couple weeks began
working full time on it. It's been about 3 days now and it's almost finished.
*/

/* Working emulators "out-of-the-box" (default settings tested only)
  - SNES
  - NES
  - NGP
  - GBC
  - SMS

Problematic Emulators
  - PCE (needs mouse scroll/offset setting impl!)

Unlisted emulators haven't been tested yet

Bresenham Line algo from http://rosettacode.org/wiki/Bitmap/Bresenham%27s_line_algorithm#C
*/
#ifndef __MDFN_NETPLAY_GRAFFITI_H
#define __MDFN_NETPLAY_GRAFFITI_H

#define ENABLE_GRAFFITI // comment to disable GRAFFITI from compiling

#include "netplay-text.h"
#include "netplay-private.h"
#include "video.h"

class Graffiti : public TextCommand
{
  using scale_t = double;
  using cmd_t = uint8;
  enum Command : cmd_t { paint, sync, clear };
  // I am using older style enum because it is easier to cast, given that I
  // cast to/from this enum through template functions...
  // (TextCommand class's << and >> operators)

public:
  static const CommandEntry ConsoleCommandEntry;
  static bool ConsoleCommandParser(const char *arg);

  Graffiti(MDFN_Surface *new_surface);
  ~Graffiti();

  // the main operating points (MOP)
  // user input draws to an internal-surface
  // Input_Event function defined in drivers/netplay-graffiti_SDL
  // which gets blitted to the pre-scaled main-surface
  void Blit(MDFN_Surface *target);
  // the server-designated user automatically runs this function to broadcast
  // their surface to the other players (a part of the State-Loading paradigm)
  bool Broadcast(); // returns true if actually broadcasted (ie when module enabled)

  bool ParseConsoleCommand(const char *msg);

  // init the object with this function as soon as possible
  void SetScale(const scale_t& x, const scale_t& y);

  // Enabling refers to whether the module is allowed to receive TextCommands
  // and has been extended to include main operating points (MOP).
  /* Normally, the MOP should not operate if this module is disabled. However,
  I have deemed it best practice to keep the module enabled even when the user
  chooses to disable it (via the console), so that the surface can continue to
  stay in sync. That way a user who later chooses to re-enable graffiti during
  the session is already in sync, thus avoiding the need to implement 
  more complicated communications protocol to "sync" from outside of a
  "REQUEST_STATE/LOADSTATE" combo event, which is currently the only way to sync.
  */
  // For this reason, I have added a higher layer called "Activation", allowing
  // the module to be enabled but not "activated" allows fine-grained control.
  // In this case, to not display or process input, but to receive surface updates.
  void Enable(bool e=true);
  void Disable();
  void Activate(bool e=true);
  void Deactivate();
  void ToggleActivate();

  void ClearLocalSurface();
  void ClearRemoteSurfaces();

  void Send(Command command);

protected:
  void Paint(
    const int& x, const int& y, const uint32& w, const uint32& h,
    const uint32& bg_color, const bool broadcast);
  void Line(
    int& x0, int& y0, const int& x1, const int& y1,
    const uint32& w, const uint32& h, const uint32& bg_color);
  std::pair<int, int> MouseCoords2SurfaceCoords(const int& x, const int& y);

  bool painting {false};
  bool active {false};

  struct View {
    View(MDFN_Surface *new_surface);
    ~View();
    void Clear();
    std::vector<uint8> Compress();
    MDFN_Surface *surface {nullptr};
    uint8 red, green, blue;
    uint32 bg_color;
    uint32 width {5}, height {5};
    int x0 {0}, y0 {0};
    scale_t xscale {1}, yscale {1};
  } view;

private:
  virtual void ShowCursor(bool s=true)=0;
  bool Process(const char *nick, const char *msg, uint32 len, bool &display);
  void RecvSync(const char *msg, uint32 len);
};

#endif
