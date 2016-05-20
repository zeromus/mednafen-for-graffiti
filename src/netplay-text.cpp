struct TextCommand
{
  using id_t = uint16_t;

  TextCommand() {
    id = ++ID;
  }

  void activate() { active = true; }
  void deactivate() { active = false; };
  bool is_active() { return active; }

  void send() {
    MDFNI_NetplayText(std::string(message));
  }
  virtual bool Process(const char *nick, const char *msg, bool &display)=0;
  id_t id;
  bool active=true;
  std::string message;

  static id_t ID;
};

TextCommand::id_t TextCommand::ID = 0;

struct DrawCommand : public TextCommand
{
  bool Process(const char *nick, const char *msg, bool &display)
  {
    printf("IN DRAW\n");

    // check for command sequence
    if (*(id_t *)msg == id)
    {
      printf ("CMD MATCH!\n");
      display = false;
    }

    return false;
  }
};

struct TextCommandRegistrar
{
  //int Register(const TextCommand &tc);

  std::vector<TextCommand *> commands = {
    new DrawCommand
  };

  /* Call registered Text handlers
  ** They can modify whether command should be printed by setting display to false
  */
  void Process(const char *nick, const char *msg, bool &display)
  {
    // iterate through plugins until a match has been found (returns true)
    for (auto& cmd : commands)
    {
      if (cmd->Process(nick, msg, display))
        return;
    }
  }
};
