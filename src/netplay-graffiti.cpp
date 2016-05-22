#include "netplay-graffiti.h"

Graffiti::Graffiti(MDFN_Surface *newcanvas) : canvas{newcanvas}
{
  canvas->Fill(0, 0, 0, 0);
  // fprintf(stderr, "GRAFFITI\n");
  // uint32 pitch32 = CurGame->fb_width; 
  // MDFN_PixelFormat nf(MDFN_COLORSPACE_RGB, 0, 8, 16, 24);
  // canvas = new MDFN_Surface(NULL, CurGame->fb_width, CurGame->fb_height, pitch32, nf);
}

void Graffiti::Draw(MDFN_Surface *target, const int xpos, const int ypos)
{
  if(!active)
    return;

  const uint32 bg_color = target->MakeColor(red, green, blue);
  const uint32 text_color = target->MakeColor(red, green, blue);

  // iterate through the canvas
  // any pixel sets that aren't 00, copy over to target
  uint32 *canvas_pixels = (uint32*)canvas->pixels;
  uint32 *target_pixels = (uint32*)target->pixels;

  for(int32 i = 0; i < target->pitchinpix * target->h; i++)
    if(canvas_pixels[i])
      target_pixels[i] = canvas_pixels[i];

 // if(w < 1 || h < 1)
 //  return;

 // Replace these width and height checks with assert()s in the future.
 // if(((uint64)x + w) > (uint32)canvas->w)
 // {
 //  fprintf(stderr, "Rect xw bug!\n");
 //  return;
 // }

 // if(((uint64)y + h) > (uint32)canvas->h)
 // {
 //  fprintf(stderr, "Rect yh bug!\n");
 //  return;
 // }
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
      red = (rand() % 7 + 1) * 32;
      green = (rand() % 7 + 1) * 32;
      blue = (rand() % 7 + 1) * 32;
      x = event.button.x; y = event.button.y;
      Paint(event.button.x, event.button.y, red, green, blue);
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
      x = event.motion.x; y = event.motion.y;
      Paint(event.motion.x, event.motion.y, red, green, blue);
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
  // printf("%s\n", msg);
  LoadPacket(msg, len);

  uint32 x,y,w,h,bg_color;
  *this >> x >> y >> w >> h >> bg_color;
  std::cout << "x: " << x << "y: " << y << "w: " << w << "h: " << h << "bg: " << bg_color << std::endl;

  if (strcasecmp(nick, OurNick))
    MDFN_DrawFillRect(canvas, x, y, w, h, bg_color);

  display = false;

  return false;
}

/*
paint: Utility function that paints colors to a canvas.
The location to paint is given by x and y, the color to paint is
a mixture of red, green, and blue values in the range 0 to 255.
*/
void Graffiti::Paint(int x, int y, uint8_t red, uint8_t green, uint8_t blue)
{
  if(!active)
    return;

  printf("x: %d, y: %d\n", x, y);
  printf("sx: %f, ox: %f\n", CurGame->mouse_scale_x, CurGame->mouse_offs_x);
  printf("sy: %f, oy: %f\n", CurGame->mouse_scale_y, CurGame->mouse_offs_y);
  x = x / xscale; //CurGame->mouse_scale_x - CurGame->mouse_offs_x;
  y = y / yscale; //CurGame->mouse_scale_y - CurGame->mouse_offs_y;

  const uint32 bg_color = canvas->MakeColor(red, green, blue);
  
  // printf("x = %d, y = %d\n", x, y);
  MDFN_DrawFillRect(canvas, x, y, 5, 5, bg_color);
  *this << x << y << 5 << 5 << bg_color;
  Send();
}