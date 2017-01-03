

#include "EdgeComponent.h"


EdgeComponent::EdgeComponent(const GraphViewTheme& theme, NodeComponent::PinComponent* startPin, NodeComponent::PinComponent* endPin, const Graph::Edge* model) :
    theme(theme), startPin(startPin), endPin(endPin), model(model) {
        
        
}

EdgeComponent::~EdgeComponent() {
    
    
}

bool EdgeComponent::hitTest(int x, int y) {
    auto position = Point<float>(x, y);
    Point<float> nearest;
    path.getNearestPoint(Point<float>(x,y), nearest);
    auto distance = position.getDistanceFrom(nearest);
    return  distance < 5;
}



void EdgeComponent::paint(Graphics& g) {
    auto w = getWidth();
    auto h = getHeight();
    
    auto pstart = getLocalPoint(startPin, Point<float>(startPin->getWidth() / 2, 0));
    auto pend = getLocalPoint(endPin, Point<float>(endPin->getWidth() / 2, 0));
    
    path.clear();
    
    path.startNewSubPath(pstart.x, pstart.y);
    if (inverted) {
        path.cubicTo(w, h * 0.5f, 0.0f, h * 0.5f, pend.x, pend.y);
    } else {
        path.cubicTo(0.0f, h * 0.5f, w, h * 0.5f, pend.x, pend.y);
    }
    
    
    if (selected) {
        g.setColour(Colour(theme.cEdgeSelected));
    } else {
        g.setColour(Colour(theme.cEdge));
    }
    
    g.strokePath(path, PathStrokeType(theme.edgeStrokeWidth));
    
    /*
    Path boundry;
    boundry.addRectangle(getLocalBounds());
    g.strokePath(boundry, PathStrokeType(1.0f));
    */
    
}



bool EdgeComponent::isConnecting(NodeComponent::PinComponent* first, NodeComponent::PinComponent* second) {
    return (startPin == first && endPin == second) || (startPin == second && endPin == first);
}













