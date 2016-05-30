#include "graffiti_SDL.h"

void Graffiti_SDL::Input_Event(const SDL_Event& event)
{
  if(!enabled || !active)
  {
    //MDFN_printf("RETURNING\n");
    return;
  }
  /* Hack to disallow a "focus-click" to act as a paint event */
  /* tested on: Linux only */
  // set true means ignore the event NEXT time this function is called
  static bool ignore {false};
  if (ignore)
  {
    ignore = false;
    return;
  }

  switch(event.type)
  {
  case SDL_USEREVENT:
    switch(event.user.code)
    {
    case CEVT_SET_INPUT_FOCUS:
      ignore = true;
      MDFN_printf("CEVT_SET_INPUT_FOCUS event\n");
      return;
    default:
      break;
    }
  default:
    break;
  }
  /* End Hack */

  switch(event.type)
  {
  case SDL_MOUSEBUTTONDOWN:
    if(event.button.state == SDL_PRESSED)
    {
      MDFN_printf ("painting TRUE\n");
      painting = true;
      auto xy = MouseCoords2SurfaceCoords(event.button.x, event.button.y);

      switch (event.button.button)
      {
      case SDL_BUTTON_RIGHT:  // eraser
        if (ltool != &eraser_tool)
        {  
          ltool = &eraser_tool;
          SetCursor(*ltool);
        }
        break;
      default:
        if (ltool != &line_tool)
        {  
          ltool = &line_tool;
          SetCursor(*ltool);
        }
        ltool->color = {
          (rand() % 7 + 1) * 32, (rand() % 7 + 1) * 32, (rand() % 7 + 1) * 32
        };
        break;
      }

      auto xyc = CalculateCenterCoords(xy.first, xy.second, ltool->w, ltool->h);
      ltool->x0 = xyc.first;
      ltool->y0 = xyc.second;

      Paint(*ltool);
    }
    break;

  case SDL_MOUSEBUTTONUP:
    if(event.button.state == SDL_RELEASED)
    {
      //MDFN_printf ("painting FALSE\n");
      painting = false;
    }
    break;

  case SDL_MOUSEMOTION:
    if(painting) {
      // Continue painting
      //MDFN_printf ("painting MOTION\n");
      auto xy = MouseCoords2SurfaceCoords(event.button.x, event.button.y);
      auto xyc = CalculateCenterCoords(xy.first, xy.second, ltool->w, ltool->h);
      auto x1 = xyc.first;
      auto y1 = xyc.second;

      Line(*ltool, x1, y1);

      ltool->x0 = x1;
      ltool->y0 = y1;
    }
    break;

    default:break;
  }
}

void Graffiti_SDL::ShowCursor(bool s)
{
  SDL_MDFN_ShowCursor(s);
}

void Graffiti_SDL::SetCursor(LineTool& lt)
{

}
