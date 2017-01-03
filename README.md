### JUCE Graph Component

This is a [JUCE Framework](https://www.juce.com) set components that allows drawing a graph by connecting nodes and edges.

<img src="https://github.com/hkarim/JUCE-Graph-Component/blob/master/Doc/ss-01.png">

### Features

- Each node can be scaled individually, zoom-in `cmd + '+'`, zoom-out `cmd + '-'`, or zoom to original scale `cmd + '0'`
- Multi-select using keyboard or mouse is supported
- Special type of node that can host any component
- Backed by a simple graph that supports communication between nodes

### Building

The following steps is tested only on macOS:

- You will need `boost::variant` on the headers search path
- Use the provided `graph-viewer.jucer` to generate a project for your platform or use the provided XCode project


