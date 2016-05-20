#pragma once

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
    void Process(const char *nick, const char *msg, bool &display);
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

  void Send(const std::string& message);
  virtual bool Process(const char *nick, const char *msg, bool &display)=0;
  
  bool MagicValid(const char* msg);

protected:
  magic_t magic=0xdead;
private:
  bool active=true;

  static std::string magic2str(magic_t m);
};
