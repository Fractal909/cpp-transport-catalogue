#pragma once

#include <map>
#include <unordered_map>
#include <vector>
#include <memory>

#include "domain.h"
#include "transport_catalogue.h"
#include "router.h"

namespace router {

    using VertexByStop = std::unordered_map<const Stop*, std::pair<size_t, size_t>>;
    using StopByVertex = std::vector<const Stop*>;

    enum RouteElemType {
        WAIT,
        GO
    };

    struct RouteElem {
        RouteElemType type;
        const Stop* from;
        const Stop* to;
        std::string_view bus_name;
        double time;
        int span_count;
    };

    using RouteElemByEdge = std::vector<RouteElem>;


    class TransportRouter {
    public:
        TransportRouter(const catalogue::TransportCatalogue& catalogue, int bus_wait_time, int bus_velocity);
        std::optional<const std::vector<RouteElem>> ComputeRoute(const std::string_view from, const std::string_view to);
    private:
        void BuildGraph();
        void ComputeDistancesAndGenerateEdges();
        int bus_wait_time_;
        int bus_velocity_;
        const catalogue::TransportCatalogue& catalogue_;

        VertexByStop vertex_by_stop_;
        StopByVertex stop_by_vertex_;

        RouteElemByEdge route_elem_by_edge_;

        std::unique_ptr<graph::DirectedWeightedGraph<double>> graph_;
        std::unique_ptr<graph::Router<double>> router_;

    };

}