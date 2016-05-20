#include "netplay-graffiti.h"

void Graffiti::Send(const std::string &msg)
{
  draw_command.Send(msg);
}
bool Graffiti::DrawCommand::Process(const char *nick, const char *msg, bool &display)
{
  // printf("0x%04X\n", magic);
  printf("IN DRAW\n");
  // for (int i=0; msg[i]; i++)
  //   printf("0x%2x ", static_cast<unsigned char>(msg[i]));
  printf("%s\n", msg);

  //display = false;

  return false;
}