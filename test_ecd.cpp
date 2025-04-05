#include <impl/basic/include.hpp>
#include "sat/solver_cmsat.hpp"
#include "algorithms/isomorphism/isomorphism.hpp"
#include "ecd.hpp"
#include "ecd_sat.hpp"
#include "graphs.hpp"
#include "invariants/colouring.hpp"
#include "invariants/connectivity.hpp"
#include "io/graph6.hpp"
#include "io/print_nice.hpp"
#include "operations/add_graph.hpp"
#include "operations/line_graph.hpp"
#include "graphs/snarks.hpp"

#include <string>
#include <vector>

using namespace ba_graph;

CMSatSolver solver;

void test_ecd(const Graph &g, int size, bool test_equal = true)
{
#ifdef BACKTR
    if(equal)
    {
        assert(ecd_size(g) == size);
    }
    else
    {
        assert(ecd_size(g) != size);
    }

    Factory f;
    std::vector<Graph> subg = ecd_subgraphs(g, f);
    assert(is_ecd(g, subg));

#endif
#ifdef SAT
    if(test_equal)
    {
        assert(ecd_size_sat(solver, g) == size);
    }
    else
    {
        assert(ecd_size_sat(solver, g) != size);
    }
#endif
}

int main()
{
    Graph g(createG());

    Vertex v1 = createV();
    Vertex v2 = createV();
    Edge e1 = createE(v1, v2);
    addMultipleV(g, {v1, v2});
    addMultipleE(g, {e1, createE(v1, v2)});
    std::vector<Graph> subg;

    subg.emplace_back(createG());
    addMultipleV(subg[0], {v1, v2});
    addMultipleE(subg[0], {e1, createE(v1, v2)});
    assert(is_ecd(g, subg));

    addMultipleE(g, {createE(v1, v2), createE(v1, v2)});
    subg.emplace_back(createG());
    addMultipleV(subg[1], {v1, v2});
    addMultipleE(subg[1], {e1, createE(v1, v2)});
    assert(!is_ecd(g, subg));  // one edge cannot be used in multiple subgraphs

    deleteE(subg[1], e1);
    addE(subg[1], createE(v1, v2));
    assert(is_ecd(g, subg));

    Edge e2 = createE(v1, v2);
    Edge e3 = createE(v1, v2);
    addMultipleE(g, {e2, e3});
    assert(!is_ecd(g, subg));

    addMultipleE(subg[1], {e2, e3});
    assert(!is_ecd(g, subg));

    deleteE(subg[1], e2);
    deleteE(subg[1], e3);
    subg.emplace_back(createG());
    addMultipleV(subg[2], {v1, v2});
    addMultipleE(subg[2], {e2, e3});
    assert(is_ecd(g, subg));

    deleteE(g, e2);
    deleteE(g, e3);
    assert(!is_ecd(g, subg));

    g = circuit(2);
    subg.clear();
    subg.emplace_back(circuit(4));
    assert(!is_ecd(g, subg));

    g = circuit(3);
    subg[0] = circuit(3);
    assert(!is_ecd(g, subg));

    g = circuit(1);
    subg[0] = circuit(1);
    assert(!is_ecd(g, subg));

    g = empty_graph(0);
    test_ecd(g, 0);

    g = circuit(2);
    test_ecd(g, 1);
    for(int i = 4; i <= 8; i += 2)
    {
        g = empty_graph(0);

        for(int j = 1; j <= 2; ++j)
        {
            Graph add(circuit(i));
            add_graph<NumberMapper>(g, add, g.order());
            test_ecd(g, 1);
        }
    }

    g = empty_graph(2);
    for(int i = 0; i < 3; ++i)
    {
        for(int j = 0; j < 2; ++j)
        {
            addE(g, Location(0, 1));
        }

        test_ecd(g, i + 1);
    }

    g = empty_graph(5);
    for(int i = 0; i < 4; ++i)
    {
        addE(g, Location(i, i + 1));
        addE(g, Location(i, i + 1));
    }
    test_ecd(g, 2);

    auto make_g2C3 = []()
    {
        Graph g_2C3(empty_graph(3));
        for(int i = 0; i < 3; ++i)
        {
            addMultipleE(g_2C3, {Location(i, (i + 1) % 3), Location(i, (i + 1) % 3)});
        }

        return g_2C3;
    };
    g = make_g2C3();
    test_ecd(g, 3);
    Graph add(make_g2C3());
    add_graph<NumberMapper>(g, add, g.order());
    test_ecd(g, 3);

    g = empty_graph(1);
    addE(g, Location(0, 0));
    test_ecd(g, -1);

    g = circuit(4);
    addE(g, Location(0, 0));
    test_ecd(g, -1);

    g = complete_graph(3);
    test_ecd(g, -1);

    g = complete_graph(4);
    test_ecd(g, -1);

    g = circuit(5);
    test_ecd(g, -1);

    g = path(4);
    test_ecd(g, -1);

    // https://doi.org/10.1016/j.disc.2013.04.027
    auto create_macajova_mazak = [](int size)
    {
        Graph g(empty_graph(1));

        for(int i = 0; i < size; ++i)
        {
            Graph add(complete_graph(4));
            add_graph<NumberMapper>(g, add, g.order());

            addE(g, Location(4 * i + 1, 4 * i));
            addE(g, Location(4 * i + 2, std::max(4 * i - 1, 0)));
        }

        addE(g, Location(0, 4 * size));
        addE(g, Location(0, 4 * size - 1));

        assert(min_deg(g) == 4 && max_deg(g) == 4);
        assert(vertex_connectivity(g) == 3);
        if(size == 1)
        {
            Graph k_5(complete_graph(5));
            assert(are_isomorphic(g, k_5));
        }

        return g;
    };

    for(int i = 1; i <= 3; ++i)
    {
        g = create_macajova_mazak(i);
        test_ecd(g, -1);
    }

    // https://doi.org/10.1016/j.disc.2011.12.007
    auto create_markstrom = []()
    {
        Graph g(empty_graph(9));

        for(int i = 0; i < 5; ++i)
        {
            for(int j = i + 1; j < 5; ++j)
            {
                addE(g, Location(i, j));
            }
        }

        for(int i = 4; i < 9; ++i)
        {
            for(int j = i + 1; j < 9; ++j)
            {
                addE(g, Location(i, j));
            }
        }

        for(int i = 2; i <= 3; ++i)
        {
            int v = i + 3;

            deleteE(g, Location(4, i));
            deleteE(g, Location(4, v));

            addE(g, Location(v, i));
        }

        assert(vertex_connectivity(g) == 3);
        assert(min_deg(g) == 4 && max_deg(g) == 4);

        return g;
    };
    g = create_markstrom();
    test_ecd(g, -1);

    // 4 regular graphs with chromatic index 4 have ecd = 2
    for(int i = 6; i <= 10; i += 2)
    {
        std::string file = std::string("graphs/4regular/chromatic_index_4/") + (i < 10 ? "0" : "") + std::to_string(i) + "_4_3_chi4.g6";

        auto graphs = read_graph6_file(file).graphs();

        for(auto &G : graphs)
        {
            assert(chromatic_index_basic(G) == 4);
            test_ecd(G, 2);
        }
    }

    Graph lg = line_graph(create_petersen());
    // 2 connected graphs with oddness = 2 must have ecd
    test_ecd(lg, -1, false);
    // line graphs of cubic graphs with chromatic index 3 must have ecd
    for(int i = 6; i <= 10; i += 2)
    {
        std::string file = std::string("graphs/3regular/chromatic_index_3/") + (i < 10 ? "0" : "") + std::to_string(i) + "_3_3_chi3.g6";

        auto graphs = read_graph6_file(file).graphs();

        for(auto &G : graphs)
        {
            assert(chromatic_index_basic(G) == 3);
            lg = line_graph(G);
            test_ecd(lg, -1, false);
        }
    }
}