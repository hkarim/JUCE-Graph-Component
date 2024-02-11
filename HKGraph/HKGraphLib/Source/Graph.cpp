#include "Graph.h"

Graph::Graph() {
  std::random_device rd;
  auto seed_data = std::array<int, std::mt19937::state_size>{};
  std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
  std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
  m_mt19937 = new std::mt19937(seq);
  m_uuid_generator = new uuids::uuid_random_generator{m_mt19937};
}

Graph::~Graph() {
  m_edges.clear();
  m_pins.clear();
  for (auto &[_, n]: m_nodes) {
    delete n;
    n = nullptr;
  }
  m_nodes.clear();

  delete m_uuid_generator;
  delete m_mt19937;
}

uuid Graph::make_uuid() {
  return m_uuid_generator->operator()();
}

void Graph::add_node(Node *node) {
  std::lock_guard<std::mutex> lock(m_mutex);

  auto id = node->m_id;
  for (auto &p: node->m_ins) {
    m_pins[p.first] = p.second;
  }
  for (auto &p: node->m_outs) {
    m_pins[p.first] = p.second;
  }
  m_nodes[id] = node;
}

void Graph::remove_node(const uuid &node_id) {
  std::lock_guard<std::mutex> lock(m_mutex);

  auto n_itr = m_nodes.find(node_id);
  if (n_itr != std::end(m_nodes)) {

    // remove edges
    auto edge_itr = std::begin(m_edges);
    while (edge_itr != std::end(m_edges)) {
      if (edge_itr->second.m_source_node_id == n_itr->first ||
          edge_itr->second.m_target_node_id == n_itr->first) {
        edge_itr = m_edges.erase(edge_itr);
      } else {
        ++edge_itr;
      }
    }

    // remove pins
    for (const auto &p: n_itr->second->m_ins) {
      m_pins.erase(p.first);
    }
    for (const auto &p: n_itr->second->m_outs) {
      m_pins.erase(p.first);
    }

    // remove node
    auto *p = n_itr->second;
    delete p;
    n_itr->second = nullptr;
    m_nodes.erase(n_itr->first);
    notify_listeners(Graph::Event::NodeRemoved);
  }
}

std::optional<Graph::Edge> Graph::connect(const uuid &source_pin_id, const uuid &target_pin_id) {
  std::lock_guard<std::mutex> lock(m_mutex);

  auto existing_itr =
    std::find_if(
      std::begin(m_edges),
      std::end(m_edges),
      [&](const auto &e) -> bool {
        return e.second.m_source_pin_id == source_pin_id &&
               e.second.m_target_pin_id == target_pin_id;
      });
  auto source_pin_itr = m_pins.find(source_pin_id);
  auto target_pin_itr = m_pins.find(target_pin_id);
  if (existing_itr == std::end(m_edges) &&
      source_pin_itr != std::end(m_pins) &&
      target_pin_itr != std::end(m_pins)) {
    auto e = Edge{
      .m_id = make_uuid(),
      .m_source_node_id = source_pin_itr->second.m_owner_node_id,
      .m_source_pin_id = source_pin_itr->first,
      .m_target_node_id = target_pin_itr->second.m_owner_node_id,
      .m_target_pin_id = target_pin_itr->first,
    };
    m_edges[e.m_id] = e;
    return e;
  }
  return std::nullopt;
}

bool Graph::disconnect(const uuid &edge_id) {
  std::lock_guard<std::mutex> lock(m_mutex);

  auto n = m_edges.erase(edge_id);
  if (n != 0) {
    notify_listeners(Graph::Event::EdgeRemoved);
  }
  return n != 0;
}

void Graph::add_listener(Graph::Listener *listener) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_listeners.push_back(listener);
}

void Graph::remove_listener(Listener *listener) {
  std::lock_guard<std::mutex> lock(m_mutex);
  std::erase_if(m_listeners, [&](auto &l) { return l == listener; });
}

void Graph::notify_listeners(const Graph::Event &event) {
  if (async) {
    auto f = std::async(
      std::launch::async,
      [&]() {
        for (auto &l: m_listeners) {
          l->on_graph_event(event);
        }
      });
    f.wait();
  } else {
    for (auto &l: m_listeners) {
      l->on_graph_event(event);
    }
  }
}

void Graph::Node::Pin::async_dispatch(Graph *graph, Data &data) {
  if (graph->async) {
    auto f = std::async(
      std::launch::async,
      [&](auto g, auto d) { this->on_data(g, d); }, graph, data);
    f.wait();
  } else {
    on_data(graph, data);
  }
}

void Graph::Node::Pin::on_data(Graph *graph, Data &data) {
  auto node_itr = graph->m_nodes.find(this->m_owner_node_id);
  if (node_itr != std::end(graph->m_nodes) && !node_itr->second->m_muted) {
    if (this->m_kind == Graph::Node::PinKind::In) {
      node_itr->second->async_dispatch(graph, *this, data);
    } else if (this->m_kind == Graph::Node::PinKind::Out) {
      auto e_itr = std::begin(graph->m_edges);
      while (e_itr != std::end(graph->m_edges)) {
        if (e_itr->second.m_source_pin_id == this->m_id) {
          e_itr->second.async_dispatch(graph, data);
        }
        ++e_itr;
      }
    }
  }
}

void Graph::Node::async_dispatch(Graph *graph, const std::optional<const Node::Pin> &pin, Data &data) {
  if (!m_muted) {
    if (graph->async) {
      auto f = std::async(
        std::launch::async,
        [&](auto g, auto p, auto d) { this->on_data(g, p, d); }, graph, pin, data);
      f.wait();
    } else {
      this->on_data(graph, pin, data);
    }
  }
}

void Graph::Node::on_data(Graph *graph, const std::optional<const Node::Pin> &pin, Data &data) {
}

void Graph::Edge::async_dispatch(Graph *graph, Data &data) const {
  if (graph->async) {
    auto f = std::async(
      std::launch::async,
      [&](auto g, auto d) { this->on_data(g, d); }, graph, data);
    f.wait();
  } else {
    this->on_data(graph, data);
  }
}

void Graph::Edge::on_data(Graph *graph, Data &data) const {
  auto p_itr = graph->m_pins.find(m_target_pin_id);
  if (p_itr != std::end(graph->m_pins)) {
    p_itr->second.async_dispatch(graph, data);
  }
}
