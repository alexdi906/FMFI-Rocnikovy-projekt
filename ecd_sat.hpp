#ifndef BA_GRAPH_SAT_CNF_ECD_HPP
#define BA_GRAPH_SAT_CNF_ECD_HPP


#include "impl/basic/include.hpp"
#include "io/print_nice.hpp"
#include "operations/line_graph.hpp"
#include "sat/cnf.hpp"
#include "sat/exec_solver.hpp"
#include "sat/solver.hpp"

#include <map>
#include <utility>
#include <vector>
#include <climits>

namespace ba_graph
{
namespace internal
{
inline bool adjacent(Location a, Location b)
{
    return a.n1() == b.n1() || a.n1() == b.n2() || a.n2() == b.n1() || a.n2() == b.n2();
}

inline bool incident(Number n, Location l)
{
    return adjacent(Location(n, n), l);
}

inline CNF cnf_ecd(const Graph& g, int k)
{
    // vars[e][c] means edge e belongs to the c/2-th color class on a position of c&1 parity
    auto edges = g.list(RP::all(), IP::primary(), IT::l());
    std::map<Location, std::vector<int>> vars;

    std::vector<std::vector<Lit>> cnf;

    int next_var = 0;
    for(auto& l : edges)
    {
        std::vector<int> v(2 * k);
        for(int c = 0; c < 2 * k; ++c)
        {
            v[c] = next_var++;
        }

        vars[l] = v;
    }

    for(int i = 0; i < (int)edges.size(); ++i)
    {
        Location l = edges[i];
        std::vector<Lit> clause;

        if(i != 0)
        {
            // each edge belongs to at least one color class
            for(int c = 0; c < 2 * k; ++c)
            {
                clause.push_back(Lit(vars[l][c], false));
            }
        }
        else
        {
            // first edge belongs to first color class and even position
            if(k != 0)
            {
                clause.push_back(Lit(vars[l][0], false));
            }
        }
        cnf.push_back(clause);

        // each edge belongs to at most one color class
        for(int c1 = 0; c1 < 2 * k; ++c1)
        {
            for(int c2 = c1 + 1; c2 < 2 * k; ++c2)
            {
                cnf.push_back({Lit(vars[l][c1], true), Lit(vars[l][c2], true)});
            }
        }

        // adjacent edges cannot belong to the same color class and parity
        for(int j = i + 1; j < (int)edges.size(); ++j)
        {
            Location l2 = edges[j];

            if(!adjacent(l, l2))
            {
                continue;
            }
            for(int c = 0; c < 2 * k; ++c)
            {
                cnf.push_back({Lit(vars[l][c], true), Lit(vars[l2][c], true)});
            }
        }

        // each endpoint has to be incident to at least one edge in the same color class and opposite parity
        for(auto& n : {l.n1(), l.n2()})
        {
            for(int c = 0; c < 2 * k; ++c)
            {
                clause = {Lit(vars[l][c], true)};

                int c2 = c + (c & 1 ? -1 : 1);
                for(auto l2 : edges)
                {
                    if(l == l2 || !incident(n, l2))
                    {
                        continue;
                    }

                    clause.push_back(Lit(vars[l2][c2], false));
                }

                cnf.push_back(clause);
            }
        }

        // each endpoint has to be incident to at most one edge in the same color class and opposite parity
        for(auto& n : {l.n1(), l.n2()})
        {
            for(int j1 = 0; j1 < (int)edges.size(); ++j1)
            {
                Location l1 = edges[j1];
                if(!incident(n, l1) || l == l1)
                {
                    continue;
                }
                for(int j2 = j1 + 1; j2 < (int)edges.size(); ++j2)
                {
                    Location l2 = edges[j2];
                    if(!incident(n, l2) || l == l2)
                    {
                        continue;
                    }

                    for(int c = 0; c < 2 * k; ++c)
                    {
                        int c2 = c + (c & 1 ? -1 : 1);
                        cnf.push_back({Lit(vars[l][c], true), Lit(vars[l1][c2], true), Lit(vars[l2][c2], true)});
                    }
                }
            }
        }


        // symmetry breaking - if I am in color class c there is a smaller edge in color class c-1
        for(int c = 2; c < 2 * k; ++c)
        {
            clause = {Lit(vars[l][c], true)};

            for(int j = 0; j < i; ++j)
            {
                clause.push_back(Lit(vars[edges[j]][(c/2-1)*2], false));
            }
            cnf.push_back(clause);
        }
        // if I am at an odd position there must be an edge on even position
        for(int c = 1; c < 2 * k; c+=2)
        {
            clause = {Lit(vars[l][c], true)};
            for(int j = 0; j < i; ++j)
            {
                clause.push_back(Lit(vars[edges[j]][(c - 1)], false));
            }
            cnf.push_back(clause);
        }
    }


    return {edges.size() * 2 * k, cnf};
}
}  // namespace internal

inline bool has_ecd_size_sat(const SatSolver& solver, const Graph& g, int k)
{
    CNF cnf = internal::cnf_ecd(g, k);
    return satisfiable(solver, cnf);
}

inline int ecd_size_sat(const SatSolver& solver, const Graph& g)
{
    int div_constant = 4;
    for(auto l1 : g.list(RP::all(), IP::primary(), IT::l()))
    {
        for(auto&i : g[l1.n1()])
        {

        }
    }

    int l = -1; int r = g.size()/div_constant;

    while(r - l > 1)
    {
        int m = (l + r) / 2;
        if(has_ecd_size_sat(solver, g, m))
        {
            r = m;
        }
        else
        {
            l = m;
        }
    }

    if(has_ecd_size_sat(solver, g, r))
    {
        return r;
    }
    return -1;

    // bool has_ecd = false;
    // for(int k = g.size()/4; k>=0; --k)
    // {
    //     if(has_ecd_size_sat(solver, g, k))
    //     {
    //         has_ecd = true;
    //     }
    //     else
    //     {
    //         return has_ecd?k+1:-1;
    //     }
    // }
    //
    // return 0;

    // for(int k = 0; k <= g.size() / 4; ++k)
    // {
    //     if(has_ecd_size_sat(solver, g, k))
    //     {
    //         return k;
    //     }
    // }

    return -1;
}
}  // namespace ba_graph
#endif  // BA_GRAPH_SAT_CNF_ECD_HPP
