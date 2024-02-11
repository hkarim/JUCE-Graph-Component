#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
  : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
    graph(new Graph()), parameters("parameters-1.0.0") {
  midiIn = new MidiInNodeProcessor(graph, "midi-in", 0, 1);
  midiOut = new MidiOutNodeProcessor(graph, "midi-out", 1, 0);
  graph->add_node(midiIn);
  graph->add_node(midiOut);
  assignMidiInOutDescriptors();
  graph->add_listener(this);
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor() {
  graph->remove_listener(this);
  delete graph;
}

void AudioPluginAudioProcessor::on_graph_event(const Graph::Event &event) {
  if (
    event == Graph::Event::NodeRemoved ||
    event == Graph::Event::NodeMuted ||
    event == Graph::Event::EdgeRemoved) {
    m_dirty = true;
  }
}

const juce::String AudioPluginAudioProcessor::getName() const {
  return "HKGraphMidi";
}

bool AudioPluginAudioProcessor::acceptsMidi() const {
  return true;
}

bool AudioPluginAudioProcessor::producesMidi() const {
  return true;
}

bool AudioPluginAudioProcessor::isMidiEffect() const {
  return false;
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const {
  return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms() {
  return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
  // so this should be at least 1, even if you're not really implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram() {
  return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram(int index) {
  juce::ignoreUnused(index);
}

const juce::String AudioPluginAudioProcessor::getProgramName(int index) {
  juce::ignoreUnused(index);
  return {};
}

void AudioPluginAudioProcessor::changeProgramName(int index, const juce::String &newName) {
  juce::ignoreUnused(index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
  juce::ignoreUnused(sampleRate, samplesPerBlock);
}

void AudioPluginAudioProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const {
#if JucePlugin_IsMidiEffect
  juce::ignoreUnused(layouts);
  return true;
#else
  // This is the place where you check if the layout is supported.
  // In this template code we only support mono or stereo.
  // Some plugin hosts, such as certain GarageBand versions, will only
  // load plugins that support stereo bus layouts.
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

  // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
      return false;
#endif

  return true;
#endif
}

void AudioPluginAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                             juce::MidiBuffer &midiMessages) {
  buffer.clear();
  std::lock_guard<std::mutex> lock(graph->m_mutex);

  if (m_dirty) {
    juce::MidiBuffer output;
    for (auto i = 1; i <= 16; i++) {
      output.addEvent(juce::MidiMessage::allNotesOff(i), 0);
    }
    midiMessages.swapWith(output);
    m_dirty = false;
    midiOut->output.clear();
  } else {
    Data input = std::make_any<juce::MidiBuffer &>(midiMessages);
    midiIn->async_dispatch(graph, std::nullopt, input);
    midiMessages.swapWith(midiOut->output);
    midiOut->output.clear();
  }

}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const {
  return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *AudioPluginAudioProcessor::createEditor() {
  return new AudioPluginAudioProcessorEditor(*this);
}


void AudioPluginAudioProcessor::saveState() {
  parameters.removeAllChildren(nullptr);
  parameters.removeAllProperties(nullptr);

  juce::ValueTree ui{"ui"};
  ui.setProperty("preferences-editor-width", juce::var(preferences.editorWidth), nullptr);
  ui.setProperty("preferences-editor-height", juce::var(preferences.editorHeight), nullptr);
  parameters.appendChild(ui, nullptr);

  juce::ValueTree graphTree{"graph"};
  graphTree.setProperty("async", graph->async, nullptr);

  // nodes
  juce::ValueTree nodesTree{"nodes"};
  for (auto const &[id, node]: graph->m_nodes) {
    if (auto np = dynamic_cast<NodeProcessor *>(node)) {
      juce::ValueTree nodeTree{juce::String(np->typeId())};
      nodeTree.setProperty("id", juce::var(to_string(id)), nullptr);
      nodeTree.setProperty("name", juce::var(node->m_name), nullptr);
      nodeTree.setProperty("sticky", juce::var(node->m_sticky), nullptr);
      nodeTree.setProperty("muted", juce::var(node->is_muted()), nullptr);
      // ins
      juce::ValueTree insTree{"ins"};
      for (auto const &[pinId, pin]: node->m_ins) {
        juce::ValueTree pinTree{juce::String(to_string(pinId))};
        pinTree.setProperty("id", juce::var(to_string(pinId)), nullptr);
        pinTree.setProperty("order", juce::var(static_cast<int>(pin.m_order)), nullptr);
        pinTree.setProperty("owner_node_id", juce::var(to_string(pin.m_owner_node_id)), nullptr);
        insTree.appendChild(pinTree, nullptr);
      }
      nodeTree.appendChild(insTree, nullptr);
      // outs
      juce::ValueTree outsTree{"outs"};
      for (auto const &[pinId, pin]: node->m_outs) {
        juce::ValueTree pinTree{juce::String(to_string(pinId))};
        pinTree.setProperty("id", juce::var(to_string(pinId)), nullptr);
        pinTree.setProperty("order", juce::var(static_cast<int>(pin.m_order)), nullptr);
        pinTree.setProperty("owner_node_id", juce::var(to_string(pin.m_owner_node_id)), nullptr);
        outsTree.appendChild(pinTree, nullptr);
      }
      nodeTree.appendChild(outsTree, nullptr);

      // let the processor save its state
      np->saveState(nodeTree);

      // append the current node tree
      nodesTree.appendChild(nodeTree, nullptr);
    }
  }
  graphTree.appendChild(nodesTree, nullptr);

  // edges
  juce::ValueTree edgesTree{"edges"};
  for (auto const &[id, edge]: graph->m_edges) {
    juce::ValueTree edgeTree{juce::String(to_string(id))};
    edgeTree.setProperty("id", juce::var(to_string(id)), nullptr);
    edgeTree.setProperty("source_node_id", juce::var(to_string(edge.m_source_node_id)), nullptr);
    edgeTree.setProperty("source_pin_id", juce::var(to_string(edge.m_source_pin_id)), nullptr);
    edgeTree.setProperty("target_node_id", juce::var(to_string(edge.m_target_node_id)), nullptr);
    edgeTree.setProperty("target_pin_id", juce::var(to_string(edge.m_target_pin_id)), nullptr);
    // append the current edge tree
    edgesTree.appendChild(edgeTree, nullptr);
  }
  graphTree.appendChild(edgesTree, nullptr);

  // descriptors
  juce::ValueTree descriptorsTree{"descriptors"};
  for (auto const &[id, descriptor]: nodeDescriptors) {
    juce::String did(to_string(id));
    juce::ValueTree descriptorTree{did};
    descriptor->save(descriptorTree);
    // append the current descriptor tree
    descriptorsTree.appendChild(descriptorTree, nullptr);
  }
  graphTree.appendChild(descriptorsTree, nullptr);

  parameters.appendChild(graphTree, nullptr);
}

void AudioPluginAudioProcessor::restoreState() {
  nodeDescriptors.clear();
  graph->m_edges.clear();
  graph->m_pins.clear();
  for (auto &[_, n]: graph->m_nodes) {
    delete n;
    n = nullptr;
  }
  graph->m_nodes.clear();

  auto ui = parameters.getChildWithName("ui");
  preferences.editorWidth = ui.getProperty("preferences-editor-width");
  preferences.editorHeight = ui.getProperty("preferences-editor-height");

  auto graphTree = parameters.getChildWithName("graph");
  if (graphTree.isValid()) {
    graph->async = graphTree.getProperty("async");

    // nodes
    auto nodesTree = graphTree.getChildWithName("nodes");
    if (nodesTree.isValid()) {
      for (auto const &nodeTree: nodesTree) {
        auto typeId = nodeTree.getType().toString().toStdString();
        auto node = ProcessorRegistry::makeNodeProcessor(graph, typeId);
        if (node != nullptr) {
          juce::String id = nodeTree.getProperty("id");
          auto optId = uuid::from_string(id.toStdString());
          jassert(optId != std::nullopt);
          node->m_id.swap(optId.value());

          juce::String name = nodeTree.getProperty("name");
          node->m_name = name.toStdString();
          node->m_sticky = nodeTree.getProperty("sticky");
          node->set_muted_no_notification(nodeTree.getProperty("muted"));

          // ins
          auto insTree = nodeTree.getChildWithName("ins");
          if (insTree.isValid()) {
            for (auto const &pinTree: insTree) {
              Graph::Node::Pin pin;
              pin.m_kind = Graph::Node::PinKind::In;
              juce::String pinId = pinTree.getProperty("id");
              auto optPinId = uuid::from_string(pinId.toStdString());
              jassert(optPinId != std::nullopt);
              pin.m_id.swap(optPinId.value());

              int order = pinTree.getProperty("order");
              pin.m_order = static_cast<std::uint32_t>(order);

              juce::String ownerNodeId = pinTree.getProperty("owner_node_id");
              auto optOwnerNodeId = uuid::from_string(ownerNodeId.toStdString());
              jassert(optOwnerNodeId != std::nullopt);
              pin.m_owner_node_id.swap(optOwnerNodeId.value());
              node->m_ins[pin.m_id] = pin;
            }
          }

          // outs
          auto outsTree = nodeTree.getChildWithName("outs");
          if (outsTree.isValid()) {
            for (auto const &pinTree: outsTree) {
              Graph::Node::Pin pin;
              pin.m_kind = Graph::Node::PinKind::Out;
              juce::String pinId = pinTree.getProperty("id");
              auto optPinId = uuid::from_string(pinId.toStdString());
              jassert(optPinId != std::nullopt);
              pin.m_id.swap(optPinId.value());

              int order = pinTree.getProperty("order");
              pin.m_order = static_cast<std::uint32_t>(order);

              juce::String ownerNodeId = pinTree.getProperty("owner_node_id");
              auto optOwnerNodeId = uuid::from_string(ownerNodeId.toStdString());
              jassert(optOwnerNodeId != std::nullopt);
              pin.m_owner_node_id.swap(optOwnerNodeId.value());
              node->m_outs[pin.m_id] = pin;
            }
          }

          if (auto np = dynamic_cast<NodeProcessor *>(node)) {
            // let the processor restore its state
            np->restoreState(nodeTree);
          }
          graph->add_node(node);
        }
      }
    }

    // edges
    auto edgesTree = graphTree.getChildWithName("edges");
    if (edgesTree.isValid()) {
      for (auto const &edgeTree: edgesTree) {
        Graph::Edge edge;

        juce::String id = edgeTree.getProperty("id");
        auto optId = uuid::from_string(id.toStdString());
        jassert(optId != std::nullopt);
        edge.m_id.swap(optId.value());

        id = edgeTree.getProperty("source_node_id");
        optId = uuid::from_string(id.toStdString());
        jassert(optId != std::nullopt);
        edge.m_source_node_id.swap(optId.value());

        id = edgeTree.getProperty("source_pin_id");
        optId = uuid::from_string(id.toStdString());
        jassert(optId != std::nullopt);
        edge.m_source_pin_id.swap(optId.value());

        id = edgeTree.getProperty("target_node_id");
        optId = uuid::from_string(id.toStdString());
        jassert(optId != std::nullopt);
        edge.m_target_node_id.swap(optId.value());

        id = edgeTree.getProperty("target_pin_id");
        optId = uuid::from_string(id.toStdString());
        jassert(optId != std::nullopt);
        edge.m_target_pin_id.swap(optId.value());

        graph->m_edges[edge.m_id] = edge;
      }
    }

    // descriptors
    auto descriptorsTree = graphTree.getChildWithName("descriptors");
    if (descriptorsTree.isValid()) {
      for (auto const &descriptorTree: descriptorsTree) {
        juce::String id = descriptorTree.getType().toString();
        auto optId = uuid::from_string(id.toStdString());
        jassert(optId != std::nullopt);
        auto nodeId = optId.value();
        auto descriptor = NodeDescriptor::restore(descriptorTree);
        std::unique_ptr<NodeDescriptor> p(descriptor);
        auto ptr = graph->m_nodes.find(nodeId);
        if (ptr != std::end(graph->m_nodes)) {
          if (auto np = dynamic_cast<NodeProcessor *>(ptr->second)) {
            descriptor->processor = np;
            nodeDescriptors[nodeId] = std::move(p);
          }
        }
      }
    }
  }

  recoverMidiInOut();
  graph->debug();

}

void AudioPluginAudioProcessor::recoverMidiInOut() {
  auto in = std::find_if(graph->m_nodes.begin(), graph->m_nodes.end(), [&](auto const &n) {
    auto node = n.second;
    if (auto np = dynamic_cast<NodeProcessor *>(node)) {
      return np->typeId() == Processors::midiInNodeProcessor;
    } else return false;
  });

  if (in != std::end(graph->m_nodes)) {
    if (auto np = dynamic_cast<MidiInNodeProcessor *>(in->second)) {
      this->midiIn = np;
    }
  }

  auto out = std::find_if(graph->m_nodes.begin(), graph->m_nodes.end(), [&](auto const &n) {
    auto node = n.second;
    if (auto np = dynamic_cast<NodeProcessor *>(node)) {
      return np->typeId() == Processors::midiOutNodeProcessor;
    } else return false;
  });

  if (out != std::end(graph->m_nodes)) {
    if (auto np = dynamic_cast<MidiOutNodeProcessor *>(out->second)) {
      this->midiOut = np;
    }
  }

}

void AudioPluginAudioProcessor::assignMidiInOutDescriptors() {
  GraphViewTheme theme;
  auto midiInDescriptor = std::make_unique<NodeDescriptor>();
  midiIn->m_sticky = true;
  midiInDescriptor->kind = NodeDescriptorKind::Plain;
  midiInDescriptor->processor = midiIn;
  midiInDescriptor->theme = theme;
  midiInDescriptor->scale = juce::AffineTransform::scale(theme.initialScaleFactor);
  midiInDescriptor->translation = juce::AffineTransform().translated(
    static_cast<float>(preferences.editorWidth) / 2 - 80, 10.0f);
  midiInDescriptor->width = 100;
  midiInDescriptor->height = 40;
  midiInDescriptor->selected = false;
  midiInDescriptor->scaleFactor = theme.initialScaleFactor;
  midiInDescriptor->cNodeBackgroundCurrent = theme.cNodeBackground;
  midiInDescriptor->position = juce::Point<float>(0.0f, 0.0f);
  nodeDescriptors[midiIn->m_id] = std::move(midiInDescriptor);

  auto midiOutDescriptor = std::make_unique<NodeDescriptor>();
  midiOut->m_sticky = true;
  midiOutDescriptor->kind = NodeDescriptorKind::Plain;
  midiOutDescriptor->processor = midiOut;
  midiOutDescriptor->theme = theme;
  midiOutDescriptor->scale = juce::AffineTransform::scale(theme.initialScaleFactor);
  midiOutDescriptor->translation = juce::AffineTransform().translated(
    static_cast<float>(preferences.editorWidth) / 2 - 80,
    static_cast<float>(preferences.editorHeight) - 40);
  midiOutDescriptor->width = 100;
  midiOutDescriptor->height = 40;
  midiOutDescriptor->selected = false;
  midiOutDescriptor->scaleFactor = theme.initialScaleFactor;
  midiOutDescriptor->cNodeBackgroundCurrent = theme.cNodeBackground;
  midiOutDescriptor->position = juce::Point<float>(0.0f, 0.0f);
  nodeDescriptors[midiOut->m_id] = std::move(midiOutDescriptor);
}


void AudioPluginAudioProcessor::getStateInformation(juce::MemoryBlock &destData) {
  saveState();
  juce::MemoryOutputStream out{destData, false};
  parameters.writeToStream(out);
}

void AudioPluginAudioProcessor::setStateInformation(const void *data, int sizeInBytes) {
  juce::MemoryInputStream in{data, static_cast<size_t>(sizeInBytes), false};
  parameters = juce::ValueTree::readFromStream(in);
  restoreState();
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new AudioPluginAudioProcessor();
}
