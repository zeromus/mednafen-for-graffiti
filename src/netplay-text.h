#pragma once

#include <limits>
#include <iostream>

class TextCommand
{
public:
  using magic_t = uint16_t;
  
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

  TextCommand();

  magic_t Magic();

  void Activate();
  void Deactivate();
  bool Active();

  template<class T, typename std::enable_if<std::is_integral<T>::value && sizeof(T)==1, int>::type = 0>
  TextCommand& operator<< (T i)
  {
    //std::cout << "8bit: 0x" << std::hex << i << std::endl;
    msg += std::string(reinterpret_cast<const char *>(&i), sizeof(i));
    return *this;
  }

  template<class T, typename std::enable_if<std::is_integral<T>::value && sizeof(T)==2, int>::type = 0>
  TextCommand& operator<< (T i)
  {
    //std::cout << "16bit: 0x" << std::hex << i << std::endl;
    char buf[sizeof(T)];
    MDFN_enlsb<T, false>(buf, i);

    msg += std::string(reinterpret_cast<const char *>(buf), sizeof(i));
    return *this;
  }

  template<class T, typename std::enable_if<std::is_integral<T>::value && sizeof(T)==4, int>::type = 0>
  TextCommand& operator<< (T i)
  {
    //std::cout << "32bit: 0x" << std::hex << i << std::endl;
    char buf[sizeof(T)];
    MDFN_enlsb<T, false>(buf, i);

    msg += std::string(reinterpret_cast<const char *>(buf), sizeof(i));
    return *this;
  }
////////////
  template<class T, typename std::enable_if<std::is_integral<T>::value && sizeof(T)==1, int>::type = 0>
  TextCommand& operator>> (T &i)
  {
    //std::cout << "8bit: 0x" << std::hex << i << std::endl;
    i = reinterpret_cast<T>(imsg[0]);
    imsg.erase(0, sizeof(i));
    return *this;
  }

  template<class T, typename std::enable_if<std::is_integral<T>::value && sizeof(T)==2, int>::type = 0>
  TextCommand& operator>> (T &i)
  {
    //std::cout << "16bit: 0x" << std::hex << i << std::endl;
    i = MDFN_delsb<T, false>(&imsg[0]);

    imsg.erase(0, sizeof(i));
    return *this;
  }

  template<class T, typename std::enable_if<std::is_integral<T>::value && sizeof(T)==4, int>::type = 0>
  TextCommand& operator>> (T &i)
  {
    //std::cout << "32bit: 0x" << std::hex << i << std::endl;

    i = MDFN_delsb<T, false>(&imsg[0]);
    imsg.erase(0, sizeof(i));
    return *this;
  }

  void Send(const std::string& message = "");
  virtual bool Process(const char *nick, const char *msg, uint32 len, bool &display)=0;
  
  bool MagicValid(const char* msg);

  void LoadPacket(const char* str, uint32 len);

protected:
  std::string msg = "";
  std::string imsg;
  magic_t magic=0xdead;
  bool active=true;
  static std::string magic2str(magic_t m);
private:
};
