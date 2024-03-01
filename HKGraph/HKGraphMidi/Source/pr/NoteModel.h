#pragma once

struct NoteModel {
  int lane{0};
  int start{0};
  int end{0};
  float velocity{0.5f};
  float scaledWidth{1.0f};
  float scaledHeight{1.0f};
};
