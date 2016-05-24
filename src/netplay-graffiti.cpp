#include "netplay-graffiti.h"

static bool CC_graffiti(const char *arg)
{
  extern Graffiti *graffiti;

  if (!strcmp("", arg))
    graffiti->Toggle();

  if (!strcmp("on", arg))
  {
    graffiti->Enable();
  }
  else if (!strcmp("off", arg))
  {
    graffiti->Disable();
  }
  else if (!strcmp("clear", arg))
  {
    graffiti->Clear();
  }

  return true;  // keep console open
}

// TODO: Write proper help stanza
const CommandEntry GraffitiCommand {
  "/g", CC_graffiti,
  gettext_noop(""),
  "Graffiti"
};

Graffiti::Graffiti(MDFN_Surface *newcanvas) :view(newcanvas)
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
///////////////
void Graffiti::Enable(bool e)
{
  SDL_ShowCursor(e); enabled = e;
}

void Graffiti::Disable()
{
  SDL_ShowCursor(0); enabled = false;
}

void Graffiti::Toggle()
{
  Enable(!enabled);
}
////////////////
void Graffiti::Clear()
{
  if (view.canvas)
  {
    view.canvas->Fill(0,0,0,0);
    // TODO: Tell your netplay friend(s) to do the same!
  }
}

void Graffiti::SetScale(const scale_t& x, const scale_t& y)
{
  view.xscale = x;
  view.yscale = y;
}

bool Graffiti::Broadcast()
{
  if (will_broadcast)
  {
    will_broadcast = false;

    // compress and send view.surface
    return true;
  }

  return false;
}

void Graffiti::Blit(MDFN_Surface *target)
{
  if(!active)
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

  LoadPacket(msg, len);

  uint32 x,y,w,h,bg_color;
  *this >> x >> y >> w >> h >> bg_color;
  std::cout << "x: " << x << "y: " << y << "w: " << w << "h: " << h << "bg: " << bg_color << std::endl;

  if (strcasecmp(nick, OurNick))
    MDFN_DrawFillRect(view.canvas, x, y, w, h, bg_color);

  display = false;

  return false;
}

/*
paint: Utility function that paints colors to a canvas.
The location to paint is given by x and y, the color to paint is
a mixture of red, green, and blue values in the range 0 to 255.
*/
void Graffiti::Paint(const int& x, const int& y)
{
  if(!active)
    return;

  printf("x: %d, y: %d\n", x, y);
  printf("sx: %f, ox: %f\n", CurGame->mouse_scale_x, CurGame->mouse_offs_x);
  printf("sy: %f, oy: %f\n", CurGame->mouse_scale_y, CurGame->mouse_offs_y);
  int xx = x / view.xscale; //CurGame->mouse_scale_x - CurGame->mouse_offs_x;
  int yy = y / view.yscale; //CurGame->mouse_scale_y - CurGame->mouse_offs_y;

  const uint32 bg_color = view.canvas->MakeColor(view.red, view.green, view.blue);
  
  // printf("x = %d, y = %d\n", x, y);
  MDFN_DrawFillRect(view.canvas, xx, yy, view.width, view.height, bg_color);
  *this << xx << yy << view.width << view.height << bg_color;
  Send();
}