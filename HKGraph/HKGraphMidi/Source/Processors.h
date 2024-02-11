#pragma once
#include <string>

struct Processors {
  static const std::string midiInNodeProcessor;
  static const std::string midiOutNodeProcessor;
  static const std::string passthroughProcessor;
  static const std::string channelRouterProcessor;
  static const std::string channelSplitterNodeProcessor;
  static const std::string keyboardProcessor;
  static const std::string transposeProcessor;
};
