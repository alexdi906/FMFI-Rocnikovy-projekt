#ifndef BA_GRAPH_SAT_CNF_ECD_HPP
#define BA_GRAPH_SAT_CNF_ECD_HPP

#include "invariants/girth.hpp"
#include "io/print_nice.hpp"
#include "sat/cnf.hpp"
#include "sat/exec_solver.hpp"
#include "sat/solver.hpp"
#include "preprocess_breakid.hpp"
#include <impl/basic/include.hpp>
#include <map>
#include <utility>
#include <vector>

namespace ba_graph
{
namespace internal
{

inline CNF cnf_ecd(const Graph& g, int k)
{
    if(k == 0)
    {
        return g.size() ? cnf_unsatisfiable : cnf_satisfiable;
    }
    if(g.size() == 0)
    {
        return cnf_satisfiable;
    }

    auto edges = g.list(RP::all(), IP::primary(), IT::l());
    Number n1 = edges[0].n1();
    // edges with the lowest index will be around n1
    for(auto& I : g[n1])
    {
        Location l = I.is_primary() ? I.l() : I.jump().l();
        auto it = std::find(edges.begin(), edges.end(), l);
        assert(it != edges.end());
        edges.erase(it);
        edges.insert(edges.begin(), l);
    }
    assert(edges.size() == g.size());

    // vars[e][c] means edge e belongs to the c/2-th color class on a position of c&1 parity
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

    // each edge belongs to at least one color class
    for(Location l : edges)
    {
        std::vector<Lit> clause;

        for(int c = 0; c < 2 * k; ++c)
        {
            clause.push_back(Lit(vars[l][c], false));
        }
        cnf.push_back(clause);
    }

    // each edge belongs to at most one color class
    for(Location l : edges)
    {
        for(int c1 = 0; c1 < 2 * k; ++c1)
        {
            for(int c2 = c1 + 1; c2 < 2 * k; ++c2)
            {
                cnf.push_back({Lit(vars[l][c1], true), Lit(vars[l][c2], true)});
            }
        }
    }

    // adjacent edges cannot belong to the same color class and parity
    for(Location l : edges)
    {
        for(auto& n : {l.n1(), l.n2()})
        {
            for(auto& I : g[n])
            {
                Location l2 = I.is_primary() ? I.l() : I.jump().l();
                if(l2 == l || vars[l][0] > vars[l2][0])
                {
                    continue;
                }
                for(int c = 0; c < 2 * k; ++c)
                {
                    cnf.push_back({Lit(vars[l][c], true), Lit(vars[l2][c], true)});
                }
            }
        }
    }

    // each endpoint has to be incident to at least one edge in the same color class and opposite parity
    for(Location l : edges)
    {
        for(auto& n : {l.n1(), l.n2()})
        {
            for(int c = 0; c < 2 * k; ++c)
            {
                std::vector<Lit> clause = {Lit(vars[l][c], true)};

                int c2 = c + (c & 1 ? -1 : 1);
                for(auto& I : g[n])
                {
                    Location l2 = I.is_primary() ? I.l() : I.jump().l();
                    if(l == l2)
                    {
                        continue;
                    }

                    clause.push_back(Lit(vars[l2][c2], false));
                }

                cnf.push_back(clause);
            }
        }
    }

    return {edges.size() * 2 * k, cnf};
}
}  // namespace internal

inline bool has_ecd_size_sat(const SatSolver& solver, const Graph& g, int k)
{
    CNF cnf = internal::cnf_ecd(g, k);
    cnf = preprocess_breakid(cnf);
    return satisfiable(solver, cnf);
}

inline int ecd_size_sat(const SatSolver& solver, const Graph& g)
{
    // search space (l,r]
    int l = min_deg(g) == 4 && max_deg(g) == 4 ? 1 : -1;
    int div_constant = has_parallel_edge(g) ? 2 : 4;
    int r = g.size() / div_constant;

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
    // for(int k = r; k>l; --k)
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
    // if(has_ecd)
    // {
    //     return l+1;
    // }
    // return 0;

    // for(int k = l + 1; k <= r; ++k)
    // {
    //     if(has_ecd_size_sat(solver, g, k))
    //     {
    //         return k;
    //     }
    // }
    //
    // return -1;
}
}  // namespace ba_graph
#endif  // BA_GRAPH_SAT_CNF_ECD_HPP
