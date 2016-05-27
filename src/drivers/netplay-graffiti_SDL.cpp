#include "netplay-graffiti_SDL.h"

Graffiti_SDL::Graffiti_SDL(MDFN_Surface *new_canvas) :Graffiti(new_canvas)
{}

void Graffiti_SDL::ShowCursor(bool s)
{
  SDL_MDFN_ShowCursor(s);
}

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
      view.x0 = xy.first;
      view.y0 = xy.second;

      switch (event.button.button)
      {
      case SDL_BUTTON_RIGHT:  // eraser
        view.red = 0;
        view.green = 0;
        view.blue = 0;
        break;
      default:
        view.red = (rand() % 7 + 1) * 32;
        view.green = (rand() % 7 + 1) * 32;
        view.blue = (rand() % 7 + 1) * 32;
        break;
      }
      view.bg_color = view.surface->MakeColor(view.red, view.green, view.blue);
      Paint(xy.first, xy.second, view.width, view.height, view.bg_color, true);
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
      Line(view.x0, view.y0, xy.first, xy.second, view.width, view.height, view.bg_color);
    }
    break;

    default:break;
  }
}