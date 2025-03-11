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
int cnt_no;

void process_graph(std::string& file_name, Graph& g, Factory& f, void* param)
{
    (void)file_name;
    (void)param;
    (void)f;
    // if(!is_claw_free(g))
    // {
    //     return;
    // }
    // if(ecd_size(g) == -1)
    // {
    //     cnt_no++;
    // }
    if(ecd_size_sat(solver, g) == -1)
    {
        cnt_no++;
    }
}

int main()
{
    std::string file = "graphs/4regular/11_4_3.g6";
    cnt_no = 0;
    auto start = high_resolution_clock::now();
    read_graph6_file<void>(file, process_graph, nullptr);
    std::cout << cnt_no << " " << std::endl;
    auto stop = high_resolution_clock::now();
    duration<double> elapsed = (stop - start);
    std::cout << elapsed.count() << std::endl;
}