#ifndef BA_GRAPH_INVARIANTS_ECD_HPP
#define BA_GRAPH_INVARIANTS_ECD_HPP

#include <algorithm>
#include <climits>
#include <map>
#include <utility>
#include <vector>

#include "../basic_impl.hpp"
#include "../algorithms/isomorphism/isomorphism.hpp"
#include "../operations/basic.hpp"
#include "../operations/line_graph.hpp"
#include "degree.hpp"
#include "distance.hpp"

namespace ba_graph {
namespace internal {
// Cycle decomposition, is a partition of E(g) into edge-disjoint cycles.
// Cycle decomposition is called even, if each cycle of the cycle decomposition
// is an even length cycle. For a cycle decomposition, we color each cycle of
// the decomposition so that the cycles sharing a vertex do not receive the same
// color. Thus, the union of cycles in each color class is a 2-regular subgraph.
// If the minimum number of colors required for such a coloring is m, then the
// cycle decomposition is said to be of size m.
// Source: https://doi.org/10.1016/j.disc.2023.113844
class Ecd {
 public:
    explicit Ecd(const Graph &g) : g_(g), lg_(line_graph(g)), edge_to_number_(line_graph_with_map(g).second) {
        std::vector<Number> nums = lg_.list(RP::all(), RT::n());
        uncolored_.insert(nums.begin(), nums.end());
        min_ecd_size_ = INT_MAX;
        has_ecd_ = false;
        coloring_.resize(max_number(lg_).to_int() + 1, -1);

        if (g.contains([](const Rotation &r) -> bool { return r.degree() & 1; }) ||
            g.contains(RP::all(), IP::loop()) || (g.size() & 1)) {
            return;
        }

        StartCycle(0);
    }

    // construct each ecd color class based on the minimal ecd size edge coloring
    std::vector<Graph> GetEcd(Factory &f = static_factory) {
        if (!has_ecd_) {
            return {};
        }

        std::vector<Graph> subgraphs;
        for (int i = 0; i < GetSize(); ++i) {
            subgraphs.emplace_back(createG(f));
        }

        for (auto &e : g_.list(RP::all(), IP::primary(), IT::e())) {
            Graph &g = subgraphs[min_ecd_coloring_[edge_to_number_[e].to_int()] / 2];

            if (!g.contains(RP::v(e.v1()))) {
                addV(g, e.v1(), g_.find(RP::v(e.v1()))->n(), f);
            }
            if (!g.contains(RP::v(e.v2()))) {
                addV(g, e.v2(), g_.find(RP::v(e.v2()))->n(), f);
            }
            addE(g, e, f);
        }

        return subgraphs;
    }

    int GetSize() const { return !has_ecd_ ? -1 : min_ecd_size_; }

 protected:
    const Graph &g_;
    const Graph lg_; // for simplicity, we will be assigning vertices of a line
    // graph to cycles
    int min_ecd_size_;
    bool has_ecd_;
    std::vector<int> coloring_; // to which color class does vertex belong, color class c
    // consists of vertex colors 2*c, 2*c+1
    std::vector<int> min_ecd_coloring_;
    std::set<Number> uncolored_;
    std::map<Edge, Number> edge_to_number_; // conversion from line graph

    // try to assign vertex to a cycle of color class col/2
    void AssignCol(Number vert, int col, int cur_size) {
        coloring_[vert.to_int()] = col;
        uncolored_.erase(vert);

        FindCycle(vert, col, cur_size);

        coloring_[vert.to_int()] = -1;
        uncolored_.insert(vert);
    }

    // find an even cycle using colors col, col+1
    // alternately. The cycle will belong to color class col/2
    void FindCycle(Number cur_vert, int col, int cur_size) {
        int oth_col = (col & 1 ? col - 1 : col + 1);
        int cnt_oth_col = 0;

        for (auto &i : lg_[cur_vert]) {
            Number neigh = i.n2();

            if (coloring_[neigh.to_int()] == -1) {
                continue;
            }
            // in an even cycle both my neighbors have to be of different parity
            if (coloring_[neigh.to_int()] == col) {
                return;
            }
            if (coloring_[neigh.to_int()] == oth_col) {
                cnt_oth_col++;
                // exactly 2 of my neighbors have to be of different parity
                if (cnt_oth_col > 2) {
                    return;
                }
            }
        }
        // found a good even cycle
        if (cnt_oth_col == 2) {
            StartCycle(cur_size);
            return;
        }

        for (auto &i : lg_[cur_vert]) {
            Number neigh = i.n2();
            if (coloring_[neigh.to_int()] == -1) {
                AssignCol(neigh, oth_col, cur_size);
            }
        }
    }

    void StartCycle(int cur_size) {

        if (uncolored_.empty()) {

            min_ecd_size_ = cur_size;
            min_ecd_coloring_ = coloring_;
            has_ecd_ = true;
            return;
        }

        Number start_vert = *uncolored_.begin();

        // try to assign vertex to some existing color class
        for (int c = 0; c < cur_size; ++c) {
            AssignCol(start_vert, 2 * c, cur_size);
        }

        // assign to a new color class
        cur_size++;
        if (cur_size >= min_ecd_size_) {
            return;
        }

        AssignCol(start_vert, 2 * (cur_size - 1), cur_size);
    }
};
} // namespace internal

// minimal size of the Ecd, if there is none, return -1
inline int ecd_size(const Graph &g) {
    internal::Ecd ecd(g);

    return ecd.GetSize();
}

// get the subgraphs which make up the ecd, length of the vector is number of
// color classes. If no ecd, returns {}
inline std::vector<Graph> ecd_subgraphs(const Graph &g,
                                        Factory &f = static_factory) {
    internal::Ecd ecd(g);

    return ecd.GetEcd(f);
}

// check whether we have an ecd decomposition of the given graph
inline bool is_ecd(const Graph &g, std::vector<Graph> &subgraphs) {
    std::vector<Vertex> vert_set;
    std::vector<Edge> edge_set;

    for (auto &sub_g : subgraphs) {
        if (min_deg(sub_g) != 2 || max_deg(sub_g) != 2) {
            return false;
        }
        auto comps = components(sub_g);

        for (auto &comp : comps) {
            if (comp.size() & 1) {
                return false;
            }
        }

        std::vector<Vertex> sub_g_vert = sub_g.list(RP::all(), RT::v());
        std::vector<Edge> sub_g_edg = sub_g.list(RP::all(), IP::primary(), IT::e());

        vert_set.insert(vert_set.begin(), sub_g_vert.begin(), sub_g_vert.end());
        edge_set.insert(edge_set.end(), sub_g_edg.begin(), sub_g_edg.end());
    }

    std::sort(vert_set.begin(), vert_set.end());
    std::sort(edge_set.begin(), edge_set.end());

    if (std::adjacent_find(edge_set.begin(), edge_set.end()) != edge_set.end()) {
        return false;
    }

    vert_set.erase(std::unique(vert_set.begin(), vert_set.end()), vert_set.end());

    Graph g_2(createG());
    addMultipleV(g_2, vert_set);
    addMultipleE(g_2, edge_set);

    return are_isomorphic(g, g_2);
}

} // namespace ba_graph
#endif
