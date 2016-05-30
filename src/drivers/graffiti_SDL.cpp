#include "graffiti_SDL.h"

Graffiti_SDL::Graffiti_SDL(MDFN_Surface *new_canvas) :Graffiti(new_canvas)
{

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

      switch (event.button.button)
      {
      case SDL_BUTTON_RIGHT:  // eraser
        // if (ltool != &eraser_tool)
        // {
          ltool = &eraser_tool;
          //SetCursor(*ltool);
        //}
        break;
      default:
        //{
          ltool = &line_tool;
          //SetCursor(*ltool);
        //}
        ltool->color = {
          (rand() % 7 + 1) * 32, (rand() % 7 + 1) * 32, (rand() % 7 + 1) * 32
        };
        break;
      }

      auto xyc = CalculateCenterCoords(xy.first, xy.second, ltool->w, ltool->h);
      ltool->x0 = xyc.first;
      ltool->y0 = xyc.second;
      SetCursor(*ltool);
      Paint(*ltool);
    }
    break;

  case SDL_MOUSEBUTTONUP:
    if(event.button.state == SDL_RELEASED)
    {
      //MDFN_printf ("painting FALSE\n");
      painting = false;
      if (cursor)
      {
        SDL_MDFN_SetCursor(syscursor);
        SDL_MDFN_FreeCursor(cursor);
        cursor = nullptr;
      }
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

static inline Graffiti::coord_t fix(Graffiti::coord_t w)
{
  constexpr Graffiti::coord_t W = 8;
  auto dx = w % W;
  if (w/W == 0)
    w = W;
  else if (dx >= (W/2))
    w += W - dx;
  else
    w -= dx;

  return w;
}

void Graffiti_SDL::SetCursor(LineTool& lt)
{ // assumes width and height are "sanely bounded"
  if (cursor)
  {
    SDL_MDFN_SetCursor(syscursor);
    SDL_MDFN_FreeCursor(cursor);
    cursor = nullptr;
  }

  coord_t w = lt.w * view.xscale;
  coord_t h = lt.h * view.yscale;

  MDFN_printf("w: %d, h: %d\n", w, h);

  w = fix(w);
  h = fix(h);

  MDFN_printf("w2: %d, h2: %d\n", w, h);

  auto w8 = w/8;

  MDFN_printf("w8: %d\n", w8);


  std::vector<uint8> data(w8*h, 255), mask(w8*h, 255);

  // want to outline the edge!
  for (int x=0; x < w8; x++)
  {
    data[x] = 0x0;
    data[(h-1)*w8 + x] = 0;
  }
  for (int y=1; y < (h-1); y++)
  {
    data[y*w8 + 0] = 0x7f;
    data[y*w8 + (w8-1)] &= 0xfe;
  }

  SDL_MDFN_CreateCursor(&cursor, &data[0], &mask[0], w, h, w/2, h/2);
  //SDL_MDFN_SetCursor(cursor);
}
