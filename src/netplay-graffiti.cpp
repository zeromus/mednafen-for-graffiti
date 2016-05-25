#include "netplay-graffiti.h"

static bool CC_graffiti(const char *arg)
{
  extern Graffiti *graffiti;

  return graffiti->ParseConsoleCommand(arg);
}

// TODO: Write proper help stanza
const CommandEntry GraffitiCommand {
  "/g", CC_graffiti,
  gettext_noop(""),
  "Graffiti"
};

Graffiti::Graffiti(MDFN_Surface *new_canvas) :
  TextCommand(0xf171),
  view{new_canvas}
{
}


Graffiti::View::View(MDFN_Surface *new_canvas)
{
  if (canvas)
  {
    delete canvas;
  }

  canvas = new_canvas;
  canvas->Fill(0, 0, 0, 0);
}

Graffiti::View::~View()
{
  if (canvas)
  {
    delete canvas;
    canvas = nullptr;
  }
}

void Graffiti::View::Clear()
{
  if (canvas)
  {
    canvas->Fill(0,0,0,0);
  }
}
///////////////
bool Graffiti::ParseConsoleCommand(const char *arg)
{
  if (!strcmp("", arg))
    ToggleActivate();
  else if (!strcmp("enable", arg) || !strcmp("en", arg))
    Activate();
  else if (!strcmp("disable", arg) || !strcmp("dis", arg))
  {
    Deactivate();
    return false;
  }
  else if (!strcmp("clear", arg))
  {
    ClearLocalSurface();
    ClearRemoteSurfaces();
    return false;
  }

  return true;  // keep console open
}

void Graffiti::ClearLocalSurface() { view.Clear(); }
void Graffiti::ClearRemoteSurfaces() { Send(Command::clear); }
///////////////
void Graffiti::Enable(bool e)
{
  active = e;
  TextCommand::Enable(e);
  SDL_MDFN_ShowCursor(e);
}

void Graffiti::Disable()
{
  active = false;
  TextCommand::Disable();
  SDL_MDFN_ShowCursor(0);
  ClearLocalSurface();
}

void Graffiti::Activate(bool e)
{
  SDL_MDFN_ShowCursor(e);
  active = e;
}

void Graffiti::Deactivate()
{
  SDL_MDFN_ShowCursor(0);
  active = false;
}

void Graffiti::ToggleActivate()
{
  Activate(!active);
}
////////////////
void Graffiti::SetScale(const scale_t& x, const scale_t& y)
{
  view.xscale = x;
  view.yscale = y;
}

bool Graffiti::Broadcast()
{
  if(!enabled)
    return false;

  printf("BROADCASTING\n");
  // compress and send view.surface
  std::vector<uint8> cbuf;
  uLongf clen;

  for (int i=0; i < 24; i++)
    printf("0x%02x ", static_cast<unsigned char>(view.canvas->pixels[i]));

  fflush(stdout);

  *this << static_cast<cmd_t>(Command::sync);

  // TODO / WARNING -- this relies on all client surfaces using the same BPP
  clen = view.canvas->Size() + view.canvas->Size() / 1000 + 12;
  cbuf.resize(4 + clen);
  MDFN_en32lsb(&cbuf[0], view.canvas->Size());
  compress2((Bytef *)&cbuf[0] + 4, &clen, (Bytef *)view.canvas->pixels, view.canvas->Size(), 7);

  fprintf(stderr, "Clen = %d", clen);
  // WARNING INEFFICIENT - just to see if it works first
  // ideally cbuf should be the istr to begin with (resized and at the proper index)
  cbuf.resize(clen + 4);
  for (auto i : cbuf)
    *this << i;

  printf("canvas size: %d\n", view.canvas->Size());

  Send(Command::sync);
  return true;
}

void Graffiti::Blit(MDFN_Surface *target)
{
  if(!enabled || !active)
    return;

  // iterate through the canvas
  // any pixel sets that aren't 00, copy over to target
  uint32 *canvas_pixels = (uint32*)view.canvas->pixels;
  uint32 *target_pixels = (uint32*)target->pixels;

  for(uint32 i = 0; i < target->pitchinpix * target->h; i++)
    if(canvas_pixels[i])
      target_pixels[i] = canvas_pixels[i];
}

void Graffiti::Input_Event(const SDL_Event &event)
{
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

void Graffiti::Paint(const int& x, const int& y)
{
  printf("x: %d, y: %d\n", x, y);
  printf("sx: %f, ox: %f\n", CurGame->mouse_scale_x, CurGame->mouse_offs_x);
  printf("sy: %f, oy: %f\n", CurGame->mouse_scale_y, CurGame->mouse_offs_y);
  // WARNING mouse_scale_x and mouse_offs_x UNTESTED
  scale_t mouse_scale_x = CurGame->mouse_scale_x ? CurGame->mouse_scale_x : 1.0;
  scale_t mouse_scale_y = CurGame->mouse_scale_y ? CurGame->mouse_scale_y : 1.0;
  int xx = x / view.xscale / mouse_scale_x - CurGame->mouse_offs_x;
  // WARNING mouse_scale_y untested
  int yy = y / view.yscale / mouse_scale_y + CurGame->mouse_offs_y;

  const uint32 bg_color = view.canvas->MakeColor(view.red, view.green, view.blue);

  MDFN_DrawFillRect(view.canvas, xx, yy, view.width, view.height, bg_color);
  *this << static_cast<cmd_t>(Command::paint) << xx << yy << view.width << view.height << bg_color;
  Send(Command::paint);
}

void Graffiti::Line(int& x0, int& y0,const int& x1,const int& y1)
{
  int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
  int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1;
  int err = (dx>dy ? dx : -dy)/2, e2;

  for(;;){
    Paint(x0,y0);
    if (x0==x1 && y0==y1) break;
    e2 = err;
    if (e2 >-dx) { err -= dy; x0 += sx; }
    if (e2 < dy) { err += dx; y0 += sy; }
  }

  x0 = x1;
  y0 = y1;
}

void Graffiti::Send(Command command)
{ // assumes {Super,}magic is properly orchestrated and no other content has been pushed to str
  switch(command)
  {
  case Command::clear:
    *this << static_cast<cmd_t>(Command::clear);
    break;
  case Command::paint:
    break;
  case Command::sync:
    // TODO
    break;
  }
  TextCommand::Send();
}

bool Graffiti::Process(const char *nick, const char *msg, uint32 len, bool &display)
{
  LoadPacket(msg, sizeof(cmd_t));
  cmd_t cmd;
  *this >> cmd;

  msg += sizeof(cmd_t);

  switch(cmd)
  {
  case Command::paint:
    {
      LoadPacket(&msg[0], len);
      uint32 x,y,w,h,bg_color;
      *this >> x >> y >> w >> h >> bg_color;
      std::cout << "x: " << x << "y: " << y << "w: " << w << "h: " << h << "bg: " << bg_color << std::endl;

      if (strcasecmp(nick, OurNick))
        MDFN_DrawFillRect(view.canvas, x, y, w, h, bg_color);
    }
    break;
  case Command::sync:
    {
      // TODO: download / decompress / load surface data
      if (!strcasecmp(nick, OurNick))
        break;
      fprintf (stderr, "SYNC RECEIVED\n");
      for (int i=0; i < 24; i++)
        printf("0x%02x ", static_cast<unsigned char>(msg[i]));

      RecvSync(msg, len);
    }
    break;
  case Command::clear:
    if (!strcasecmp(nick, OurNick))
      break;
    view.Clear();
    break;
  }

  display = false;
  return true;
}

void Graffiti::RecvSync(const char *msg, uint32 len)
{
  uLongf dlen = MDFN_de32lsb(&msg[0]);
  printf ("dlen = %d\n", dlen);
  if(len > view.canvas->Size()) // Uncompressed length sanity check - 1 MiB max.
  {
    throw MDFN_Error(0, _("Uncompressed save state data is too large: %llu"), (unsigned long long)len);
  }

  uncompress((Bytef *)view.canvas->pixels, &dlen, (Bytef *)&msg[4], len - 4);
}