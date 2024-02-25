#pragma once
#include "Graph.h"
#include "Processors.h"
#include "PassthroughProcessor.h"
#include "ChannelRouterProcessor.h"
#include "ChannelSplitterProcessor.h"
#include "KeyboardProcessor.h"
#include "NoteFilterProcessor.h"
#include "MidiInNodeProcessor.h"
#include "MidiOutNodeProcessor.h"
#include "TransposeProcessor.h"
#include "CurveProcessor.h"
#include "ChordSplitterProcessor.h"
#include "PianoRollProcessor.h"

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
    else if (typeId ==  Processors::channelSplitterProcessor)
      nodeProcessor = new ChannelSplitterProcessor(graph);
    else if (typeId ==  Processors::chordSplitterProcessor)
      nodeProcessor = new ChordSplitterProcessor(graph);
    else if (typeId ==  Processors::keyboardProcessor)
      nodeProcessor = new KeyboardProcessor(graph);
    else if (typeId ==  Processors::noteFilterProcessor)
      nodeProcessor = new NoteFilterProcessor(graph);
    else if (typeId == Processors::transposeProcessor)
      nodeProcessor = new TransposeProcessor(graph);
    else if (typeId == Processors::velocityCurveProcessor)
      nodeProcessor = new VelocityCurveProcessor(graph);
    else if (typeId == Processors::controllerCurveProcessor)
      nodeProcessor = new ControllerCurveProcessor(graph);
    else if (typeId == Processors::pianoRollProcessor)
      nodeProcessor = new PianoRollProcessor(graph);

    return nodeProcessor;
  }
};
