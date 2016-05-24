#include "netplay-graffiti.h"

static bool CC_graffiti(const char *arg)
{
  extern Graffiti *graffiti;

  return graffiti->ConsoleParse(arg);
}

// TODO: Write proper help stanza
const CommandEntry GraffitiCommand {
  "/g", CC_graffiti,
  gettext_noop(""),
  "Graffiti"
};

Graffiti::Graffiti(MDFN_Surface *newcanvas) :
  TextCommand(0xf171),
  view{newcanvas}
{
}


Graffiti::View::View(MDFN_Surface *newcanvas)
{
  if (canvas)
  {
    delete canvas;
  }

  canvas = newcanvas;
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
bool Graffiti::ConsoleParse(const char *arg)
{
  if (!strcmp("", arg))
    Toggle();
  else if (!strcmp("on", arg))
    Enable();
  else if (!strcmp("off", arg))
    Disable();
  else if (!strcmp("clear", arg))
  {
    ClearLocal();
    ClearRemote();
  }

  return true;  // keep console open
}

void Graffiti::ClearLocal() { view.Clear(); }
void Graffiti::ClearRemote() { Send(Command::clear); }
///////////////
void Graffiti::Enable(bool e)
{
  TextCommand::Enable(e);
  SDL_ShowCursor(e);
}

void Graffiti::Disable()
{
  TextCommand::Disable();
  SDL_ShowCursor(0);
  ClearLocal();
}
////////////////
void Graffiti::SetScale(const scale_t& x, const scale_t& y)
{
  view.xscale = x;
  view.yscale = y;
}

bool Graffiti::Broadcast()
{
  if(!enabled || !will_broadcast)
    return false;

  printf("BROADCASTING\n");
  will_broadcast = false;
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

  // INEFFICIENT - just to see if it works first
  for (auto i : cbuf)
    *this << i;

  printf("canvas size: %d\n", view.canvas->Size());

  Send(Command::sync);
  return true;
}

void Graffiti::Blit(MDFN_Surface *target)
{
  if(!enabled)
    return;

  // iterate through the canvas
  // any pixel sets that aren't 00, copy over to target
  uint32 *canvas_pixels = (uint32*)view.canvas->pixels;
  uint32 *target_pixels = (uint32*)target->pixels;

  for(int32 i = 0; i < target->pitchinpix * target->h; i++)
    if(canvas_pixels[i])
      target_pixels[i] = canvas_pixels[i];
}

void Graffiti::Input_Event(const SDL_Event &event)
{
  if(!enabled)
    return;

  switch(event.type)
  {
  case SDL_MOUSEBUTTONDOWN:
    if(event.button.state == SDL_PRESSED)
    {
      printf ("painting TRUE\n");
      painting = true;
      view.red = (rand() % 8) * 32;
      view.green = (rand() % 8) * 32;
      view.blue = (rand() % 8) * 32;
      Paint(event.button.x, event.button.y);
    }
    break;

  case SDL_MOUSEBUTTONUP:
    if(event.button.state == SDL_RELEASED)
    {
      printf ("painting FALSE\n");
      painting = false;
    }
    break;

  case SDL_MOUSEMOTION:
    if(painting) {
      // Continue painting
      printf ("painting MOTION\n");
      Paint(event.motion.x, event.motion.y);
    }
    break;

    default:break;
  }
}

// void Graffiti::Send(const std::string &msg)
// {
//   draw_command.Send(msg);
// }

bool Graffiti::Process(const char *nick, const char *msg, uint32 len, bool &display)
{
  // printf("0x%04X\n", magic);
  printf("IN DRAW\n");
  // for (int i=0; i < len; i++)
  //   printf("0x%02x ", static_cast<unsigned char>(msg[i]));

  LoadPacket(msg, sizeof(cmd_t));
  cmd_t cmd;
  *this >> cmd;
  switch(cmd)
  {
  case Command::paint:
    {
      LoadPacket(&msg[sizeof(cmd_t)], len - sizeof(cmd_t));
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

      std::thread t1(&Graffiti::RecvSync, this, msg, len);
      t1.detach();
      //RecvSync(msg, len);
    }
    break;
  case Command::clear:
    // Clear surface
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
  uLongf dlen = MDFN_de32lsb(&msg[sizeof(cmd_t)]); 
  printf ("dlen = %d\n", dlen);
  // if(len > 12 * 1024 * 1024) // Uncompressed length sanity check - 12 MiB max.
  // {
  //   throw MDFN_Error(0, _("Uncompressed save state data is too large: %llu"), (unsigned long long)len);
  // }

  //MemoryStream sm(len, -1);

  //MemoryStream sm(dlen, -1);
  uncompress((Bytef *)view.canvas->pixels, &dlen, (Bytef *)&msg[sizeof(cmd_t) + 4], len - 4 - sizeof(cmd_t));
}

/*
paint: Utility function that paints colors to a canvas.
The location to paint is given by x and y, the color to paint is
a mixture of red, green, and blue values in the range 0 to 255.
*/
void Graffiti::Paint(const int& x, const int& y)
{
  printf("x: %d, y: %d\n", x, y);
  printf("sx: %f, ox: %f\n", CurGame->mouse_scale_x, CurGame->mouse_offs_x);
  printf("sy: %f, oy: %f\n", CurGame->mouse_scale_y, CurGame->mouse_offs_y);
  int xx = x / view.xscale; //CurGame->mouse_scale_x - CurGame->mouse_offs_x;
  int yy = y / view.yscale; //CurGame->mouse_scale_y - CurGame->mouse_offs_y;

  const uint32 bg_color = view.canvas->MakeColor(view.red, view.green, view.blue);
  
  // printf("x = %d, y = %d\n", x, y);
  MDFN_DrawFillRect(view.canvas, xx, yy, view.width, view.height, bg_color);
  *this << static_cast<cmd_t>(Command::paint) << xx << yy << view.width << view.height << bg_color;
  Send(Command::paint);
}

void Graffiti::Send(Command command)
{ // assumes {Super}magic is properly orchestrated and no other content has been pushed to str
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