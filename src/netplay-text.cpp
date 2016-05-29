#include "netplay-text.h"

const TextCommand::limit_t TextCommand::NormalPayloadLimit = 2000;

TextCommand::TextCommand(
  const std::string title, const magic_t m, const limit_t l) :
  title{title}, magic{m}, payload_limit{l}
{
  //MDFN_printf("magic => 0x%X\n", magic);
  TextCommand::Registrar.Register(this);
  *this << Registration::SuperMagic << magic;
}

TextCommand::magic_t TextCommand::Magic() const { return magic; }

void TextCommand::Enable(bool e)
{
  enabled = e;
}

void TextCommand::Disable()
{
  enabled = false;
}

void TextCommand::ToggleEnable()
{
  Enable(!enabled);
}

bool TextCommand::Enabled() const { return enabled; }

void TextCommand::Send(const std::string& message)
{
  omsg += message;
  MDFNI_NetplayRaw(omsg.c_str(), omsg.size());
  omsg.clear();
  *this << Registration::SuperMagic << magic;
}

static inline bool magic_valid(const char *msg, TextCommand::magic_t magic)
{
  //MDFN_printf("magic == %04x\n", magic);
  return *reinterpret_cast<const TextCommand::magic_t *>(msg) == magic;
}

bool TextCommand::Registration::SuperMagicValid(const char *msg) const
{
  return magic_valid(msg, SuperMagic);
}

bool TextCommand::MagicValid(const char* msg) const
{
  return magic_valid(msg, magic);
}

void TextCommand::LoadPacket(const char* str, uint32 len)
{
  imsg = std::string(str, len);
}

std::string TextCommand::magic2str(magic_t magic)
{
  std::string s;
  const char *m = reinterpret_cast<const char *>(&magic);
  s += *(m++);
  s += *m;
  return s;
}

bool TextCommand::EnableOnStart()
{
  return enable_on_start;
}

TextCommand::limit_t TextCommand::PayloadLimit()
{
  return payload_limit;
}

//////////////////////////////////////////////////////////////////////////////

int TextCommand::Registration::Register(TextCommand* tc)
{
  // assert all magic are unique
  //MDFN_printf("magic => 0x%X\n", tc->Magic());
  if (FindByMagic(tc->Magic()))
    throw MDFN_Error(0, _("TextCommand with duplicate magic: 0x%x"), tc->Magic());
  commands.push_back(tc);
  return 0;
}

TextCommand::limit_t TextCommand::Registration::MaxPayloadLimit()
{
  auto c = network_buffer.capacity();
  // CONSIDER : throw exception if capacity is 0
  return c;
}

void TextCommand::Registration::CreateBuffer()
{
  for (const auto& cmd : commands)
    network_buffer.reserve(cmd->PayloadLimit());

  network_buffer.reserve(TextCommand::NormalPayloadLimit);
}

void TextCommand::Registration::DestroyBuffer()
{
  network_buffer.clear();
  network_buffer.shrink_to_fit();
}

TextCommand* TextCommand::Registration::FindByMagic(magic_t magic)
{
  for (const auto& cmd : commands)
  {
    if (cmd->Magic() == magic)
      return cmd;
  }

  return nullptr;
}

bool TextCommand::Registration::Process(
  const char *nick, const char *msg, uint32 len, bool &display)
{
  // for (int i=0; i < len; i++)
  //   MDFN_printf("0x%02x ", static_cast<unsigned char>(msg[i]));

  if (!SuperMagicValid(msg))
    return false;
  // TODO: a MAP<cmd_t, TextCommand*> would be better!?
  // iterate through plugins until a match has been found (returns true)
  auto sublen = len - (2 * sizeof(magic_t));
  for (auto& cmd : commands)
  {
    if (!cmd->MagicValid(&msg[sizeof(magic_t)]))
      continue;
    if (!cmd->Enabled())
      continue;
    if(sublen > cmd->PayloadLimit()) // Sanity check
      throw MDFN_Error(
        0, _("TextCommand length is too long for \"%s\": limit: %u, actual: %u"),
        cmd->title.c_str(), cmd->PayloadLimit(), sublen);
    if (cmd->Process(nick, &msg[sizeof(magic_t)*2], len, display))
      return true;
  }

  return false;
}

void TextCommand::Registration::EnableOnStart()
{
  for (const auto& cmd : commands)
  {
    if (cmd->EnableOnStart())
    {
      MDFN_printf("EnableOnStart: %s\n", cmd->title.c_str());
      cmd->Enable();
    }
  }
}

void TextCommand::Registration::DisableCommands()
{
  for (const auto& cmd : commands)
  {
    MDFN_printf("Disable: %s\n", cmd->title.c_str());
    cmd->Disable();
  }
}

// TextCommand::limit_t TextCommand::Registration::FindMaxPayloadLimit()
// {
//   limit_t max = 0;
//   for (const auto& cmd : commands)
//     if (cmd->payload_limit > max)
//       max = cmd->payload_limit;
// }


TextCommand::Registration TextCommand::Registrar {};
