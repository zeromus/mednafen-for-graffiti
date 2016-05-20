#include "netplay-graffiti.h"

Graffiti::Graffiti(MDFN_Surface *newcanvas) : canvas{newcanvas}
{
  // fprintf(stderr, "GRAFFITI\n");
  // uint32 pitch32 = CurGame->fb_width; 
  // MDFN_PixelFormat nf(MDFN_COLORSPACE_RGB, 0, 8, 16, 24);
  // canvas = new MDFN_Surface(NULL, CurGame->fb_width, CurGame->fb_height, pitch32, nf);
}

void Graffiti::Draw(MDFN_Surface *target, const int xpos, const int ypos)
{
  if(!active)
    return;

  const uint32 bg_color = target->MakeColor(0, 0, 0);
  const uint32 text_color = target->MakeColor(red, green, blue);
  //char virtfps[32], drawnfps[32], blitfps[32];

  //CalcFramerates(virtfps, drawnfps, blitfps, 32);

  printf("x = %d, y = %d\n", x, y);
  MDFN_DrawFillRect(target, x, y, 5, 5, bg_color);

  // DrawTextTrans(target->pixels + xpos + ypos * target->pitch32, target->pitch32 << 2, box_width, virtfps, text_color, FALSE, TRUE);
  // DrawTextTrans(target->pixels + xpos + (ypos + 7) * target->pitch32, target->pitch32 << 2, box_width, drawnfps, text_color, FALSE, TRUE);
  // DrawTextTrans(target->pixels + xpos + (ypos + 7 * 2) * target->pitch32, target->pitch32 << 2, box_width, blitfps, text_color, FALSE, TRUE);

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
      // red = (rand() % 7 + 1) * 32;
      // green = (rand() % 7 + 1) * 32;
      // blue = (rand() % 7 + 1) * 32;
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

bool Graffiti::Process(const char *nick, const char *msg, bool &display)
{
  // printf("0x%04X\n", magic);
  printf("IN DRAW\n");
  // for (int i=0; msg[i]; i++)
  //   printf("0x%2x ", static_cast<unsigned char>(msg[i]));
  printf("%s\n", msg);

  display = false;

  return false;
}

/*
paint: Utility function that paints colors to a surface.
The location to paint is given by x and y, the color to paint is
a mixture of red, green, and blue values in the range 0 to 255.
*/
void Graffiti::Paint(int x, int y, uint8_t red, uint8_t green, uint8_t blue)
{
  
}