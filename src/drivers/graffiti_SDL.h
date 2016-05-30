#ifndef __MDFN_NETPLAY_GRAFFITI_SDL_H
#define __MDFN_NETPLAY_GRAFFITI_SDL_H

#include "main.h" // this is not a "lazy include"
#include "../graffiti.h"

class Graffiti_SDL : public Graffiti
{
public:
  using Graffiti::Graffiti; // use Graffiti's constructor

  // the main operating points (MOP)
  // user input draws to an internal-surface
  void Input_Event(const SDL_Event& event);
private:
  LineTool *ltool {nullptr};
  
  SDL_Cursor *cursor {nullptr};
  void SetCursor(LineTool& lt);
  void ShowCursor(bool s=true);
};

#endif
