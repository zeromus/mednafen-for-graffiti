#ifndef __MDFN_NETPLAY_GRAFFITI_DRIVER_H
#define __MDFN_NETPLAY_GRAFFITI_DRIVER_H

#include "main.h"
#include "../netplay-graffiti.h"

class GraffitiDriver : public Graffiti
{
public:
  GraffitiDriver(MDFN_Surface *new_canvas);

  // the main operating points (MOP)
  // user input draws to an internal-surface
  void Input_Event(const SDL_Event& event);
private:
  void ShowCursor(bool s=true);
};

#endif
