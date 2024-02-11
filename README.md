### JUCE Graph Component

This is a [JUCE Framework](https://www.juce.com) set of components that allows drawing a graph by connecting nodes and edges.
The component is inspired by the `plugin host demo` application that comes with JUCE, but this one is a different implementation.

<img src="https://github.com/hkarim/JUCE-Graph-Component/blob/master/Doc/ss-plugin-01.png">

### Features

- Each node can be scaled individually, zoom-in `cmd + '+'`, zoom-out `cmd + '-'`, or zoom to original scale `cmd + '0'`
- Mute and un-mute nodes using `cmd + 'm'`
- Delete a node or edge by selecting it and hit delete or backspace
- Removing an edge by double-clicking it is also supported
- Multi-select using keyboard or mouse is supported
- Duplicate a selection using `cmd + 'd'`
- Backed by a simple graph that supports communication between nodes
- The graph can run in both in sync and async modes
- Sample application to preview the component and how to use it
- Very simple theming for changing the overall components look and feel

### Demo Plugin - HKGraphMidi

- All the features of the component backed in a VST3 plugin
- Sample MIDI processors including a note transposer, channel splitter, channel router, on-screen keyboard, etc.
- Saving and loading plugin state is also supported

### Demo Application - HKGraphApp

- Right-click on the demo window to add example nodes, and drag pins to create edges
- Click and drag on the window to select multiple nodes and edges
- To scale a node, select it and hit `cmd + '+'`, `cmd + '-'` or `cmd + '0'`
- Delete a node or edge by selecting it and hit delete or backspace
- Duplicate a node using `cmd + 'd'`, duplicating host nodes is not supported yet

### Building

The following steps are tested only on macOS:

- You will need a C++20 compiler, the build is tested with Clang 17
- See `CMakeLists.txt` for instructions whether you have `JUCE` locally or installed using a package manager
- Assuming you have `vcpkg` and `JUCE` installed, the following will get you started:

```sh
cmake -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake . -B cmake-build
cmake --build cmake-build --config Release --target HKGraphMidi_VST3
cmake --build cmake-build --config Release --target HKGraphApp
```

### Legacy

Old implementation is now on the `legacy` branch

### License

MIT




