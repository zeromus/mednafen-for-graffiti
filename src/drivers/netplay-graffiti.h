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
#ifndef __MDFN_NETPLAY_GRAFFITI_DRIVER_H
#define __MDFN_NETPLAY_GRAFFITI_DRIVER_H

#define ENABLE_GRAFFITI // comment to disable GRAFFITI from compiling

#include "main.h"
#include "../netplay-graffiti.h"

class GraffitiDriver : public Graffiti
{
public:
  GraffitiDriver(MDFN_Surface *new_canvas);

  void ShowCursor(bool s=true);
  // the main operating points (MOP)
  // user input draws to an internal-surface
  void Input_Event(const void* ev);
};

#endif
