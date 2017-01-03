#pragma once

struct GraphViewTheme {
    
    int pinWidth = 10;
    int pinHeight = 7;
    int pinSpacing = 10;
    int nodeHeight = 40;
    int hostComponentDragAreaHeight = 30;
    float edgeStrokeWidth = 3.0f;
    
    float initialScaleFactor = 1.0f;
    float scaleStep = 0.2f;
    float scaleMax = 2.0f;
    float scaleMin = 0.4f;
    
    unsigned int cBackground = 0xFF242424;
    unsigned int cNodeBackground = 0xCF4F4F4F;
    unsigned int cNodeBackgroundHover = 0xCF656464;
    unsigned int cNodeBackgroundSelected = 0xCFDE4F18;
    unsigned int cPinBackground = 0xFF767676;
    unsigned int cPinBackgroundSelected = 0xFFE27E2A;
    unsigned int cEdge = 0xFF767676;
    unsigned int cEdgeSelected = 0xFFE27E2A;
    unsigned int cSelectionBackground = 0x3D555555;
    
};
