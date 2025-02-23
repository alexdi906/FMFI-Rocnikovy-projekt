#ifndef BA_GRAPH_SAT_CNF_ECD_HPP
#define BA_GRAPH_SAT_CNF_ECD_HPP

#include <sat/cnf.hpp>
#include <impl/basic/include.hpp>
#include <operations/line_graph.hpp>
#include <sat/exec_solver.hpp>
#include <sat/solver.hpp>
#include <io/print_nice.hpp>

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

    inline CNF cnf_ecd(const Graph &g, int k)
    {

        // vars[e][c] means edge e belongs to the c/2-th color class on a position of c&1 parity
        auto edges = g.list(RP::all(), IP::primary(), IT::l());
        assert(edges.size() == g.size());
        std::map<Location, std::vector<int>> vars;

        std::vector<std::vector<Lit>> cnf;

        int next_var = 0;
        for(auto&l : edges)
        {
            std::vector<int> v(2*k);
            for(int c = 0; c<2*k; ++c)
            {
                v[c] = next_var++;
            }

            vars[l] = v;
        }


        for(int i = 0; i<(int)edges.size(); ++i)
        {
            Location l = edges[i];
            std::vector<Lit> clause;

            //each edge belongs to at least one color class
            for(int c = 0; c<2*k; ++c){
                clause.push_back(Lit(vars[l][c],false));
            }
            cnf.push_back(clause);

            if(i == 0 && k!=0)
            {
                cnf.push_back({Lit(vars[l][0],false)});
            }

            //each edge belongs to at most one color class
            for(int c1 = 0; c1<2*k; ++c1)
            {
                for(int c2 = c1+1; c2<2*k; ++c2)
                {
                    cnf.push_back({Lit(vars[l][c1], true), Lit(vars[l][c2], true)});
                }
            }

            //adjacent edges cannot belong to the same color class and parity
            for(int j = i+1; j<(int)edges.size(); ++j)
            {
                Location l2 = edges[j];

                if(!adjacent(l, l2))
                {
                    continue;
                }
                for(int c = 0; c<2*k; ++c)
                {

                    cnf.push_back({Lit(vars[l][c], true), Lit(vars[l2][c], true)});
                }
            }

            // each endpoint has to be incident to at least one edge in the same color class and opposite parity
            for(auto& n : {l.n1(), l.n2()})
            {
                for(int c = 0; c<2*k; ++c)
                {
                    clause = {Lit(vars[l][c], true)};

                    int c2 = c + (c&1?-1:1);
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
            for(auto& n : {l.n1(), l.n2()}){

                for(int j1 = 0; j1<(int)edges.size(); ++j1)
                {
                    Location l1 = edges[j1];
                    if(!incident(n, l1))
                    {
                        continue;
                    }
                    for(int j2 = j1+1; j2<(int)edges.size(); ++j2)
                    {
                        Location l2 = edges[j2];
                        if(!incident(n, l2))
                        {
                            continue;
                        }

                        for(int c = 0; c<2*k; ++c)
                        {

                            int c2 = c + (c&1?-1:1);
                            cnf.push_back({Lit(vars[l][c], true), Lit(vars[l1][c2], true), Lit(vars[l2][c2], true)});
                        }

                    }
                }
            }
        }

        assert(next_var == (int)edges.size()*2*k);
        return {edges.size()*2*k, cnf};
    }
}

inline bool has_ecd_size_sat(const SatSolver& solver, const Graph &g, int k)
{
    CNF cnf = internal::cnf_ecd(g,k);
    return satisfiable(solver, cnf);
}

inline int ecd_size_sat(const SatSolver &solver, const Graph &g)
{
    for(int k = 0; k<=g.size()/2; ++k)
    {
        if(has_ecd_size_sat(solver, g, k))
        {
            return k;
        }
    }

    return -1;
}
}
#endif //BA_GRAPH_SAT_CNF_ECD_HPP
