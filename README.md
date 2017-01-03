### JUCE Graph Component

This is a [JUCE Framework](https://www.juce.com) set of components that allows drawing a graph by connecting nodes and edges.
The component is inspired by the `plugin host demo` application that comes with JUCE, but this one is a different implementation.

<img src="https://github.com/hkarim/JUCE-Graph-Component/blob/master/Doc/ss-01.png">

### Features

- Each node can be scaled individually, zoom-in `cmd + '+'`, zoom-out `cmd + '-'`, or zoom to original scale `cmd + '0'`
- Multi-select using keyboard or mouse is supported
- Special type of node that can host any component
- Backed by a simple graph that supports communication between nodes
- Sample application to preview the component and how to use it
- Very simple theming for changing the overall components look and feel

### Demo Application

- Right-click on the demo window to add example nodes, and drag pins to create edges
- Click and drag on the window to select multiple nodes and edges
- To scale a node, select it and hit `cmd + '+'`, `cmd + '-'` or `cmd + '0'`
- Delete a node or edge by selecting it and hit delete or backspace
- Duplicate a node using `cmd + 'd'`, duplicating host nodes is not supported yet

### Building

The following steps is tested only on macOS:

- You will need a C++14 compiler
- `boost::variant` must be on the headers search path
- Use the provided `graph-viewer.jucer` to generate a project for your platform or use the provided XCode project
- If you are generating a project from the jucer file, make sure you setup the correct JUCE modules paths correctly

### License
MIT




