#pragma once

struct NoteModel {
  int lane{0};
  int start{0};
  int end{0};
  int velocity{0};
  float scaledWidth{1.0f};
  float scaledHeight{1.0f};
};
