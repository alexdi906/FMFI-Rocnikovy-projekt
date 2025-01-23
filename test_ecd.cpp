#include "implementation.h"
#include <algorithms/isomorphism/isomorphism.hpp>
#include <cassert>
#include <graphs.hpp>
#include <invariants/connectivity.hpp>
#include <invariants/ecd.hpp>
#include <io/graph6.hpp>
#include <io/print_nice.hpp>
#include <iostream>
#include <operations/add_graph.hpp>
#include <operations/line_graph.hpp>
#include <string>
#include <vector>

using namespace ba_graph;

void TestEcd(Graph &g, int size) {
    assert(ecd_size(g) == size);

    Factory f;
    std::vector<Graph> subg = ecd_subgraphs(g, f);
    assert(is_ecd(g, subg));
}

int main() {
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
    TestEcd(g, 0);

    g = circuit(2);
    TestEcd(g, 1);
    for (int i = 4; i <= 8; i += 2) {
        g = empty_graph(0);

        for (int j = 1; j <= 2; ++j) {
            Graph add(circuit(i));
            add_graph<NumberMapper>(g, add, g.order());
            TestEcd(g, 1);
        }
    }

    g = empty_graph(2);
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 2; ++j) {
            addE(g, Location(0, 1));
        }

        TestEcd(g, i + 1);
    }

    g = empty_graph(5);
    for (int i = 0; i < 4; ++i) {
        addE(g, Location(i, i + 1));
        addE(g, Location(i, i + 1));
    }
    TestEcd(g, 2);

    auto make_g_2C3 = []() {
        Graph g_2C3(empty_graph(3));
        for (int i = 0; i < 3; ++i) {
            addMultipleE(g_2C3, {Location(i, (i + 1) % 3), Location(i, (i + 1) % 3)});
        }

        return g_2C3;
    };
    g = make_g_2C3();
    TestEcd(g, 3);
    Graph add(make_g_2C3());
    add_graph<NumberMapper>(g, add, g.order());
    TestEcd(g, 3);

    g = empty_graph(1);
    addE(g, Location(0, 0));
    assert(ecd_size(g) == -1);

    g = circuit(4);
    addE(g, Location(0, 0));
    assert(ecd_size(g) == -1);

    g = complete_graph(3);
    assert(ecd_size(g) == -1);

    g = complete_graph(4);
    assert(ecd_size(g) == -1);

    g = circuit(5);
    assert(ecd_size(g) == -1);

    g = path(4);
    assert(ecd_size(g) == -1);

    // https://doi.org/10.1016/j.disc.2013.04.027
    auto create_macajova_mazak = [](int size) {
        Graph g(empty_graph(1));

        for (int i = 0; i < size; ++i) {
            Graph add(complete_graph(4));
            add_graph<NumberMapper>(g, add, g.order());

            addE(g, Location(4 * i + 1, 4 * i));
            addE(g, Location(4 * i + 2, std::max(4 * i - 1, 0)));
        }

        addE(g, Location(0, 4 * size));
        addE(g, Location(0, 4 * size - 1));

        assert(min_deg(g) == 4 && max_deg(g) == 4);
        assert(vertex_connectivity(g) == 3);
        if (size == 1) {
            Graph k_5(complete_graph(5));
            assert(are_isomorphic(g, k_5));
        }

        return g;
    };

    for (int i = 1; i <= 2; ++i) {
        g = create_macajova_mazak(i);
        assert(ecd_size(g) == -1);
    }

    // https://doi.org/10.1016/j.disc.2011.12.007
    auto create_markstrom = []() {
        Graph g(empty_graph(9));

        for (int i = 0; i < 5; ++i) {
            for (int j = i + 1; j < 5; ++j) {
                addE(g, Location(i, j));
            }
        }

        for (int i = 4; i < 9; ++i) {
            for (int j = i + 1; j < 9; ++j) {
                addE(g, Location(i, j));
            }
        }

        for (int i = 2; i <= 3; ++i) {
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
    assert(ecd_size(g) == -1);

    const std::string kPath = "../ba-graph/resources/graphs/4regular/";

    // 4 regular graphs with chromatic index 4 have ecd = 2
    for (int o = 6; o <= 12; o += 2) {
        std::string file = kPath + "chromatic_index_4/" + (o < 10 ? "0" : "") +
            std::to_string(o) + "_4_3_chi4.g6";

        Factory f;
        auto graphs = read_graph6_file(file, f, 0, 10).graphs();

        for (auto &G : graphs) {
            TestEcd(G, 2);
        }
    }


    // line graphs of cubic graphs with chromatic index 3 must have ecd
    // all these graphs have chi = 3 according to invariants/test_colouring
    for (int i = 4; i <= 8; i += 2) {
        std::string filename =
            "../ba-graph/resources/graphs/" + internal::stored_cubic_path(1, 3, i);
        Factory f;
        auto graphs = read_graph6_file(filename, f, 0, 5).graphs();
        for (auto &G : graphs) {
            Graph lg(line_graph(G, f));
            assert(ecd_size(lg) != -1);
        }
    }

    //graphs with no ecd
    const std::vector<int> kOrdersClawfree = {10, 13};

    for (int o : kOrdersClawfree) {
        std::string file = kPath + "no_ECD/" + (o < 10 ? "0" : "") +
            std::to_string(o) + "_4_3.clawfree.g6.C_NO";

        Factory f;
        auto graphs = read_graph6_file(file, f).graphs();

        for (auto &G : graphs) {
            assert(ecd_size(G) == -1);
        }
    }

    const std::vector<int> kOrdersClaw = {5, 9};
    for (int o : kOrdersClaw) {
        std::string file = kPath + "no_ECD/" + (o < 10 ? "0" : "") +
            std::to_string(o) + "_4_3.3c.g6.C_NO";

        Factory f;
        auto graphs = read_graph6_file(file, f).graphs();

        for (auto &G : graphs) {
            assert(ecd_size(G) == -1);
        }
    }
}
