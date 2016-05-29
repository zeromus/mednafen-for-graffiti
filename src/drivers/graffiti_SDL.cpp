#include "graffiti_SDL.h"

Graffiti_SDL::Graffiti_SDL(MDFN_Surface *new_canvas) :Graffiti(new_canvas)
{}

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
        ltool = &eraser_tool;
        break;
      default:
        ltool = &line_tool;
        ltool->color = {
          (rand() % 7 + 1) * 32, (rand() % 7 + 1) * 32, (rand() % 7 + 1) * 32
        };
        break;
      }

      auto xyc = CalculateCenterCoords(xy.first, xy.second, ltool->w, ltool->h);
      ltool->x0 = xyc.first;
      ltool->y0 = xyc.second;

      Paint(ltool->x0, ltool->y0, ltool->w, ltool->h, ltool->color, true);
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
      coord_t x1 = xyc.first;
      coord_t y1 = xyc.second;

      Line(ltool->x0, ltool->y0, x1, y1, ltool->w, ltool->h, ltool->color, true);

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
