#include "transport_router.h"
#include <iostream>




namespace router {

    const double km_to_m = 1000;
    const double h_to_m = 60;



    TransportRouter::TransportRouter(const catalogue::TransportCatalogue& catalogue, int bus_wait_time, int bus_velocity)
        :bus_wait_time_(bus_wait_time), bus_velocity_(bus_velocity), catalogue_(catalogue)
    {
        BuildGraph();
    }




    void TransportRouter::BuildGraph() {

        size_t iter = 0;
        for (const auto& stop : catalogue_.GetStops()) {
            vertex_by_stop_.insert({ stop, {iter + 1, iter} });
            stop_by_vertex_.push_back(stop);
            stop_by_vertex_.push_back(stop);
            iter += 2;
        }

        graph_ = std::make_unique < graph::DirectedWeightedGraph<double>>(vertex_by_stop_.size() * 2);

        for (size_t i = 0; i < vertex_by_stop_.size() * 2; i += 2) {
            graph_->AddEdge({ i + 1, i, (double)bus_wait_time_ });
            route_elem_by_edge_.push_back({ RouteElemType::WAIT, stop_by_vertex_.at(i + 1), stop_by_vertex_.at(i), "", (double)bus_wait_time_, -1 });
        }

        ComputeDistancesAndGenerateEdges();
    }

    void TransportRouter::ComputeDistancesAndGenerateEdges() {

        for (const auto& bus : catalogue_.GetBuses()) {

            for (auto it_from = bus->stops.begin(); it_from != bus->stops.end(); ++it_from) {

                int total_distance = 0;
                int total_span = 0;

                for (auto it_to = std::next(it_from); it_to != bus->stops.end(); ++it_to) {
                    int distance = 0;


                    distance = catalogue_.GetDistanceBetweenStops(*std::prev(it_to), *it_to);
                    if (distance == -1) {
                        distance = catalogue_.GetDistanceBetweenStops(*it_to, *std::prev(it_to));
                    }

                    total_distance += distance;
                    ++total_span;
                    if (*it_from != *it_to) {

                        double time = total_distance / (km_to_m * bus_velocity_ / h_to_m);

                        graph_->AddEdge({ vertex_by_stop_.at(*it_from).second, vertex_by_stop_.at(*it_to).first, time });

                        route_elem_by_edge_.push_back(RouteElem{ RouteElemType::GO, (*it_from), (*it_to), bus->name, time, total_span });
                    }
                }
            }
        }
        router_ = std::make_unique<graph::Router<double>>(*graph_);
    }

    std::optional<const std::vector<RouteElem>> TransportRouter::ComputeRoute(const std::string_view from, const std::string_view to) {

        std::vector<RouteElem> result;

        if (from != to) {
            const auto route = router_->BuildRoute(vertex_by_stop_.at(catalogue_.FindStopByName(from)).first, vertex_by_stop_.at(catalogue_.FindStopByName(to)).first);

            if (route.has_value()) {
                for (const auto edge : route.value().edges) {
                    result.push_back(route_elem_by_edge_.at(edge));
                }
                return result;
            }
            else {
                return std::nullopt;
            }
        }
        else {
            return result;
        }
    }
}
