/* netplay-text.h : an abstraction over the MDFNNPCMD_TEXT comm. channel

Mednafen Netplay's MDFNNPCMD_TEXT was seen by me as a convenient pre-existing
comm. channel that could be simultaneously re-purposed. This set of classes
facilitates adding netplay client features without requiring server
modification.

The TextCommand class should be used as a component, or as a base class of your
netplay feature. When you instantiate it, provide a unique "magic number" that
no other TextCommand uses [TODO: there should be a check for duplicates anyways],
and specify a payload limit (in bytes).

*/
#ifndef _MDFN_NETPLAY_TEXT_H
#define _MDFN_NETPLAY_TEXT_H

#include "mednafen.h"
#include "netplay.h"
#include "netplay-driver.h"
#include <zlib.h>

class TextCommand
{
public:
  using magic_t = uint16;
  using limit_t = uint32;

  static const limit_t NormalPayloadLimit; // chat messages
  
  struct Registration {
  public:
    int Register(TextCommand *tc);

    static constexpr magic_t SuperMagic {0x0101};
    /* Call registered Text handlers
    ** They can modify whether command should be printed by setting display to false
    */
    void CreateBuffer();
    void DestroyBuffer();

    void EnableOnStart();
    void DisableCommands();

    TextCommand* FindByMagic(magic_t magic);
    bool Process(const char *nick, const char *msg, uint32 len, bool &display);

    // when netplay begins, this will be max(all-registered-commands' payload_limit)
    std::vector<char> network_buffer;
    limit_t MaxPayloadLimit();
  private:
    std::vector<TextCommand *> commands;
    bool SuperMagicValid(const char* msg) const;
    //limit_t FindMaxPayloadLimit();
  };

  static Registration Registrar;

  TextCommand(const std::string title, const magic_t m, const limit_t l);

  magic_t Magic() const;

  virtual void Enable(bool e=true);
  virtual void Disable();
  virtual void ToggleEnable();
  virtual bool Enabled() const;

  template<class T>
  TextCommand& operator<< (T i)
  {
    char buf[sizeof(T)];
    MDFN_enlsb<T, false>(buf, i);

    omsg += std::string(reinterpret_cast<const char *>(buf), sizeof(i));
    return *this;
  }

  template<class T>
  TextCommand& operator>> (T &i)
  {
    i = MDFN_delsb<T, false>(&imsg[0]);
    imsg.erase(0, sizeof(i));
    return *this;
  }

  void Send(const std::string& message = "");
  virtual bool Process(const char *nick, const char *msg, uint32 len, bool &display)=0;
  
  bool MagicValid(const char* msg) const;

  void LoadPacket(const char* str, uint32 len);

  bool EnableOnStart();

  limit_t PayloadLimit();

  std::string title;
protected:
  std::string omsg;
  std::string imsg;
  bool enabled {false};
  bool enable_on_start {false};
  static std::string magic2str(magic_t m);
  const magic_t magic;
  const limit_t payload_limit;
};

#endif
