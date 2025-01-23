#ifndef BA_GRAPH_INVARIANTS_ECD_HPP
#define BA_GRAPH_INVARIANTS_ECD_HPP

#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <climits>
#include <utility>

#include "../operations/line_graph.hpp"
#include "distance.hpp"
#include "degree.hpp"
#include "../algorithms/isomorphism/isomorphism.hpp"

namespace ba_graph
{
namespace internal
{
//Source: https://doi.org/10.1016/j.disc.2023.113844
//Cycle decomposition, is a partition of E(G) into edge-disjoint cycles.
//Cycle decomposition is called even, if each cycle of the cycle decomposition is an even length cycle.
//For a cycle decomposition, we color each cycle of the decomposition so that the cycles sharing a vertex do not receive
//the same color. Thus, the union of cycles in each color class is a 2-regular subgraph. If the minimum number of colors
//required for such a coloring is m, then the cycle decomposition is said to be of size m.
class ECD
{
    public:
        explicit ECD(const Graph&G): G(G), LG(std::move(line_graph(G)))
        {
            edgeToNumber  = line_graph_with_map(G).second;
            nums = LG.list(RP::all(), RT::n());
            uncolored.insert(nums.begin(), nums.end());
            minSize = INT_MAX;
            hasEcd = false;

            if (G.contains([](const Rotation& r)-> bool { return r.degree() & 1; }) || G.contains(RP::all(), IP::loop()) || (G.size()&1))
            {
                return;
            }

            startCycle(0);
            assert(uncolored.size() == nums.size());
            assert(coloring.size() == 0);
            coloring = minColoring;
        }

        //construct each ecd color class based on the minimal size edge coloring
        std::vector<Graph> getECD(Factory&f = static_factory)
        {
            if(!hasEcd)
            {
                return {};
            }

            std::vector<Graph> subgraphs;
            for(int i = 0; i<getSize(); ++i)
            {
                subgraphs.emplace_back(std::move(createG(f)));
            }

            for (auto&e : G.list(RP::all(), IP::primary(), IT::e()))
            {
                Graph&g = subgraphs[coloring[edgeToNumber[e]]/2];
                if(!g.contains(RP::v(e.v1())))
                {
                    addV(g,e.v1(), G.find(RP::v(e.v1()))->n(),f);
                }
                if(!g.contains(RP::v(e.v2())))
                {
                    addV(g,e.v2(), G.find(RP::v(e.v2()))->n(),f);
                }
                addE(g,e,f);
            }

            return subgraphs;
        }

        int getSize() const
        {
            return !hasEcd?-1:minSize;
        }

    protected:
        const Graph&G;
        const Graph LG; // for simplicity, we will be assigning vertices of a line graph to cycles
        int minSize; // minimum ecd size
        bool hasEcd;
        std::map<Number, int> minColoring; //to which color class does vertex belong, color class i consists of vertex colors 2*i, 2*i+1
        std::map<Number, int> coloring;
        //std::map<Number, std::set<int>> saturation;
        std::set<Number> uncolored;
        std::map<Edge, Number> edgeToNumber; // conversion from line graph
        std::vector<Number> nums;


        Number maxSaturVert(std::set<Number>&candidates)
        {

            std::map<Number, std::set<int>> saturation;

            std::vector<Number> changed;
            std::set<std::pair<int,Number>> order;

            for(auto&n : candidates)
            {
                for(auto&I : LG[n])
                {
                    Number neigh = I.n2();

                    if(!coloring.count(neigh))
                    {
                        continue;
                    }
                    int col = coloring[neigh];

                    if(!saturation[n].contains(col))
                    {
                        saturation[n].insert(col);
                        changed.push_back(neigh);
                    }
                }

                order.insert(std::make_pair(saturation[n].size(), n));
            }

            return std::prev(order.end())->second;
        }

        //try to assign vertex to cycle of color class col/2
        void assignCol(Number vert, int col, Number startVert, int curSize)
        {
            coloring[vert] = col;
            uncolored.erase(vert);

            findCycle(vert, col, startVert, curSize);

            coloring.erase(vert);
            uncolored.insert(vert);
        }

        //find an even cycle beginning at startVert, using colors col, col+1 alternately
        //the cycle will belong to color class col/2
           void findCycle(Number curVert, int col, Number startVert, int curSize)
        {

            int othCol = col/2*2 + ((col&1)^1);
            int cntOthCol = 0;

            for(auto&I : LG[curVert])
            {
                Number neigh = I.n2();
                if(!coloring.contains(neigh))
                {
                    continue;
                }
                //in an even cycle both my neighbors have to be of different parity
                if(coloring[neigh] == col)
                {
                    return;
                }
                if(coloring[neigh] == othCol)
                {
                    cntOthCol++;
                    //exactly 2 of my neighbors have to be of different parity
                    if(cntOthCol > 2)
                    {
                        return;
                    }
                }
            }

            //if we returned to start (and we didn't just leave start) we have found an even cycle
            if(cntOthCol == 2)
            {
                startCycle(curSize);
                return;
            }
                std::vector<std::pair<int, Number>> order;

                for(auto&I : LG[curVert])
                {
                    Number neigh = I.n2();
                    if(!coloring.contains(neigh))
                    {

                        assignCol(neigh,othCol, startVert, curSize);
                    }

                }
        }


        void startCycle(int curSize)
        {

            // all vertices have been assigned to cycles
            if(uncolored.empty())
            {
                minSize = curSize;
                minColoring = coloring;
                hasEcd = true;
                return;
            }

            Number startVert = maxSaturVert(uncolored);

            //try to assign vertex to some existing color class
            for(int i = 0; i<curSize; ++i)
            {
                assignCol(startVert, 2*i, startVert,curSize);
            }

            //assign to a new color class
            curSize++;
            if(curSize >= minSize)
            {
                return;
            }

            assignCol(startVert, 2*(curSize-1), startVert, curSize);
        }

};
}//namespace internal

// size of the ECD, if there is none, return -1
inline int ecd_size(const Graph&G)
{
    internal::ECD ecd(G);

    return ecd.getSize();
}

// get the subgraphs which make up the ecd, length of the vector is number of color classes
// if no ecd, returns {}
inline std::vector<Graph> ecd_subgraphs(const Graph&G, Factory&f = static_factory)
{
    internal::ECD ecd(G);

    return ecd.getECD(f);
}

// check whether we have an ecd decomposition of the given graph
inline bool is_ecd(const Graph&G, std::vector<Graph>&subgraphs)
{
    std::vector<Vertex> vertSet;
    std::vector<Edge> edgeSet;

    for(auto&g : subgraphs)
    {
        if(min_deg(g) != 2 || max_deg(g) != 2)
        {
            return false;
        }
        auto comps = components(g);

        for(auto&comp : comps)
        {
            if(comp.size()&1)
            {
                return false;
            }
        }

        std::vector<Vertex> verts = g.list(RP::all(),RT::v());
        std::vector<Edge> edgs = g.list(RP::all(), IP::primary(), IT::e());

        vertSet.insert(vertSet.begin(), verts.begin(), verts.end());
        edgeSet.insert(edgeSet.end(), edgs.begin(), edgs.end());
    }

    std::sort(vertSet.begin(), vertSet.end());
    std::sort(edgeSet.begin(), edgeSet.end());

    if(std::adjacent_find(edgeSet.begin(), edgeSet.end()) != edgeSet.end())
    {
        return false;
    }

    vertSet.erase(std::unique(vertSet.begin(), vertSet.end()),vertSet.end());

    Graph G2(createG());
    addMultipleV(G2, vertSet);
    addMultipleE(G2, edgeSet);

    return are_isomorphic(G, G2);
}

}//namespace ba_graph
#endif
