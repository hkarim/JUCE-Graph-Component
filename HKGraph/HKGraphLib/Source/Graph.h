#pragma once

#include <iostream>
#include <map>
#include <any>
#include <future>
#include "uuid.h"

using uuid = uuids::uuid;
using Data = std::any;

struct Graph {

  enum class Event {
    NodeAdded,
    NodeRemoved,
    NodeMuted,
    EdgeAdded,
    EdgeRemoved,
  };

  struct Listener {
    virtual void on_graph_event(const Event &event) = 0;
  };

  struct Node {
    enum class PinKind {
      In, Out
    };

    struct Pin {
      void async_dispatch(Graph *graph, Data &data);

      void on_data(Graph *graph, Data &data);

      uuid m_id;
      std::uint32_t m_order;
      PinKind m_kind;
      uuid m_owner_node_id;
    };

    virtual ~Node() = default;

    [[nodiscard]] bool is_muted() const {
      return m_muted;
    }

    void set_muted_no_notification(bool v) {
      m_muted = v;
    }

    void set_muted(Graph *graph, bool v) {
      m_muted = v;
      graph->notify_listeners(Graph::Event::NodeMuted);
    }

    void toggle_muted(Graph *graph) {
      m_muted = !m_muted;
      graph->notify_listeners(Graph::Event::NodeMuted);
    }

    void async_dispatch(Graph *graph, const std::optional<const Node::Pin> &pin, Data &data);

    virtual void on_data(Graph *graph, const std::optional<const Node::Pin> &pin, Data &data);

    void debug() const {
      std::cout
        << "Node " << m_name << " "
        << "["
        << "id: " << m_id << ", "
        << "ins: " << m_ins.size() << ", "
        << "outs: " << m_outs.size() << ", "
        << "] "
        << std::endl;
    }

    uuid m_id;
    std::string m_name;
    std::unordered_map<uuid, Pin> m_ins;
    std::unordered_map<uuid, Pin> m_outs;
    bool m_sticky{false};
  protected:
    bool m_muted{false};
  };

  struct Edge {
    void async_dispatch(Graph *graph, Data &data) const;

    void on_data(Graph *graph, Data &data) const;

    uuid m_id;
    uuid m_source_node_id;
    uuid m_source_pin_id;
    uuid m_target_node_id;
    uuid m_target_pin_id;
  };

  Graph();

  ~Graph();

  uuid make_uuid();

  void add_node(Node *node);

  void remove_node(const uuid &node_id);

  std::optional<Edge> connect(const uuid &source_pin_id, const uuid &target_pin_id);

  bool disconnect(const uuid &edge_id);

  void debug() const {
    std::cout
      << "Graph ["
      << "nodes: " << m_nodes.size() << ", "
      << "edges: " << m_edges.size() << ", "
      << "pins: " << m_pins.size() << ", "
      << "] "
      << std::endl;
  }

  void add_listener(Listener *listener);

  void remove_listener(Listener *listener);

  void notify_listeners(const Graph::Event &event);

  bool async{true};
  std::unordered_map<uuid, Node *> m_nodes;
  std::unordered_map<uuid, Node::Pin> m_pins;
  std::unordered_map<uuid, Edge> m_edges;
  std::mutex m_mutex;
  std::vector<Listener *> m_listeners;
private:
  std::mt19937 *m_mt19937;
  uuids::uuid_random_generator *m_uuid_generator;
};