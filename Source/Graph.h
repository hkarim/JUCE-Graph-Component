#pragma once


#include <memory>
#include <string>
#include <vector>
#include <functional>

struct Graph {
    
    enum class PinType;
    struct Pin;
    struct Node;
    struct Edge;
    
    struct NodeListener {
        virtual ~NodeListener() {}
        virtual void onData(const Node* sourceNode, const Pin* sourcePin, const var& data) {}
    };
    
    std::vector<std::unique_ptr<Node>> nodes;
    std::vector<std::unique_ptr<Edge>> edges;
    
    enum class PinType { In, Out };
    
    struct Pin {
        
        const PinType pinType;
        const Node* node;
        const int order;
        
        Pin(const PinType& pinType, const Node* node, const int order) : pinType(pinType), node(node), order(order) {
            
        }
        
        void flow(const var& data) const {
            if (pinType == PinType::In) {
                //printf("[flow] in-pin:%d data:%s\n", order, data.stringValue.c_str());
                node->flow(this, data);
            } else {
                //printf("[flow] out-pin:%d data:%s\n", order, data.stringValue.c_str());
                std::vector<const Pin*> targets;
                node->graph->targets(this, targets);
                for (auto& p : targets) p->flow(data);
            }
        }
    };
    
    struct Node {
        
        const Graph* graph;
        const std::string name;
        std::vector<std::unique_ptr<Pin>> ins;
        std::vector<std::unique_ptr<Pin>> outs;
        std::vector<NodeListener*> listeners;
        
        Node(const Graph* graph, const std::string& name, const int numIns, const int numOuts) : graph(graph), name(name) {
            for (auto i = 0; i < numIns; i++) {
                auto pin = std::make_unique<Pin>(PinType::In, this, i);
                ins.push_back(std::move(pin));
            }
            
            for (auto i = 0; i <numOuts; i++) {
                auto pin = std::make_unique<Pin>(PinType::Out, this, i);
                outs.push_back(std::move(pin));
            }
        }
        
        void addListener(NodeListener* listener) {
            listeners.push_back(listener);
        }
        
        void removeListener(NodeListener* listener) {
            auto removeItr = std::remove_if(std::begin(listeners), std::end(listeners), [&](auto& current) -> bool { return current == listener; });
            if (removeItr != std::end(listeners)) {
                listeners.erase(removeItr);
            }
        }
        
        void flow(const Pin* source, const var& data) const {
            
            for (auto& l : listeners) l->onData(this, source, data);
            
            //publish(data);
            
        }
        
        void publish(const var& data) const {
            //printf("[publish] node:%s data:%s\n", name.c_str(), data.stringValue.c_str());
            for (auto& p : outs) {
                p->flow(data);
            }
        }
    };
    
    struct Edge {
        
        const Pin* source;
        const Pin* target;
        
        Edge(const Pin* source, const Pin* target) : source(source), target(target) {
            
        }
    };
    
    Node* addNode(const std::string& name, const int numIns, const int numOuts) {
        //printf("[graph-add-node] name:%s, ins: %d, outs:%d\n", name.c_str(), numIns, numOuts);
        auto node = std::make_unique<Node>(this, name, numIns, numOuts);
        auto ptr = node.get();
        nodes.push_back(std::move(node));
        return ptr;
    }
    
    void removeNode(const Node* node) {
        //printf("[graph-remove-node] name:%s\n", node->name.c_str());
        // remove all connected edges
        std::vector<const Edge*> edgesToRemove;
        for (auto& e : edges) {
            if (e->source->node == node || e->target->node == node) {
                edgesToRemove.push_back(e.get());
            }
        }
        for (auto& e : edgesToRemove) {
            removeEdge(e);
        }
        
        // remove the node
        auto removeItr = std::remove_if(std::begin(nodes), std::end(nodes), [&](auto& current) -> bool { return current.get() == node; });
        nodes.erase(removeItr);
        //report();
    }
    
    const Edge* addEdge(const Pin* source, const Pin* target) {
        //printf("[graph-add-edge] source-node:%s source-pin:%d, target-node:%s, target-pin:%d\n",
        //       source->node->name.c_str(), source->order, target->node->name.c_str(), target->order);
        auto edge = std::make_unique<Edge>(source, target);
        auto ptr = edge.get();
        edges.push_back(std::move(edge));
        return ptr;
    }
    
    void removeEdge(const Edge* edge) {
        //printf("[graph-remove-edge] source-node:%s source-pin:%d, target-node:%s, target-pin:%d\n",
        //       edge->source->node->name.c_str(),
        //       edge->source->order,
        //       edge->target->node->name.c_str(),
        //       edge->target->order);
        auto removeItr = std::remove_if(std::begin(edges), std::end(edges), [&](auto& current) -> bool { return current.get() == edge; } );
        edges.erase(removeItr);
        //report();
    }
    
    void targets(const Pin* source, std::vector<const Pin*>& pins) const {
        pins.clear();
        for (auto& e : edges) {
            if (e->source == source) pins.push_back(e->target);
        }
        
    }
    
    void targets(const Node* source, std::vector<const Node*>& nodes) const {
        nodes.clear();
        for (auto& e : edges) {
            if (e->source->node == source) nodes.push_back(e->target->node);
        }
 
    }
    
    void dfs(const Node* node, const std::function<void (const Node*)>& visit) const {
        std::vector<const Node*> nodes;
        targets(node, nodes);
        for (auto& n : nodes) dfs(n, visit);
        visit(node);
    }
    
    void bfs(const Node* node, const std::function<void (const Node*)>& visit) const {
        visit(node);
        std::vector<const Node*> nodes;
        targets(node, nodes);
        for (auto& n : nodes) bfs(n, visit);
    }
    
    void report() const {
        printf("#nodes:%lu, #edges:%lu\n", nodes.size(), edges.size());
    }
    
    
};


