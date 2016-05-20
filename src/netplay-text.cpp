#include "netplay-text.h"

TextCommand::TextCommand()
{
  TextCommand::Registrar.Register(this);
}

TextCommand::magic_t TextCommand::Magic() { return magic; }
void TextCommand::Activate() { active = true; }
void TextCommand::Deactivate() { active = false; };
bool TextCommand::Active() { return active; }
void TextCommand::Send(const std::string& message) {
  std::string msg = magic2str(Registration::SuperMagic) + magic2str(magic) + message;
  MDFNI_NetplayText(msg.c_str());
}

static inline bool magic_valid(const char *msg, TextCommand::magic_t magic)
{
  return *reinterpret_cast<const TextCommand::magic_t *>(msg) == magic;
}

bool TextCommand::Registration::SuperMagicValid(const char *msg)
{
  return magic_valid(msg, SuperMagic);
}

bool TextCommand::MagicValid(const char* msg)
{
  return magic_valid(msg, magic);
}
std::string TextCommand::magic2str(magic_t magic)
{
  std::string s;
  const char *m = reinterpret_cast<const char *>(&magic);
  s += *(m++);
  s += *m;
  return s;
}

int TextCommand::Registration::Register(TextCommand* tc)
{
  commands.push_back(tc);
  // assert all magic are unique
  return 0;
}

void TextCommand::Registration::Process(const char *nick, const char *msg, bool &display)
{
  if (!SuperMagicValid(msg))
    return;
  // iterate through plugins until a match has been found (returns true)
  for (auto& cmd : commands)
  {
    if (!cmd->MagicValid(&msg[sizeof(magic_t)]))
      continue;
    if (cmd->Process(nick, &msg[sizeof(magic_t)*2], display))
      return;
  }
}

TextCommand::Registration TextCommand::Registrar {};
