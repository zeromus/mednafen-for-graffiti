/^# Packages using this file: / {
  s/# Packages using this file://
  ta
  :a
  s/ mednafen / mednafen /
  tb
  s/ $/ mednafen /
  :b
  s/^/# Packages using this file:/
}
