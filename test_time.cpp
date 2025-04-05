#include <chrono>
#include <impl/basic/include.hpp>
#include <invariants/colouring.hpp>

#include "sat/solver_cmsat.hpp"
#include "ecd.hpp"
#include "ecd_sat.hpp"
#include "io/graph6.hpp"
#include "util/cxxopts.hpp"
#include "algorithms/isomorphism/isomorphism.hpp"

#include <graphs/snarks.hpp>
using namespace std::chrono;
using namespace ba_graph;

CMSatSolver solver;
int cnt_no;

void process_graph(std::string& file_name, Graph& g, Factory& f, void* param)
{
    (void)file_name;
    (void)param;
    (void)f;
    Graph lg = line_graph(g);
    if(ecd_size_sat(solver, lg) == -1)
    {
        write_graph6_stream(g, std::cerr);
        cnt_no++;
    }
}

int main()
{
    // int ind;
    // std::cin >> ind;
    // auto graphs = read_graph6_file("../no_ECD/17_4_3.clawfree.g6.C_NO").graphs();
    // Graph&g = graphs[ind];
    // std::ofstream cnf_file("cnf.txt");
    // cnf_file << cnf_dimacs(internal::cnf_ecd(g,g.size()/4));
    // cnf_file.close();
    // std::ofstream brk_file("brk.txt");
    // brk_file << cnf_dimacs(preprocess_breakid(internal::cnf_ecd(g,g.size()/4)));
    // brk_file.close();
    auto start = high_resolution_clock::now();

    auto graphs = read_graph6_file("graphs/4regular/13_4_3.g6", static_factory, 0, 500).graphs();
    int cnt_chr = 0;
    int cnt_ecd = 0;
    for(auto& g : graphs)
    {
        if(chromatic_index_basic(g) == 5)
        {
            cnt_chr++;
        }
        if(ecd_size(g) == -1)
        {
            cnt_ecd++;
        }
        // if(ecd_size_sat(solver,g) == -1)
        // {
        //     cnt_ecd++;
        // }
    }
    auto stop = high_resolution_clock::now();
    std::cerr << cnt_chr << " " << cnt_ecd << std::endl;
    duration<double> elapsed = (stop - start);
    std::cerr << elapsed.count() << std::endl;
}