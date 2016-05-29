#include "netplay-text.h"

const MDFNNP_SubTextCommand::limit_t MDFNNP_SubTextCommand::NormalPayloadLimit = 2000;

MDFNNP_SubTextCommand::MDFNNP_SubTextCommand(
  const std::string title, const magic_t m, const limit_t l) :
  title{title}, magic{m}, payload_limit{l}
{
  //MDFN_printf("magic => 0x%X\n", magic);
  MDFNNP_SubTextCommand::Registrar.Register(this);
  *this << Registration::SuperMagic << magic;
}

MDFNNP_SubTextCommand::magic_t MDFNNP_SubTextCommand::Magic() const { return magic; }

void MDFNNP_SubTextCommand::Enable(bool e)
{
  enabled = e;
}

void MDFNNP_SubTextCommand::Disable()
{
  enabled = false;
}

void MDFNNP_SubTextCommand::ToggleEnable()
{
  Enable(!enabled);
}

bool MDFNNP_SubTextCommand::Enabled() const { return enabled; }

void MDFNNP_SubTextCommand::Send(const std::string& message)
{
  omsg += message;
  MDFNI_NetplayRaw(omsg.c_str(), omsg.size());
  omsg.clear();
  *this << Registration::SuperMagic << magic;
}

static inline bool magic_valid(const char *msg, MDFNNP_SubTextCommand::magic_t magic)
{
  //MDFN_printf("magic == %04x\n", magic);
  return *reinterpret_cast<const MDFNNP_SubTextCommand::magic_t *>(msg) == magic;
}

bool MDFNNP_SubTextCommand::Registration::SuperMagicValid(const char *msg) const
{
  return magic_valid(msg, SuperMagic);
}

bool MDFNNP_SubTextCommand::MagicValid(const char* msg) const
{
  return magic_valid(msg, magic);
}

void MDFNNP_SubTextCommand::LoadPacket(const char* str, uint32 len)
{
  imsg = std::string(str, len);
}

std::string MDFNNP_SubTextCommand::magic2str(magic_t magic)
{
  std::string s;
  const char *m = reinterpret_cast<const char *>(&magic);
  s += *(m++);
  s += *m;
  return s;
}

bool MDFNNP_SubTextCommand::EnableOnStart()
{
  return enable_on_start;
}

MDFNNP_SubTextCommand::limit_t MDFNNP_SubTextCommand::PayloadLimit()
{
  return payload_limit;
}

//////////////////////////////////////////////////////////////////////////////

int MDFNNP_SubTextCommand::Registration::Register(MDFNNP_SubTextCommand* tc)
{
  // assert all magic are unique
  //MDFN_printf("magic => 0x%X\n", tc->Magic());
  if (FindByMagic(tc->Magic()))
    throw MDFN_Error(0, _("MDFNNP_SubTextCommand with duplicate magic: 0x%x"), tc->Magic());
  commands.push_back(tc);
  return 0;
}

MDFNNP_SubTextCommand::limit_t MDFNNP_SubTextCommand::Registration::MaxPayloadLimit()
{
  auto c = network_buffer.capacity();
  // CONSIDER : throw exception if capacity is 0
  return c;
}

void MDFNNP_SubTextCommand::Registration::CreateBuffer()
{
  for (const auto& cmd : commands)
    network_buffer.reserve(cmd->PayloadLimit());

  network_buffer.reserve(MDFNNP_SubTextCommand::NormalPayloadLimit);
}

void MDFNNP_SubTextCommand::Registration::DestroyBuffer()
{
  network_buffer.clear();
  network_buffer.shrink_to_fit();
}

MDFNNP_SubTextCommand* MDFNNP_SubTextCommand::Registration::FindByMagic(magic_t magic)
{
  for (const auto& cmd : commands)
  {
    if (cmd->Magic() == magic)
      return cmd;
  }

  return nullptr;
}

bool MDFNNP_SubTextCommand::Registration::Process(
  const char *nick, const char *msg, uint32 len, bool &display)
{
  // for (int i=0; i < len; i++)
  //   MDFN_printf("0x%02x ", static_cast<unsigned char>(msg[i]));

  if (!SuperMagicValid(msg))
    return false;
  // TODO: a MAP<cmd_t, MDFNNP_SubTextCommand*> would be better!?
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
        0, _("MDFNNP_SubTextCommand length is too long for \"%s\": limit: %u, actual: %u"),
        cmd->title.c_str(), cmd->PayloadLimit(), sublen);
    if (cmd->Process(nick, &msg[sizeof(magic_t)*2], len, display))
      return true;
  }

  return false;
}

void MDFNNP_SubTextCommand::Registration::EnableOnStart()
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

void MDFNNP_SubTextCommand::Registration::DisableCommands()
{
  for (const auto& cmd : commands)
  {
    MDFN_printf("Disable: %s\n", cmd->title.c_str());
    cmd->Disable();
  }
}


MDFNNP_SubTextCommand::Registration MDFNNP_SubTextCommand::Registrar {};
