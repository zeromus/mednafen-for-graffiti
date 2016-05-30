#include "graffiti_SDL.h"

Graffiti_SDL::Graffiti_SDL(MDFN_Surface *new_canvas) :Graffiti(new_canvas)
{
  ltool = &line_tool[static_cast<int>(LineToolType::line)];
}

void Graffiti_SDL::SetScale(scale_t x, scale_t y)
{
  Graffiti::SetScale(x, y);

  CreateLineToolCursors();
}

void Graffiti_SDL::Enable(bool e)
{
  MDFN_printf("YEEHAW)");
  Graffiti::Enable(e);
  if (e)
    SetCursor(LineToolType::line);
  else SDL_MDFN_SetCursor(syscursor);
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
      if (!MDFNDHaveFocus)
        SDL_SetCursor(syscursor);
      else
      {
        ignore = true;
        SDL_SetCursor(tool_cursor[static_cast<int>(LineToolType::line)]);
      }
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
    if (event.button.button == SDL_BUTTON_WHEELUP)
    {
      LineTool& lt = line_tool[static_cast<int>(LineToolType::line)];
      int w = lt.w + 1, h = lt.h + 1;
      if (w > 0 && h > 0)
        SetLineToolSize(w, h);
    }
    else if (event.button.button == SDL_BUTTON_WHEELDOWN)
    {
      LineTool& lt = line_tool[static_cast<int>(LineToolType::line)];
      int w = lt.w - 1, h = lt.h - 1;
      if (w > 0 && h > 0)
        SetLineToolSize(w, h);
    }
    else if(event.button.state == SDL_PRESSED)
    {
      MDFN_printf ("painting TRUE\n");
      painting = true;
      auto xy = MouseCoords2SurfaceCoords(event.button.x, event.button.y);

      LineToolType ti = LineToolType::line;
      switch (event.button.button)
      {
      case SDL_BUTTON_RIGHT:  // eraser
        ti = LineToolType::eraser;
        ltool = &line_tool[static_cast<int>(ti)];
        SetCursor(ti);
        break;
      default:
        ti = LineToolType::line;
        ltool = &line_tool[static_cast<int>(ti)];
        ltool->color = {
          (rand() % 7 + 1) * 32, (rand() % 7 + 1) * 32, (rand() % 7 + 1) * 32
        };
        ShowCursor(false);
        //SDL_MDFN_SetCursor(syscursor);
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
      SDL_MDFN_SetCursor(tool_cursor[static_cast<int>(LineToolType::line)]);
      ShowCursor(true);
      ltool = &line_tool[static_cast<int>(LineToolType::line)];
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
  else if (dx)
    w += W - dx;
  //else
    //w -= dx;

  return w;
}

void Graffiti_SDL::CreateLineToolCursors()
{
  for (int i=0; i < static_cast<int>(LineToolType::amount); i++)
    CreateCursor(static_cast<LineToolType>(i), false);
}

void Graffiti_SDL::SetCursor(LineToolType ti)
{
  SDL_MDFN_SetCursor(tool_cursor[static_cast<int>(ti)]);
}

void Graffiti_SDL::CreateCursor(LineToolType ltt, bool set)
{ // assumes width and height are "sanely bounded"
  int ti = static_cast<int>(ltt);
  if (tool_cursor[ti])
  {
    SDL_MDFN_FreeCursor(tool_cursor[ti]);
    tool_cursor[ti] = nullptr;
  }

  LineTool& lt = line_tool[ti];

  MDFN_printf("lt.w: %d, lt.h: %d\n", lt.w, lt.h);

  coord_t w = (lt.w+2) * view.xscale;
  coord_t h = (lt.h+2) * view.yscale;

  auto dx = w % 8;
  MDFN_printf("w: %d, h: %d, dx: %d\n", w, h, dx);

  auto mouse_w = fix(w);
  //h = fix(h); // technically, h need not be a multiple of 8, but 8x1 cursor for
              // a 1x1 pixel is ugly, so I fix the height too

  MDFN_printf("w2: %d, h2: %d\n", mouse_w, h);

  auto w8 = mouse_w/8;

  MDFN_printf("w8: %d\n", w8);

  constexpr uint8 Lut[] = {0x0, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe};

  if (ltt == LineToolType::line)
  {
    std::vector<uint8> data(w8*h, 255), mask(w8*h, 255);

    // want to outline the edge!
    for (int x=0; x < w8; x++)
    {
      data[x] = 0x0;
      data[(h-1)*w8 + x] = 0;
    }
    for (int y=0; y < h; y++)
    {
      data[y*w8 + 0] &= 0x7f;
      data[y*w8 + (w8-1)] &= 0xfe;

      if (dx)
      {
        data[y*w8 + w8-1] &= Lut[dx-1];
        mask[y*w8 + w8-1] &= Lut[dx];
      }
    }

    CursorSpec_SDL sc = { &tool_cursor[ti], &data[0], &mask[0], mouse_w, h, w/2, h/2, set };
    SDL_MDFN_CreateCursor(&sc);
  }
  else if (ltt == LineToolType::eraser)
  {
    std::vector<uint8> data(w8*h, 0), mask(w8*h, 0);

    // want to outline the edge!
    for (int x=0; x < w8; x++)
    {
      mask[x] = 0xff;
      mask[(h-1)*w8 + x] = 0xff;

      uint8 v;
      if (h >= 2)
      {
        if (!x) v = 0x7f; else if (x == w8-1) v = 0xfe; else v = 0xff;
        mask[(1)*w8 + x] = v;
        data[(1)*w8 + x] = v;
        mask[(h-1-1)*w8 + x] = v;
        data[(h-1-1)*w8 + x] = v;
      }
      if (h >= 3)
      {
        if (!x) v = 0x3f; else if (x == w8-1) v = 0xfc; else v = 0xff;
        mask[(2)*w8 + x] = v;
        mask[(h-1-2)*w8 + x] = v;
      }
    }
    for (int y=1; y < (h-1); y++)
    {
      mask[y*w8 + 0] |= 0xe0;
      mask[y*w8 + (w8-1)] |= 0x07;
      data[y*w8 + 0] |= 0x40;
      data[y*w8 + (w8-1)] |= 0x02;
    }
    CursorSpec_SDL sc = { &tool_cursor[ti], &data[0], &mask[0], mouse_w, h, w/2, h/2, set };
    SDL_MDFN_CreateCursor(&sc);
  }
}

void Graffiti_SDL::SetLineToolSize(wh_t w, wh_t h)
{
  Graffiti::SetLineToolSize(w, h);
  // CreateLineToolCursors();
  for (int i=0; i < static_cast<int>(LineToolType::amount); i++)
  {
    bool set = false;
    if (ltool == &line_tool[i])
    {
      printf ("i == %d\n", i);
      set = true;
    }
    CreateCursor(static_cast<LineToolType>(i), set);
  }
}
