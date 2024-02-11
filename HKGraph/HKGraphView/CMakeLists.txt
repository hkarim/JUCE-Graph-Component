cmake_minimum_required(VERSION 3.15)

project(HKGraphView VERSION 0.0.1)

add_library(HKGraphView STATIC)

target_include_directories(HKGraphView
        INTERFACE
        $<TARGET_PROPERTY:HKGraphView,INCLUDE_DIRECTORIES>)

target_include_directories(HKGraphView
        PUBLIC
        ../HKGraphLib/Source
)

target_sources(HKGraphView
        PRIVATE
        Source/EdgeComponent.cpp
        Source/GraphViewComponent.cpp
        Source/NodeComponent.cpp
        Source/HostNodeComponent.cpp
        Source/SelectionComponent.cpp
        Source/UnboundEdgeComponent.cpp
)

target_compile_definitions(HKGraphView
        PRIVATE
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
        JUCE_APPLICATION_NAME_STRING= "$<TARGET_PROPERTY:HKGraphView,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING= "$<TARGET_PROPERTY:HKGraphView,JUCE_VERSION>")

target_link_libraries(HKGraphView
        PRIVATE
        # GuiAppData            # If we'd created a binary data target, we'd link to it here
        HKGraphLib
        juce::juce_gui_extra
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
