#include <chrono>
#include <impl/basic/include.hpp>
#include <invariants/colouring.hpp>

#include "ecd.hpp"
#include "ecd_sat.hpp"
#include "io/graph6.hpp"
#include "sat/solver_cmsat.hpp"
#include "util/cxxopts.hpp"
using namespace std::chrono;
using namespace ba_graph;

CMSatSolver solver;

int main()
{
    auto graphs = read_graph6_file("graphs/3regular/12_3_3.g6").graphs();
    int cnt_no = 0;
    int cnt_chi = 0;
    auto start = high_resolution_clock::now();
    for(auto& g : graphs)
    {
        if(chromatic_index_basic(g) != 3)
        {
            cnt_chi++;
            Graph lg = line_graph(g);
            std::cout << ecd_size_sat(solver, lg) << std::endl;
            cnt_no++;
        }
    }
    std::cout << cnt_no << " " << cnt_chi << std::endl;
    auto stop = high_resolution_clock::now();
    duration<double> elapsed = (stop - start);
    std::cout << elapsed.count() << std::endl;
    // auto graphs = read_graph6_file("../no_ECD/16_4_3.clawfree.g6.C_NO").graphs();

    // double sum = 0;
    // for(auto &G : graphs)
    // {
    //     for(int i = 0; i<=G.size()/4;++i)
    //     {
    // auto start = high_resolution_clock::now();
    //
    //         has_ecd_size_sat(solver,G,i);
    //         auto stop = high_resolution_clock::now();
    //         duration<double> elapsed = (stop - start);
    //         sum += elapsed.count();
    //     }
    // }
    //
    //
    // std::cout << sum << std::endl;
}