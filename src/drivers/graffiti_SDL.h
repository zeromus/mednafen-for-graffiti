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

  void SetScale(scale_t x, scale_t y);
  void SetLineToolSize(wh_t w, wh_t h);
  void Enable(bool e=true);

private:
  LineTool *ltool {nullptr};
  
  SDL_Cursor *tool_cursor[static_cast<int>(LineToolType::amount)] {nullptr, nullptr};
  SDL_Cursor *syscursor {SDL_GetCursor()};

  void CreateCursor(LineToolType ltt, bool set);
  void SetCursor(LineToolType ti);

  void CreateLineToolCursors();
  void ShowCursor(bool s=true);
};

#endif
