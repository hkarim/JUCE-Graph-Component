#pragma once

struct PlaybackProcessor : public NodeProcessor {
  double sampleRate{};
  int samplesPerBlock{};

  explicit PlaybackProcessor(Graph *graph) : NodeProcessor(graph) {}

  PlaybackProcessor(Graph *graph, const std::string &name, uint32_t n_ins, uint32_t n_outs)
  : NodeProcessor(graph, name, n_ins, n_outs) {}

  virtual void prepareToPlay(double playbackSampleRate, int playbackSamplesPerBlock) {
    this->sampleRate = playbackSampleRate;
    this->samplesPerBlock = playbackSamplesPerBlock;
  }

  virtual void releaseResources() {
  }

};