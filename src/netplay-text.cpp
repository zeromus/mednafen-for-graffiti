static bool Draw(const char *nick, const char *msg, bool &display);
static bool Draw2(const char *nick, const char *msg, bool &display);
bool (*FunctionPointers[])(const char *nick, const char *msg, bool &display) = {
  Draw,
  Draw2,
  NULL
};
/* Call registered Text handlers
** They can modify whether command should be printed by setting display to false
*/
static void ProcessText(const char *nick, const char *msg, bool &display)
{
  // iterate through plugins until a match has been found (returns true)
  for (int i=0; FunctionPointers[i] != NULL; i++)
    if (FunctionPointers[i](nick, msg, display))
      return;
}

static bool Draw(const char *nick, const char *msg, bool &display)
{
  printf("IN DRAW\n");

  // check for command sequence
  const uint16_t CmdSeq = 0x1dea;
  if (*(uint16_t *)msg == CmdSeq)
  {
    printf ("CMD MATCH!\n");
    display = false;
  }
  
  return false;
}

static bool Draw2(const char *nick, const char *msg, bool &display)
{
  printf("IN DRAW2\n");
  return true;
}