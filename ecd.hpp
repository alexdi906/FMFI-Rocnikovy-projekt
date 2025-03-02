#ifndef BA_GRAPH_INVARIANTS_ECD_HPP
#define BA_GRAPH_INVARIANTS_ECD_HPP

#include "algorithms/isomorphism/isomorphism.hpp"
#include "invariants/degree.hpp"
#include "invariants/distance.hpp"
#include "operations/basic.hpp"
#include "operations/line_graph.hpp"

#include <algorithm>
#include <climits>
#include <map>
#include <utility>
#include <vector>

namespace ba_graph
{
namespace internal
{
class Ecd
{
   public:
    Ecd(const Graph& g) : g(g), lg(line_graph(g)), edge_to_number(line_graph_with_map(g).second)
    {
        std::vector<Number> nums = lg.list(RP::all(), RT::n());
        uncolored.insert(nums.begin(), nums.end());
        min_ecd_size = INT_MAX;
        has_ecd = false;
        coloring.resize(max_number(lg).to_int() + 1, -1);

        if (g.contains([](const Rotation& r) -> bool { return r.degree() & 1; }) || g.contains(RP::all(), IP::loop()) || (g.size() & 1))
        {
            return;
        }

        startCycle(0);
    }

    // construct each ecd color class based on the minimal ecd size edge coloring
    std::vector<Graph> getEcd(Factory& f = static_factory)
    {
        if (!has_ecd)
        {
            return {};
        }

        std::vector<Graph> subgraphs;
        for (int i = 0; i < getSize(); ++i)
        {
            subgraphs.emplace_back(createG(f));
        }

        for (auto& e : g.list(RP::all(), IP::primary(), IT::e()))
        {
            Graph& sub_g = subgraphs[min_ecd_coloring[edge_to_number[e].to_int()] / 2];

            if (!sub_g.contains(RP::v(e.v1())))
            {
                addV(sub_g, e.v1(), g.find(RP::v(e.v1()))->n(), f);
            }
            if (!sub_g.contains(RP::v(e.v2())))
            {
                addV(sub_g, e.v2(), g.find(RP::v(e.v2()))->n(), f);
            }
            addE(sub_g, e, f);
        }

        return subgraphs;
    }

    int getSize() const
    {
        return !has_ecd ? -1 : min_ecd_size;
    }

   protected:
    const Graph& g;
    const Graph lg;  // for simplicity, we will be assigning vertices of a line graph to cycles
    int min_ecd_size;
    bool has_ecd;
    std::vector<int> coloring;  // to which color class does vertex belong, color class c consists of vertex colors 2*c, 2*c+1
    std::vector<int> min_ecd_coloring;
    std::set<Number> uncolored;
    std::map<Edge, Number> edge_to_number;  // conversion from line graph

    // try to assign vertex to a cycle of color class col/2
    void assignCol(Number vert, int col, int cur_size)
    {
        coloring[vert.to_int()] = col;
        uncolored.erase(vert);

        findCycle(vert, col, cur_size);

        coloring[vert.to_int()] = -1;
        uncolored.insert(vert);
    }

    // find an even cycle using colors col, col+1 alternately. The cycle will belong to color class col/2
    void findCycle(Number cur_vert, int col, int cur_size)
    {
        int oth_col = (col & 1 ? col - 1 : col + 1);
        int cnt_oth_col = 0;

        for (auto& i : lg[cur_vert])
        {
            Number neigh = i.n2();

            if (coloring[neigh.to_int()] == -1)
            {
                continue;
            }
            // in an even cycle both my neighbors have to be of different parity
            if (coloring[neigh.to_int()] == col)
            {
                return;
            }
            if (coloring[neigh.to_int()] == oth_col)
            {
                cnt_oth_col++;
                // exactly 2 of my neighbors have to be of different parity
                if (cnt_oth_col > 2)
                {
                    return;
                }
            }
        }
        // found a good even cycle
        if (cnt_oth_col == 2)
        {
            startCycle(cur_size);
            return;
        }

        for (auto& i : lg[cur_vert])
        {
            Number neigh = i.n2();
            if (coloring[neigh.to_int()] == -1)
            {
                assignCol(neigh, oth_col, cur_size);
            }
        }
    }

    void startCycle(int cur_size)
    {
        if (uncolored.empty())
        {
            min_ecd_size = cur_size;
            min_ecd_coloring = coloring;
            has_ecd = true;
            return;
        }

        Number start_vert = *uncolored.begin();

        // try to assign vertex to some existing color class
        for (int c = 0; c < cur_size; ++c)
        {
            assignCol(start_vert, 2 * c, cur_size);
        }

        // assign to a new color class
        cur_size++;
        if (cur_size >= min_ecd_size)
        {
            return;
        }

        assignCol(start_vert, 2 * (cur_size - 1), cur_size);
    }
};
}  // namespace internal

// minimal size of the Ecd, if there is none, return -1
inline int ecd_size(const Graph& g)
{
    internal::Ecd ecd(g);

    return ecd.getSize();
}

// get the subgraphs which make up the ecd, length of the vector is number of
// color classes. If no ecd, returns {}
inline std::vector<Graph> ecd_subgraphs(const Graph& g, Factory& f = static_factory)
{
    internal::Ecd ecd(g);

    return ecd.getEcd(f);
}

// check whether we have an ecd decomposition of the given graph
inline bool is_ecd(const Graph& g, std::vector<Graph>& subgraphs)
{
    std::vector<Vertex> vert_set;
    std::vector<Edge> edge_set;

    for (auto& sub_g : subgraphs)
    {
        if (min_deg(sub_g) != 2 || max_deg(sub_g) != 2)
        {
            return false;
        }
        auto comps = components(sub_g);

        for (auto& comp : comps)
        {
            if (comp.size() & 1)
            {
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

    if (std::adjacent_find(edge_set.begin(), edge_set.end()) != edge_set.end())
    {
        return false;
    }

    vert_set.erase(std::unique(vert_set.begin(), vert_set.end()), vert_set.end());

    Graph g_2(createG());
    addMultipleV(g_2, vert_set);
    addMultipleE(g_2, edge_set);

    return are_isomorphic(g, g_2);
}
}  // namespace ba_graph
#endif
