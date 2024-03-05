#pragma once

struct PianoRollTheme {
  static constexpr unsigned int whiteKeysBg = 0xff2a2d31;
  static constexpr unsigned int blackKeysBg = 0xff232528;

  static constexpr unsigned int hWhiteLanesSeparatorFg = 0xff1f2123;
  static constexpr unsigned int hOctaveLanesSeparatorFg = 0xff3a3d42;
  static constexpr unsigned int vBarFg = 0xff44484C;
  static constexpr unsigned int vBeatFg = 0xff3A3D42;
  static constexpr unsigned int vQuantizeFg = 0xff313439;

  static constexpr int hLaneSeparatorHeight = 1;
  static constexpr int vBarSeparatorWidth = 1;

  static constexpr unsigned int noteSelectedBg = 0xffe8d5c9;
  static constexpr unsigned int noteUnselectedBg = 0xffb8744a;
  static constexpr float noteBorderWidth = 0.2f;

};
