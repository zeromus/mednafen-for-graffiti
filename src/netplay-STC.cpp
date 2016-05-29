#include "netplay-STC.h"

const MDFNNP_STC::limit_t MDFNNP_STC::NormalPayloadLimit = 2000;

MDFNNP_STC::MDFNNP_STC(
  const std::string title, const magic_t m, const limit_t l) :
  title{title}, magic{m}, payload_limit{l}
{
  //MDFN_printf("magic => 0x%X\n", magic);
  MDFNNP_STC::Registrar.Register(this);
  *this << Registration::SuperMagic << magic;
}

MDFNNP_STC::magic_t MDFNNP_STC::Magic() const { return magic; }

void MDFNNP_STC::Enable(bool e)
{
  enabled = e;
}

void MDFNNP_STC::Disable()
{
  enabled = false;
}

void MDFNNP_STC::ToggleEnable()
{
  Enable(!enabled);
}

bool MDFNNP_STC::Enabled() const { return enabled; }

void MDFNNP_STC::Send(const std::string& message)
{
  omsg += message;
  MDFNI_NetplayRaw(omsg.c_str(), omsg.size());
  omsg.clear();
  *this << Registration::SuperMagic << magic;
}

static inline bool magic_valid(const char *msg, MDFNNP_STC::magic_t magic)
{
  //MDFN_printf("magic == %04x\n", magic);
  return *reinterpret_cast<const MDFNNP_STC::magic_t *>(msg) == magic;
}

bool MDFNNP_STC::Registration::SuperMagicValid(const char *msg) const
{
  return magic_valid(msg, SuperMagic);
}

bool MDFNNP_STC::MagicValid(const char* msg) const
{
  return magic_valid(msg, magic);
}

void MDFNNP_STC::LoadPacket(const char* str, uint32 len)
{
  imsg = std::string(str, len);
}

std::string MDFNNP_STC::magic2str(magic_t magic)
{
  std::string s;
  const char *m = reinterpret_cast<const char *>(&magic);
  s += *(m++);
  s += *m;
  return s;
}

bool MDFNNP_STC::EnableOnStart()
{
  return enable_on_start;
}

MDFNNP_STC::limit_t MDFNNP_STC::PayloadLimit()
{
  return payload_limit;
}

//////////////////////////////////////////////////////////////////////////////

int MDFNNP_STC::Registration::Register(MDFNNP_STC* tc)
{
  // assert all magic are unique
  //MDFN_printf("magic => 0x%X\n", tc->Magic());
  if (FindByMagic(tc->Magic()))
    throw MDFN_Error(0, _("MDFNNP_STC with duplicate magic: 0x%x"), tc->Magic());
  commands.push_back(tc);
  return 0;
}

MDFNNP_STC::limit_t MDFNNP_STC::Registration::MaxPayloadLimit()
{
  auto c = network_buffer.capacity();
  // CONSIDER : throw exception if capacity is 0
  return c;
}

void MDFNNP_STC::Registration::CreateBuffer()
{
  for (const auto& cmd : commands)
    network_buffer.reserve(cmd->PayloadLimit());

  network_buffer.reserve(MDFNNP_STC::NormalPayloadLimit);
}

void MDFNNP_STC::Registration::DestroyBuffer()
{
  network_buffer.clear();
  network_buffer.shrink_to_fit();
}

MDFNNP_STC* MDFNNP_STC::Registration::FindByMagic(magic_t magic)
{
  for (const auto& cmd : commands)
  {
    if (cmd->Magic() == magic)
      return cmd;
  }

  return nullptr;
}

bool MDFNNP_STC::Registration::Process(
  const char *nick, const char *msg, uint32 len, bool &display)
{
  // for (int i=0; i < len; i++)
  //   MDFN_printf("0x%02x ", static_cast<unsigned char>(msg[i]));

  if (!SuperMagicValid(msg))
    return false;
  // TODO: a MAP<cmd_t, MDFNNP_STC*> would be better!?
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
        0, _("MDFNNP_STC length is too long for \"%s\": limit: %u, actual: %u"),
        cmd->title.c_str(), cmd->PayloadLimit(), sublen);
    if (cmd->Process(nick, &msg[sizeof(magic_t)*2], len, display))
      return true;
  }

  return false;
}

void MDFNNP_STC::Registration::EnableOnStart()
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

void MDFNNP_STC::Registration::DisableCommands()
{
  for (const auto& cmd : commands)
  {
    MDFN_printf("Disable: %s\n", cmd->title.c_str());
    cmd->Disable();
  }
}


MDFNNP_STC::Registration MDFNNP_STC::Registrar {};
