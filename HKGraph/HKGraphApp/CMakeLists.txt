cmake_minimum_required(VERSION 3.15)

project(HKGraphApp VERSION 0.0.1)

juce_add_gui_app(HKGraphApp
    # VERSION ...                       # Set this if the app version is different to the project version
    # ICON_BIG ...                      # ICON_* arguments specify a path to an image file to use as an icon
    # ICON_SMALL ...
    # DOCUMENT_EXTENSIONS ...           # Specify file extensions that should be associated with this app
    # COMPANY_NAME ...                  # Specify the name of the app's author
    PRODUCT_NAME "HKGraphApp")     # The name of the final executable, which can differ from the target name

#juce_generate_juce_header(HKGraphApp)

target_include_directories(HKGraphApp
        PUBLIC
        ../External/Source
        ../HKGraphLib/Source
        ../HKGraphView/Source
)

target_sources(HKGraphApp
    PRIVATE
        Source/GraphEditor.cpp
        Source/Main.cpp
)

target_compile_definitions(HKGraphApp
    PRIVATE
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:HKGraphApp,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:HKGraphApp,JUCE_VERSION>")

target_link_libraries(HKGraphApp
    PRIVATE
        # GuiAppData            # If we'd created a binary data target, we'd link to it here
        HKGraphView
        juce::juce_gui_extra
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
