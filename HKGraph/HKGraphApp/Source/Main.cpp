#include "GraphEditor.h"

class GraphViewerApplication : public juce::JUCEApplication {

public:

  GraphViewerApplication() = default;

  ~GraphViewerApplication() override = default;

  const juce::String getApplicationName() override { return "demo"; }

  const juce::String getApplicationVersion() override { return "1.0"; }

  bool moreThanOneInstanceAllowed() override { return true; }

  void initialise(const juce::String &commandLine) override {
    juce::ignoreUnused(commandLine);
    mainWindow = std::make_unique<MainWindow>(getApplicationName());
  }

  void shutdown() override {
  }

  void systemRequestedQuit() override {
    quit();
  }

  void anotherInstanceStarted(const juce::String &commandLine) override {
    juce::ignoreUnused(commandLine);
  }


  class MainView : public juce::Component {
  public:
    explicit MainView() : graph(new Graph()), view(new GraphEditor(graph)) {
      graph->async = false;
      view->setSize(800, 400);
    }

    ~MainView() override {
      delete graph;
    }

    void resized() override {
      auto graphViewer = view.get();
      addAndMakeVisible(graphViewer);
      layout();
    }

    void childBoundsChanged(Component *child) override {
      juce::ignoreUnused(child);
      layout();
    }

    void layout() {
      auto graphViewer = view.get();

      juce::FlexBox flexBox;
      flexBox.flexDirection = juce::FlexBox::Direction::column;
      flexBox.justifyContent = juce::FlexBox::JustifyContent::flexStart;
      flexBox.alignItems = juce::FlexBox::AlignItems::flexStart;
      flexBox.alignContent = juce::FlexBox::AlignContent::stretch;


      juce::FlexItem gvi(*graphViewer);
      gvi.alignSelf = juce::FlexItem::AlignSelf::stretch;
      gvi.minHeight = static_cast<float>(getHeight());
      flexBox.items.add(gvi);

      flexBox.performLayout(getBounds());
    }

  private:
    Graph *graph;
    std::unique_ptr<GraphEditor> view;
  };


  class MainWindow : public juce::DocumentWindow {
  public:
    std::unique_ptr<MainView> view;

    explicit MainWindow(const juce::String &name) : DocumentWindow(name, juce::Colours::lightgrey,
                                                                   DocumentWindow::allButtons) {
      view = std::make_unique<MainView>();
      view->setBounds(0, 0, 800, 600);
      setUsingNativeTitleBar(true);
      setContentNonOwned(view.get(), true);
      centreWithSize(getWidth(), getHeight());
      setVisible(true);
      setResizable(true, false);
    }

    ~MainWindow() override = default;

    void closeButtonPressed() override {
      JUCEApplication::getInstance()->systemRequestedQuit();
    }


  private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
  };

private:
  std::unique_ptr<MainWindow> mainWindow;
};


START_JUCE_APPLICATION(GraphViewerApplication)
