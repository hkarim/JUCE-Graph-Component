

#include "../JuceLibraryCode/JuceHeader.h"
#include "GraphViewComponent.h"
#include "SampleHostedComponents.h"


class GraphViewerApplication : public JUCEApplication {
    
public:
    
    GraphViewerApplication() {}

    const String getApplicationName() override       { return ProjectInfo::projectName; }
    const String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override       { return true; }

    void initialise(const String& commandLine) override {
        mainWindow = new MainWindow (getApplicationName());
    }

    void shutdown() override {
        mainWindow = nullptr; // (deletes our window)
    }

    void systemRequestedQuit() override {
        quit();
    }

    void anotherInstanceStarted(const String& commandLine) override {
    }
    
    
    class MainView : public Component {
        
    public:
        
        std::unique_ptr<CustomGraphView> view;
        std::unique_ptr<Toolbar> toolbar;
        
        
        MainView() {
            
            view = std::make_unique<CustomGraphView>();
            view->setSize(800, 400);
            toolbar = std::make_unique<Toolbar>();
            
        }
        
        void resized() override {
            
            auto graphViewer = view.get();
            addAndMakeVisible(graphViewer);
            //addAndMakeVisible(toolbar.get());
            
            layout();
            
        }
        
        void childBoundsChanged(Component* child) override {
            layout();
        }
        
        void layout() {
            auto graphViewer = view.get();
            
            FlexBox flexBox;
            flexBox.flexDirection = FlexBox::Direction::column;
            flexBox.justifyContent = FlexBox::JustifyContent::flexStart;
            flexBox.alignItems = FlexBox::AlignItems::flexStart;
            flexBox.alignContent = FlexBox::AlignContent::stretch;
            
            FlexItem tbi(*toolbar);
            tbi.alignSelf = FlexItem::AlignSelf::stretch;
            tbi.minHeight = 40.0f;
            
            FlexItem gvi(*graphViewer);
            gvi.alignSelf = FlexItem::AlignSelf::stretch;
            gvi.minHeight = getHeight();
           
            
            //flexBox.items.add(tbi);
            flexBox.items.add(gvi);
            
            flexBox.performLayout(getBounds());
        }
        
    };

    
    class MainWindow : public DocumentWindow {
        
    public:
        
        std::unique_ptr<MainView> view;
        //std::unique_ptr<GraphController> controller;
        
        MainWindow(String name) : DocumentWindow(name, Colours::lightgrey, DocumentWindow::allButtons) {
            
            view = std::make_unique<MainView>();
            view->setBounds(0, 0, 800, 600);
            
            setUsingNativeTitleBar(true);
            setContentNonOwned(view.get(), true);

            centreWithSize(getWidth(), getHeight());
            setVisible(true);
            setResizable(true, false);
        }

        void closeButtonPressed() override {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }


    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    ScopedPointer<MainWindow> mainWindow;
};




START_JUCE_APPLICATION(GraphViewerApplication)





