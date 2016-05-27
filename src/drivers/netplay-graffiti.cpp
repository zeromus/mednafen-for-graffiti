#include "netplay-graffiti.h"

GraffitiDriver::GraffitiDriver(MDFN_Surface *new_canvas) :Graffiti(new_canvas)
{}

void GraffitiDriver::ShowCursor(bool s)
{
  SDL_MDFN_ShowCursor(s);
}

void GraffitiDriver::Input_Event(const void* ev)
{
  const SDL_Event& event = *reinterpret_cast<const SDL_Event*>(ev);

  if(!enabled || !active)
    return;

  /* Hack to disallow a "focus-click" to act as a paint event */
  static SDL_Event last {};
  switch(last.type)
  {
  case SDL_USEREVENT:
    switch(last.user.code)
    {
    case CEVT_SET_INPUT_FOCUS:
      last = event;
      return;
    }
    break;
  default:
    last = event;
    break;
  }
  /* End Hack */

  switch(event.type)
  {
  case SDL_MOUSEBUTTONDOWN:
    if(event.button.state == SDL_PRESSED)
    {
      //printf ("painting TRUE\n");
      painting = true;
      view.red = (rand() % 8) * 32;
      view.green = (rand() % 8) * 32;
      view.blue = (rand() % 8) * 32;
      Paint(event.button.x, event.button.y);
      view.x0 = event.button.x;
      view.y0 = event.button.y;
    }
    break;

  case SDL_MOUSEBUTTONUP:
    if(event.button.state == SDL_RELEASED)
    {
      //printf ("painting FALSE\n");
      painting = false;
    }
    break;

  case SDL_MOUSEMOTION:
    if(painting) {
      // Continue painting
      //printf ("painting MOTION\n");
      Line(view.x0, view.y0, event.motion.x, event.motion.y);
    }
    break;

    default:break;
  }
}