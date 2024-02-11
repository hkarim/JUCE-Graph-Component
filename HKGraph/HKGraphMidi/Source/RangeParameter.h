#pragma once

struct IntRangeParameter {
  int min{}, max{}, step{}, value{};
  bool changed{false};
};