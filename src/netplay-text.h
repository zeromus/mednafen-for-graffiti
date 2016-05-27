#pragma once

#include "mednafen.h"
#include "netplay.h"
#include "netplay-driver.h"
#include <zlib.h>

class TextCommand
{
public:
  using magic_t = uint16;
  
  struct Registration {
  public:
    int Register(TextCommand *tc);

    static const magic_t SuperMagic {0x0101};
    /* Call registered Text handlers
    ** They can modify whether command should be printed by setting display to false
    */
    void Process(const char *nick, const char *msg, uint32 len, bool &display);
  private:
    std::vector<TextCommand *> commands;
    bool SuperMagicValid(const char* msg);
  };

  static Registration Registrar;

  TextCommand(magic_t m);

  magic_t Magic();

  void Enable(bool e=true);
  void Disable();
  void ToggleEnable();
  bool Enabled();

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
  
  bool MagicValid(const char* msg);

  void LoadPacket(const char* str, uint32 len);

protected:
  std::string omsg;
  std::string imsg;
  bool enabled {false};
  static std::string magic2str(magic_t m);
  magic_t magic;
private:
};
