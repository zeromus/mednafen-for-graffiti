// graffiti.h: It's FREAKIN GRAFFITI OVER NETPLAY
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
/* CRITICAL CHANGES
 * Increased MDFNNP_STC size from 2,000 to 20,000
  * I WILL be incorporating a new implementation so that each Textcommand has its own limit,
  and so that regular MDFNNP_STC message limit is once again restored to original value.

 * Added MDFN_Surface::Size function
*/
#ifndef __MDFN_NETPLAY_GRAFFITI_H
#define __MDFN_NETPLAY_GRAFFITI_H

#define ENABLE_GRAFFITI // comment to disable GRAFFITI from compiling

#include "netplay-STC.h"
#include "netplay-private.h"
#include "video.h"

class Graffiti : public MDFNNP_STC
{
  using scale_t = double;
  using wh_t = uint16;  // width/height
  using color_t = uint32;
  using cmd_t = uint8;
  enum Command : cmd_t { paint, line, sync, clear };
  // I am using older style enum because it is easier to cast, given that I
  // cast to/from this enum through template functions...
  // (MDFNNP_STC class's << and >> operators)

public:
  static const CommandEntry ConsoleCommandEntry;
  static bool ConsoleCommandParser(const char *arg);
  bool ParseConsoleCommand(const char *msg);

  /* takes in a new surface, which we must personally delete. Normally, I would
  not be a fan of allocating the surface outside of the constructor, but it
  actually simplifies the code by not requiring to maintain a separate copy of
  MDFN_Surface constructor arguments */
  Graffiti(MDFN_Surface *new_surface);

  /* I would have preferred this next function as part of the constructor,
  but given the segregated nature of mednafen's video init code, it had to be
  separated.

  This class assumes that SetScale has been called appropriately before
  any drawing routines are called */
  void SetScale(scale_t x, scale_t y);

  // the main operating points (MOP)
  // A) user input draws to an internal surface
  // Input_Event() defined in driver (eg. drivers/netplay-graffiti_SDL)

  // B) internal surface gets blitted to the pre-scaled gameplay-surface
  void Blit(MDFN_Surface *target);

  /* C) as part of the State-Loading paradigm, the user who receives a state request
  automatically broadcasts their surface to all other players */
  bool Broadcast(); // returns true if actually broadcasted (ie when module enabled)

  // Enable() refers to whether the module is allowed to receive network events
  // and conditionally affects the main operating points (MOP).
  /* Normally, the MOP should not function if this module is disabled. However,
  I have deemed it best practice to keep the module enabled even when the user
  chooses to disable it (via `/g disable`), so that the surface can continue to
  stay in sync. That way a user who later chooses to re-enable graffiti during
  the session is already in sync, thus avoiding the need to implement 
  more complicated communications protocol to "sync" from outside of a regular
  "REQUEST_STATE/LOADSTATE" combo event.
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
  using coord_t = uint16;
  // Call from driver mouse-capture routine
  std::pair<coord_t, coord_t> MouseCoords2SurfaceCoords(coord_t x, coord_t y);

  struct Color
  {
    static MDFN_Surface *s;
    Color() = default;
    Color(color_t rgba);
    Color(uint8 r, uint8 g, uint8 b, uint8 a =0);
    uint8 r{}, g{}, b{}, a{};
    color_t rgba{};
  };

  static constexpr wh_t Default_width = 5, Default_height = 5;
  struct LineTool {
    LineTool() = default;
    LineTool(wh_t w, wh_t h, Color c={0,0,0}) : w{w}, h{h}, color{c} {}
    void SetSize(wh_t w, wh_t h) { if(w) this->w = w; if(h) this->h = h; }
    wh_t w{Default_width}, h{Default_height};
    coord_t x0 {0}, y0 {0};
    Color color;
  } line_tool, eraser_tool;

  // All drawing routines expect surface coordinates (not raw mouse coords)
  void Paint(const LineTool& lt);
  void Line(const LineTool& lt, coord_t x1, coord_t y1);

  bool painting {false};
  bool active {false};

  struct View {
    View(MDFN_Surface *new_surface);
    ~View();
    void Clear();
    std::vector<uint8> Compress();
    void Uncompress(uLongf dlen, const void *msg, uint32 len);
    MDFN_Surface *surface {nullptr};
    // default of 0 to assist in noticing whether SetScale was called in time
    scale_t xscale {0}, yscale {0};
  } view;

  std::pair<coord_t, coord_t> CalculateCenterCoords(
    coord_t x, coord_t y, wh_t w, wh_t h);

private:
  virtual void ShowCursor(bool s=true)=0;
  bool Process(const char *nick, const char *msg, uint32 len, bool &display);
  void RecvPaint(const char *nick, const char *msg, const uint32 len);
  void RecvLine(const char *nick, const char *msg, const uint32 len);
  void RecvSync(const char *nick, const char *msg, const uint32 len);
  void RecvClear(const char *nick, const char *msg, const uint32 len);

  void Paint(
    coord_t x, coord_t y, wh_t w, wh_t h,
    const Color& color, const bool broadcast);
  void Line(
    coord_t x0, coord_t y0, coord_t x1, coord_t y1,
    wh_t w, wh_t h, const Color& color, const bool broadcast);

  static constexpr limit_t Payload_limit = 20000;
  static constexpr magic_t Magic_id = 0xf171;
};

#endif
