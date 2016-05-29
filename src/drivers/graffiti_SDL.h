#ifndef __MDFN_NETPLAY_GRAFFITI_SDL_H
#define __MDFN_NETPLAY_GRAFFITI_SDL_H

#include "main.h" // this is not a "lazy include"
#include "../graffiti.h"

class Graffiti_SDL : public Graffiti
{
public:
  Graffiti_SDL(MDFN_Surface *new_canvas);

  // the main operating points (MOP)
  // user input draws to an internal-surface
  void Input_Event(const SDL_Event& event);
private:
  void ShowCursor(bool s=true);
  LineTool *ltool {nullptr};
};

#endif
