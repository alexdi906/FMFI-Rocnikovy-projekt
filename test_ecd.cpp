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

enum TestType
{
    Equal,
    Nequal,
    Leq
};
void test_ecd(const Graph &g, int size, TestType type = Equal)
{
#ifdef BACKTR
    if(test_equality)
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
    switch(type)
    {
        case Equal:
            assert(ecd_size_sat(solver, g) == size);
            break;
        case Nequal:
            assert(ecd_size_sat(solver, g) != size);
            break;
        case Leq:
            assert(ecd_size_sat(solver, g) <= size);
            break;
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

    auto multiply_edge = [](const Graph &g, Location e, int m)
    {
        Graph new_g(createG());
        add_graph(new_g, g, 0);
        for(int i = 0; i < m; ++i)
        {
            addE(new_g, e);
        }
        return new_g;
    };

    // eulerian bipartite graphs have an ecd
    for(int i = 2; i <= 4; i += 2)
    {
        for(int j = 2; j <= i; j += 2)
        {
            g = complete_bipartite_graph(i, j);
            test_ecd(g, -1, Nequal);

            // multiplying an edge by an even number preserves ecd
            assert(g.contains(Location(0, i)));
            test_ecd(multiply_edge(g, Location(0, i), 2), -1, Nequal);
        }
    }

    for(int m = 2; m <= 4; m += 2)
    {
        // multiplying an edge even number of times in K_5 creates an ecd
        test_ecd(multiply_edge(complete_graph(5), Location(0, 1), m), -1, Nequal);
    }

    // testing operations from https://arxiv.org/abs/1209.0160v4

    auto join_graphs = [](const Graph &g, const Graph &h)
    {
        Graph join_g(createG());
        add_graph(join_g, g, 0);
        auto num_g = join_g.list(RP::all(), RT::n());
        auto num_h = h.list(RP::all(), RT::n());

        int offset = join_g.order();

        add_disjoint(join_g, h, offset);
        for(auto n1 : num_g)
        {
            for(auto n2 : num_h)
            {
                addE(join_g, Location(n1, n2 + offset));
            }
        }
        return join_g;
    };
    auto create_co_claw = []()
    {
        Graph co_claw(circuit(3));
        addV(co_claw);
        return co_claw;
    };
    // Let G1 be a simple Eulerian graph (not necessarily connected) with |V(G1)| ≥ 3 and |V(G1)|
    // odd. Let G2 be a simple anti-Eulerian graph with |V(G2)| ≥ 2. Then the join of G1
    // and G2 is strongly even-cycle decomposable if and only if G1 != K3 or G2 != K2 .
    test_ecd(join_graphs(circuit(3), complete_graph(2)), -1, Equal);  // K_5
    test_ecd(join_graphs(circuit(5), complete_graph(2)), -1, Nequal);

    //     Let G1 and G2 be simple Eulerian graphs (can have more components) with an even number of
    // vertices. Then the join of G1 and G2 is strongly even-cycle decomposable if and
    // only if it is not K5 with an edge subdivided.
    test_ecd(join_graphs(create_co_claw(), empty_graph(2)), -1, Equal);  // K_5 with edge subdivided
    test_ecd(join_graphs(circuit(4), empty_graph(2)), -1, Nequal);
    test_ecd(join_graphs(circuit(4), circuit(2)), -1, Nequal);
    test_ecd(join_graphs(create_co_claw(), create_co_claw()), -1, Nequal);

    auto substitute_graph = [](const Graph &g, const Graph &h)
    {
        Graph new_g(createG());
        add_graph(new_g, g, 0);

        int offset = new_g.order();
        Number v = max_deg_vertex(new_g);
        auto neighb = new_g[v].neighbours();
        deleteV(new_g, v);

        auto num_h = h.list(RP::all(), RT::n());
        add_disjoint(new_g, h, offset);

        for(auto n1 : neighb)
        {
            for(auto n2 : num_h)
            {
                addE(new_g, Location(n1, n2 + offset));
            }
        }
        assert(!(new_g.size() & 1));
        return new_g;
    };

    //  Let G be a simple strongly even-cycle decomposable graph, let v
    // be a non-isolated vertex of G, and let H be a simple Eulerian graph with an odd
    // number of vertices. Then the substitution of v by H in G is strongly even-cycle
    // decomposable, provided that H is not K_3 or degG(v) ≥ 4.
    test_ecd(substitute_graph(circuit(3), circuit(3)), -1, Equal);  // K_5
    test_ecd(substitute_graph(circuit(3), circuit(5)), -1, Nequal);
    test_ecd(substitute_graph(join_graphs(create_co_claw(), empty_graph(4)), circuit(3)), -1, Nequal);

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

    // 2d-regular graph has an even cycle decomposition of index d if and only if it is class 1.
    for(int i = 6; i <= 10; ++i)
    {
        std::string file = std::string("graphs/4regular/") + (i < 10 ? "0" : "") + std::to_string(i) + "_4_3.g6";

        auto graphs = read_graph6_file(file).graphs();

        for(auto &G : graphs)
        {
            if(chromatic_index_basic(G) == 4)
            {
                test_ecd(G, 2);
            }
            else
            {
                test_ecd(G, 2, Nequal);
            }
        }
    }

    for(int i = 7; i <= 8; ++i)
    {
        std::string file = std::string("graphs/6regular/") + (i < 10 ? "0" : "") + std::to_string(i) + "_6_3.g6";

        auto graphs = read_graph6_file(file).graphs();

        for(auto &G : graphs)
        {
            if(chromatic_index_basic(G) == 6)
            {
                test_ecd(G, 3);
            }
            else
            {
                test_ecd(G, 3, Nequal);
            }
        }
    }

    test_ecd(line_graph(create_petersen()), 3);
    // line graphs of cubic graphs with chromatic index 3 must have ecd size <=3
    for(int i = 6; i <= 10; i += 2)
    {
        std::string file = std::string("graphs/3regular/chromatic_index_3/") + (i < 10 ? "0" : "") + std::to_string(i) + "_3_3_chi3.g6";

        auto graphs = read_graph6_file(file).graphs();

        for(auto &G : graphs)
        {
            assert(chromatic_index_basic(G) == 3);
            if(!(G.size() & 1))
            {
                test_ecd(line_graph(G), 2, Equal);
            }
            else
            {
                test_ecd(line_graph(G), 3, Leq);
            }
        }
    }
}