#include <impl/basic/include.hpp>

#include "sat/solver_cmsat.hpp"
#include "ecd.hpp"
#include "ecd_sat.hpp"
#include "io/graph6.hpp"
#include "util/cxxopts.hpp"

// Cycle decomposition, is a partition of E(g) into edge-disjoint cycles.
// Cycle decomposition is called even, if each cycle of the cycle decomposition
// is an even length cycle. For a cycle decomposition, we color each cycle of
// the decomposition so that the cycles sharing a vertex do not receive the same
// color. Thus, the union of cycles in each color class is a 2-regular subgraph.
// If the minimum number of colors required for such a coloring is m, then the
// cycle decomposition is said to be of size m.
// Source: https://doi.org/10.1016/j.disc.2023.113844

using namespace ba_graph;

cxxopts::Options options("ecd",
                         "\nDetermine the sizes of ecd (or -1 if doesn't exist) of graphs from a given file. "
                         "Results are printed to stdout\n");
bool use_line_graph;
std::string algorithm;
CMSatSolver solver;

void process_graph(std::string& file_name, Graph& g, Factory& f, void* param)
{
    (void)file_name;
    (void)param;
    (void)f;

    int res;
    Graph lg = line_graph(g);
    Graph& used_g = (use_line_graph ? lg : g);
    if(algorithm == "backtracking")
    {
        res = ecd_size(used_g);
    }
    else if(algorithm == "sat")
    {
        res = ecd_size_sat(solver, used_g);
    }
    else
    {
        std::cerr << "wrong algorithm: " << algorithm << std::endl;
        exit(1);
    }

    std::cout << res << std::endl;
}

void wrong_usage()
{
    std::cout << options.help() << std::endl;
    exit(1);
}
int main(int argc, char** argv)
{
    try
    {
        options.add_options()("h, help", "print help")("i,input-graph-file", "graph file to the ecd of", cxxopts::value<std::string>())(
          "l,linegraph", "whether to the ecd of the line graph", cxxopts::value<bool>()->default_value("false"))(
          "a, algorithm-used", "which algorithm to use to find ecd (backtracking/sat)", cxxopts::value<std::string>()->default_value("sat"));

        options.parse_positional({"i"});
        options.positional_help("<input graph file>");
        auto result = options.parse(argc, argv);

        if(result.count("help"))
        {
            std::cout << options.help() << std::endl;
            return 0;
        }

        std::string file;
        if(result.count("i") != 1)
        {
            std::cerr << "missing or too many inputs graphs" << std::endl;
            wrong_usage();
        }
        else
        {
            file = result["i"].as<std::string>();
        }

        algorithm = result["a"].as<std::string>();
        use_line_graph = result["l"].as<bool>();
        read_graph6_file<void>(file, process_graph, nullptr);
    }
    catch(const cxxopts::exceptions::exception& e)
    {
        std::cerr << "error parsing option:" << e.what() << std::endl;
        exit(1);
    }
}