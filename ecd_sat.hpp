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
    // color[e][c]=true iff edge e belongs to the c-th color class
    std::map<Location, std::vector<int>> color;
    // even_pos[e]=true iff edge is even position in its color class
    std::map<Location, int> even_pos;

    std::vector<Clause> cnf;

    int next_var = 0;
    for(auto& l : edges)
    {
        std::vector<int> v(k);
        for(int c = 0; c < k; ++c)
        {
            v[c] = next_var++;
        }

        color[l] = v;
        even_pos[l] = next_var++;
    }

    // each edge belongs to at least one color class
    for(auto& l : edges)
    {
        std::vector<Lit> clause;

        for(int c = 0; c < k; ++c)
        {
            clause.push_back(Lit(color[l][c], false));
        }
        cnf.push_back(clause);
    }

    // each edge belongs to at most one color class
    for(auto& l : edges)
    {
        for(int c1 = 0; c1 < k; ++c1)
        {
            for(int c2 = c1 + 1; c2 < k; ++c2)
            {
                cnf.push_back(Clause{Lit(color[l][c1], true), Lit(color[l][c2], true)});
            }
        }
    }

    // // each edge belongs to at most one parity
    // for(auto& l : edges)
    // {
    //     cnf.push_back(Clause{Lit(parity[l][0], true), Lit(parity[l][1], true)});
    // }

    // if adjacent belong to the same color class they must have opposite parity
    for(auto& l : edges)
    {
        for(auto& n : {l.n1(), l.n2()})
        {
            for(auto& i : g[n])
            {
                Location l2 = i.is_primary() ? i.l() : i.jump().l();
                if(l2 == l || color[l][0] > color[l2][0])
                {
                    continue;
                }
                for(int c = 0; c < k; ++c)
                {
                    // exactly one must be on even pos
                    cnf.push_back(Clause{Lit(color[l][c], true), Lit(color[l2][c], true), Lit(even_pos[l], true), Lit(even_pos[l2], true)});
                    cnf.push_back(Clause{Lit(color[l][c], true), Lit(color[l2][c], true), Lit(even_pos[l], false), Lit(even_pos[l2], false)});
                }
            }
        }
    }

    // each edge endpoint has to be incident to at least one edge in the same color class
    for(auto& l : edges)
    {
        for(auto& n : {l.n1(), l.n2()})
        {
            for(int c = 0; c < k; ++c)
            {
                // for(int p = 0; p < 2; ++p)
                // {
                //     std::vector<Clause> dnf;
                //     dnf.push_back(Clause{Lit(color[l][c], true)});
                //     dnf.push_back(Clause{Lit(parity[l][p], true)});

                std::vector<Lit> clause = {Lit(color[l][c], true)};
                for(auto& i : g[n])
                {
                    Location l2 = i.is_primary() ? i.l() : i.jump().l();
                    if(l == l2)
                    {
                        continue;
                    }

                    clause.push_back(Lit(color[l2][c], false));
                    // dnf.push_back(Clause{Lit(color[l2][c], false), Lit(parity[l2][p ^ 1], false)});
                }
                cnf.push_back(clause);

                // auto convert_cnf = dnf_to_cnf(dnf, next_var);
                // for(auto& cl : convert_cnf)
                // {
                //     cnf.push_back(cl);
                //     for(auto& lit : cl)
                //     {
                //         next_var = std::max(next_var, lit.var());
                //     }
                // }
            }
        }
    }

    return {next_var, cnf};
}
}  // namespace internal

inline bool has_ecd_size_sat(const SatSolver& solver, const Graph& g, int k, bool break_symmetry = true)
{
    CNF cnf = internal::cnf_ecd(g, k);
    if(break_symmetry)
    {
        cnf = preprocess_breakid(cnf);
    }
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
