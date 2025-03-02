#ifndef EVENTS_FLUSH_CLUSTER_EVENT_HPP
#define EVENTS_FLUSH_CLUSTER_EVENT_HPP

#include "../Model.hpp"

struct FlushClusterEvent {
    const Cluster* cluster;
};

#endif // EVENTS_FLUSH_CLUSTER_EVENT_HPP
