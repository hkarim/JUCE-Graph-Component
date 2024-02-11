#pragma once
#include "Graph.h"
#include "Processors.h"
#include "PassthroughProcessor.h"
#include "ChannelRouterProcessor.h"
#include "ChannelSplitterNodeProcessor.h"
#include "KeyboardProcessor.h"
#include "MidiInNodeProcessor.h"
#include "MidiOutNodeProcessor.h"
#include "TransposeProcessor.h"

struct ProcessorRegistry {

  static NodeProcessor *makeNodeProcessor(Graph *graph, const std::string& typeId) {
    NodeProcessor *nodeProcessor = nullptr;
    if (typeId == Processors::midiInNodeProcessor)
      nodeProcessor = new MidiInNodeProcessor(graph);
    else if (typeId ==  Processors::midiOutNodeProcessor)
      nodeProcessor = new MidiOutNodeProcessor(graph);
    else if (typeId ==  Processors::passthroughProcessor)
      nodeProcessor = new PassthroughProcessor(graph);
    else if (typeId ==  Processors::channelRouterProcessor)
      nodeProcessor = new ChannelRouterProcessor(graph);
    else if (typeId ==  Processors::channelSplitterNodeProcessor)
      nodeProcessor = new ChannelSplitterNodeProcessor(graph);
    else if (typeId ==  Processors::keyboardProcessor)
      nodeProcessor = new KeyboardProcessor(graph);
    else if (typeId == Processors::transposeProcessor)
      nodeProcessor = new TransposeProcessor(graph);

    return nodeProcessor;
  }
};
