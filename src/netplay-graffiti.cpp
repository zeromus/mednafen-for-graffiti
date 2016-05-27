#include "netplay-graffiti.h"

bool Graffiti::ConsoleCommandParser(const char *arg)
{
  extern Graffiti *graffiti;

  return graffiti->ParseConsoleCommand(arg);
}

// TODO: Write proper help stanza
const CommandEntry Graffiti::ConsoleCommandEntry {
  "/g", ConsoleCommandParser,
  gettext_noop(""),
  "Graffiti"
};

Graffiti::Graffiti(MDFN_Surface *new_surface) :
  TextCommand(0xf171),
  view{new_surface}
{
}


Graffiti::View::View(MDFN_Surface *new_surface)
{
  if (surface)
  {
    delete surface;
  }

  surface = new_surface;
  surface->Fill(0, 0, 0, 0);
}

Graffiti::View::~View()
{
  if (surface)
  {
    delete surface;
    surface = nullptr;
  }
}

void Graffiti::View::Clear()
{
  if (surface)
  {
    surface->Fill(0,0,0,0);
  }
}
///////////////
bool Graffiti::ParseConsoleCommand(const char *arg)
{
  if (!strcmp("", arg))
    ToggleActivate();
  else if (!strcmp("enable", arg) || !strcmp("en", arg))
  {
    Enable();
    Activate();
  }
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
  ShowCursor(e);
}

void Graffiti::Disable()
{
  active = false;
  TextCommand::Disable();
  ShowCursor(0);
  ClearLocalSurface();
}

void Graffiti::Activate(bool e)
{
  ShowCursor(e);
  active = e;
}

void Graffiti::Deactivate()
{
  ShowCursor(0);
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

  MDFN_printf("BROADCASTING\n");
  // compress and send view.surface
  std::vector<uint8> cbuf;
  uLongf clen;

  fflush(stdout);

  *this << static_cast<cmd_t>(Command::sync);

  // TODO / WARNING -- this relies on all client surfaces using the same BPP
  clen = view.surface->Size() + view.surface->Size() / 1000 + 12;
  cbuf.resize(4 + clen);
  MDFN_en32lsb(&cbuf[0], view.surface->Size());
  compress2((Bytef *)&cbuf[0] + 4, &clen, (Bytef *)view.surface->pixels, view.surface->Size(), 7);

  MDFN_printf("Clen = %d", clen);
  // WARNING INEFFICIENT - just to see if it works first
  // ideally cbuf should be the istr to begin with (resized and at the proper index)
  cbuf.resize(clen + 4);
  for (auto i : cbuf)
    *this << i;

  //MDFN_printf("surface size: %d\n", view.surface->Size());

  Send(Command::sync);
  return true;
}

void Graffiti::Blit(MDFN_Surface *target)
{
  if(!enabled || !active)
    return;

  // iterate through the surface
  // any pixel sets that aren't 00, copy over to target
  uint32 *surface_pixels = (uint32*)view.surface->pixels;
  uint32 *target_pixels = (uint32*)target->pixels;

  for(uint32 i = 0; i < target->pitchinpix * target->h; i++)
    if(surface_pixels[i])
      target_pixels[i] = surface_pixels[i];
}

extern MDFNGI *CurGame;
std::pair<int, int> Graffiti::MouseCoords2SurfaceCoords(const int& x, const int& y)
{
  MDFN_printf("x: %d, y: %d\n", x, y);
  MDFN_printf("sx: %f, ox: %f\n", CurGame->mouse_scale_x, CurGame->mouse_offs_x);
  MDFN_printf("sy: %f, oy: %f\n", CurGame->mouse_scale_y, CurGame->mouse_offs_y);
  // WARNING mouse_scale_x and mouse_offs_x UNTESTED
  scale_t mouse_scale_x = CurGame->mouse_scale_x ? CurGame->mouse_scale_x : 1.0;
  scale_t mouse_scale_y = CurGame->mouse_scale_y ? CurGame->mouse_scale_y : 1.0;
  int xx = (x / view.xscale) * mouse_scale_x + CurGame->mouse_offs_x;
  // WARNING mouse_scale_y untested
  int yy = (y / view.yscale) * mouse_scale_y + CurGame->mouse_offs_y;

  return std::pair<int, int>(xx, yy);
}

void Graffiti::Paint(
  const int& x, const int& y, const uint32& w, const uint32& h,
  const uint32& bg_color, const bool broadcast)
{
  MDFN_DrawFillRect(view.surface, x, y, w, h, bg_color);

  if (broadcast)
  {
    *this << static_cast<cmd_t>(Command::paint) << x << y << w << h << bg_color;
    Send(Command::paint);
  }
}

void Graffiti::Line(
  int& x0, int& y0, const int& x1, const int& y1,
  const uint32& w, const uint32& h, const uint32& bg_color)
{
  int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
  int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1;
  int err = (dx>dy ? dx : -dy)/2, e2;

  for(;;){
    Paint(x0, y0, w, h, bg_color, true);
    if (x0==x1 && y0==y1) break;
    e2 = err;
    if (e2 >-dx) { err -= dy; x0 += sx; }
    if (e2 < dy) { err += dx; y0 += sy; }
  }

  x0 = x1;
  y0 = y1;
}

void Graffiti::Send(Command command)
{ // assumes {Super,}magic is properly "queued" and no other content has been pushed to str
  switch(command)
  {
  case Command::clear:
    *this << static_cast<cmd_t>(Command::clear);
    break;
  case Command::paint:
    break;
  case Command::sync:
    break;
  }
  TextCommand::Send();
}

bool Graffiti::Process(const char *nick, const char *msg, uint32 len, bool& display)
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
      MDFN_printf("x: %d, y: %d, w: %d, h: %d, bg_color: %d\n", x, y, w, h, bg_color);

      if (strcasecmp(nick, OurNick))
        Paint(x, y, w, h, bg_color, false);
    }
    break;
  case Command::sync:
    if (!strcasecmp(nick, OurNick))
      break;
    MDFN_printf("SYNC RECEIVED\n");

    RecvSync(msg, len);
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
  MDFN_printf("dlen = %d\n", dlen);
  if(len > view.surface->Size()) // Uncompressed length sanity check - 1 MiB max.
  {
    throw MDFN_Error(0, _("Uncompressed graffiti state data is too large: %llu"), (unsigned long long)len);
  }

  uncompress((Bytef *)view.surface->pixels, &dlen, (Bytef *)&msg[4], len - 4);
}