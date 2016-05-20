#pragma once
#include "netplay-text.h"

class Graffiti
{
public:
  void Send(const std::string &msg);

private:
  struct DrawCommand : public TextCommand
  {
    bool Process(const char *nick, const char *msg, bool &display);
  } draw_command;
  MDFN_Surface *surface;  // I think that's the right datatype..


};


