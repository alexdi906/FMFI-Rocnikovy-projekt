#include <impl/basic/include.hpp>
#include <sat/solver_cmsat.hpp>
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

#include "ecd.hpp"
#include "ecd_sat.hpp"

using namespace ba_graph;

CMSatSolver solver;

void testEcd(Graph &g, int size) {
#ifdef BACKTR
    assert(ecd_size(g) == size);

    Factory f;
    std::vector<Graph> sub_g = ecd_subgraphs(g, f);
    assert(is_ecd(g, sub_g));

#endif
#ifdef SAT
    assert(ecd_size_sat(solver,g) == size);
#endif
}

int main() {
    Graph g(createG());

    Vertex v1 = createV();
    Vertex v2 = createV();
    Edge e1 = createE(v1, v2);
    addMultipleV(g, {v1, v2});
    addMultipleE(g, {e1, createE(v1, v2)});
    std::vector<Graph> sub_g;

    sub_g.emplace_back(createG());
    addMultipleV(sub_g[0], {v1, v2});
    addMultipleE(sub_g[0], {e1, createE(v1, v2)});
    assert(is_ecd(g, sub_g));

    addMultipleE(g, {createE(v1, v2), createE(v1, v2)});
    sub_g.emplace_back(createG());
    addMultipleV(sub_g[1], {v1, v2});
    addMultipleE(sub_g[1], {e1, createE(v1, v2)});
    assert(!is_ecd(g, sub_g));  // one edge cannot be used in multiple subgraphs

    deleteE(sub_g[1], e1);
    addE(sub_g[1], createE(v1, v2));
    assert(is_ecd(g, sub_g));

    Edge e2 = createE(v1, v2);
    Edge e3 = createE(v1, v2);
    addMultipleE(g, {e2, e3});
    assert(!is_ecd(g, sub_g));

    addMultipleE(sub_g[1], {e2, e3});
    assert(!is_ecd(g, sub_g));

    deleteE(sub_g[1], e2);
    deleteE(sub_g[1], e3);
    sub_g.emplace_back(createG());
    addMultipleV(sub_g[2], {v1, v2});
    addMultipleE(sub_g[2], {e2, e3});
    assert(is_ecd(g, sub_g));

    deleteE(g, e2);
    deleteE(g, e3);
    assert(!is_ecd(g, sub_g));

    g = circuit(2);
    sub_g.clear();
    sub_g.emplace_back(circuit(4));
    assert(!is_ecd(g, sub_g));

    g = circuit(3);
    sub_g[0] = circuit(3);
    assert(!is_ecd(g, sub_g));

    g = circuit(1);
    sub_g[0] = circuit(1);
    assert(!is_ecd(g, sub_g));

    g = empty_graph(0);
    testEcd(g, 0);

    g = circuit(2);
    testEcd(g, 1);
    for (int i = 4; i <= 8; i += 2) {
        g = empty_graph(0);

        for (int j = 1; j <= 2; ++j) {
            Graph add(circuit(i));
            add_graph<NumberMapper>(g, add, g.order());
            testEcd(g, 1);
        }
    }

    g = empty_graph(2);
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 2; ++j) {
            addE(g, Location(0, 1));
        }

        testEcd(g, i + 1);
    }

    g = empty_graph(5);
    for (int i = 0; i < 4; ++i) {
        addE(g, Location(i, i + 1));
        addE(g, Location(i, i + 1));
    }
    testEcd(g, 2);

    auto make_g_2C3 = []() {
      Graph g_2C3(empty_graph(3));
      for (int i = 0; i < 3; ++i) {
          addMultipleE(g_2C3, {Location(i, (i + 1) % 3), Location(i, (i + 1) % 3)});
      }

      return g_2C3;
    };
    g = make_g_2C3();
    testEcd(g, 3);
    Graph add(make_g_2C3());
    add_graph<NumberMapper>(g, add, g.order());
    testEcd(g, 3);

    g = empty_graph(1);
    addE(g, Location(0, 0));
    testEcd(g, -1);

    g = circuit(4);
    addE(g, Location(0, 0));
    testEcd(g, -1);

    g = complete_graph(3);
    testEcd(g, -1);

    g = complete_graph(4);
    testEcd(g, -1);

    g = circuit(5);
    testEcd(g, -1);

    g = path(4);
    testEcd(g, -1);

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

    for (int i = 1; i <= 3; ++i) {
        g = create_macajova_mazak(i);
        testEcd(g, -1);
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
    testEcd(g, -1);

    const std::string path = "../../resources/graphs/4regular/";

//    // 4 regular graphs with chromatic index 4 have ecd = 2
//    for (int o = 6; o <= 12; o += 2) {
//        std::string file = path + "chromatic_index_4/" + (o < 10 ? "0" : "") +
//            std::to_string(o) + "_4_3_chi4.g6";
//
//        Factory f;
//        auto graphs = read_graph6_file(file, f, 0, 10).graphs();
//
//        for (auto &G : graphs) {
//            testEcd(G, 2);
//        }
//    }
//
//    // line graphs of cubic graphs with chromatic index 3 must have ecd
//    // all these graphs have chi = 3 according to invariants/test_colouring
//    for (int i = 4; i <= 8; i += 2) {
//        std::string filename =
//            "../../resources/graphs/" + internal::stored_cubic_path(1, 3, i);
//        Factory f;
//        auto graphs = read_graph6_file(filename, f, 0, 5).graphs();
//        for (auto &G : graphs) {
//            Graph lg(line_graph(G, f));
//            assert(ecd_size(lg) != -1);
//        }
//    }
//
}
