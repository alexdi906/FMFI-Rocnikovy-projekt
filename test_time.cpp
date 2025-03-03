#include <impl/basic/include.hpp>
#include "io/graph6.hpp"
#include "sat/solver_cmsat.hpp"
#include "ecd.hpp"
#include "ecd_sat.hpp"
#include "util/cxxopts.hpp"

#include <chrono>
using namespace std::chrono;
using namespace ba_graph;

CMSatSolver solver;

int main(){


    auto graphs = read_graph6_file("../no_ECD/16_4_3.clawfree.g6.C_NO").graphs();

    double sum = 0;
    for(auto &G : graphs)
    {
        for(int i = 0; i<=G.size()/4;++i)
        {
            auto start = high_resolution_clock::now();

            has_ecd_size_sat(solver,G,i);
            auto stop = high_resolution_clock::now();
            duration<double> elapsed = (stop - start);
            sum += elapsed.count();
        }
    }


    std::cout << sum << std::endl;


}