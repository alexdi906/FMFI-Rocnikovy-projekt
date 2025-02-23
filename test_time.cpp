#include <impl/basic/include.hpp>
#include <sat/solver_cmsat.hpp>
#include "ecd_sat.hpp"
#include <algorithms/isomorphism/isomorphism.hpp>
#include <cassert>
#include <graphs.hpp>
#include <invariants/connectivity.hpp>
#include <io/graph6.hpp>
#include <io/print_nice.hpp>
#include <iostream>
#include <operations/add_graph.hpp>
#include <operations/line_graph.hpp>
#include <string>
#include <vector>
#include <invariants/colouring.hpp>
#include <chrono>

using namespace std::chrono;


using namespace ba_graph;
int main(){
    auto start = high_resolution_clock::now();
    // line graphs of cubic graphs with chromatic index 3 must have ecd

    std::string path = "../ba-graph/resources/graphs/4regular/";
    // line graphs of cubic graphs with chromatic index 3 must have ecd

    const std::vector<int> order_no = {13};

    CMSatSolver solver;
    // const std::vector<int> order_no = {5, 9, 13};
    std::cerr << "hap" << std::endl;
    for(int o : order_no)
    {

        std::string claw ="clawfree";
        // std::string claw = "3c";
        std::string file = path + "no_ECD/" + (o<10?"0":"") + std::to_string(o) +  "_4_3." + claw + ".g6.C_NO";

        Factory f;
        auto graphs = read_graph6_file(file, f).graphs();

        for(auto&G : graphs)
        {
            // Graph gg = line_graph(G);
            // std::vector<Number> order;
            // distance(gg, 0, gg.order()-1, INT_MAX, &order);
            // assert(std::is_sorted(order.begin(), order.end()));
            // std::cerr << order << std::endl;
            std::cerr << ecd_size_sat(solver, G) << std::endl;
            // assert(ecd_size(G) == -1);
        }
    }


    auto stop = high_resolution_clock::now();

    std::chrono::duration<double> elapsed = (stop - start);

    std::cout << elapsed.count() << std::endl;

}
