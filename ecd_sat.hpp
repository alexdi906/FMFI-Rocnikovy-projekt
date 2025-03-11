#ifndef BA_GRAPH_SAT_CNF_ECD_HPP
#define BA_GRAPH_SAT_CNF_ECD_HPP

#include <map>
#include <utility>
#include <vector>

#include "impl/basic/include.hpp"
#include "io/print_nice.hpp"
#include "sat/cnf.hpp"
#include "sat/exec_solver.hpp"
#include "sat/solver.hpp"

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

    // set around first vertex in 4 regular graphs
    if(max_deg(g) == 4 && min_deg(g) == 4)
    {
        std::vector<Location> n1_inc = {edges[0], edges[1], edges[2], edges[3]};

        assert(k >= 2);

        // first edge is 0
        cnf.push_back({Lit(vars[n1_inc[0]][0], false)});

        // if this edge is 1, then the neighbors will 2 and 3 in fixed order
        for(int i = 1; i < 4; ++i)
        {
            Location l1 = n1_inc[i];

            std::vector<Location> oth;
            for(int j = 1; j < 4; ++j)
            {
                if(i != j)
                {
                    oth.push_back(n1_inc[j]);
                }
            }
            assert(oth.size() == 2);

            Location l2 = oth[0];
            Location l3 = oth[1];

            // if l1=1, l2=2 and l3=3
            cnf.push_back({Lit(vars[l1][1], true), Lit(vars[l2][2], false)});
            cnf.push_back({Lit(vars[l1][1], true), Lit(vars[l3][3], false)});
        }
    }

    for(int i = 0; i < (int)edges.size(); ++i)
    {
        Location l = edges[i];
        std::vector<Lit> clause;

        // each edge belongs to at least one color class
        for(int c = 0; c < 2 * k; ++c)
        {
            clause.push_back(Lit(vars[l][c], false));
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

        // each endpoint has to be incident to at least one edge in the same color class and opposite parity
        for(auto& n : {l.n1(), l.n2()})
        {
            for(int c = 0; c < 2 * k; ++c)
            {
                clause = {Lit(vars[l][c], true)};

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

        // each endpoint has to be incident to at most one edge in the same color class and opposite parity
        //
        // for(auto&n : {l.n1(), l.n2()})
        // {
        //     for(auto&i1 : g[n])
        //     {
        //         Location l1 = i1.l();
        //
        //         if(l1 == l)
        //         {
        //             continue;
        //         }
        //
        //         for(auto&i2 : g[n])
        //         {
        //             Location l2 = i2.l();
        //
        //             if(l2 == l || )
        //         }
        //     }
        // }
        // for(auto& n : {l.n1(), l.n2()})
        // {
        //     for(int j1 = 0; j1 < (int)edges.size(); ++j1)
        //     {
        //         Location l1 = edges[j1];
        //         if(!incident(n, l1) || l == l1)
        //         {
        //             continue;
        //         }
        //         for(int j2 = j1 + 1; j2 < (int)edges.size(); ++j2)
        //         {
        //             Location l2 = edges[j2];
        //             if(!incident(n, l2) || l == l2)
        //             {
        //                 continue;
        //             }
        //
        //             for(int c = 0; c < 2 * k; ++c)
        //             {
        //                 int c2 = c + (c & 1 ? -1 : 1);
        //                 cnf.push_back({Lit(vars[l][c], true), Lit(vars[l1][c2], true), Lit(vars[l2][c2], true)});
        //             }
        //         }
        //     }
        // }

        // symmetry breaking - if I am in color class c there is a smaller edge in color class c-1
        for(int c = 2; c < 2 * k; ++c)
        {
            clause = {Lit(vars[l][c], true)};

            for(int j = 0; j < i; ++j)
            {
                clause.push_back(Lit(vars[edges[j]][(c / 2 - 1) * 2], false));
            }
            cnf.push_back(clause);
        }
        // if I am at an odd position there must be a smaller edge on even position
        for(int c = 1; c < 2 * k; c += 2)
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
