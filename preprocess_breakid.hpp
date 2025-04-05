#ifndef PREPROCESS_BREAKID_HPP
#define PREPROCESS_BREAKID_HPP

// https://github.com/meelgroup/breakid
#include <breakid/breakid.hpp>

namespace ba_graph
{
namespace internal
{
// according to https://github.com/meelgroup/breakid/blob/master/src/config.hpp
struct Config
{
    bool useMatrixDetection = true;
    bool useBinaryClauses = true;
    bool useShatterTranslation = false;
    bool useFullTranslation = false;
    int symBreakingFormLength = 50;
    int64_t steps_lim = std::numeric_limits<int64_t>::max();
};

}  // namespace internal
// according to https://github.com/meelgroup/breakid/blob/master/src/breakid-main.cpp
inline CNF preprocess_breakid(CNF cnf)
{
    internal::Config conf;
    BID::BreakID breakid;
    breakid.set_useMatrixDetection(conf.useMatrixDetection);
    breakid.set_useBinaryClauses(conf.useBinaryClauses);
    breakid.set_useShatterTranslation(conf.useShatterTranslation);
    breakid.set_useFullTranslation(conf.useFullTranslation);
    breakid.set_symBreakingFormLength(conf.symBreakingFormLength);
    breakid.set_verbosity(0);
    breakid.set_steps_lim(conf.steps_lim);

    int nVars = cnf.first;
    std::vector<std::vector<BID::BLit>> cnf_clauses;
    cnf_clauses.reserve(cnf.second.size());
    std::vector<BID::BLit> cnf_inclause;

    for(auto& cnf_cl : cnf.second)
    {
        for(auto& cnf_lit : cnf_cl)
        {
#ifdef BA_GRAPH_DEBUG
            assert(cnf_lit.var() >= 0);
#endif
            cnf_inclause.push_back(BID::BLit(cnf_lit.var(), cnf_lit.neg()));
        }
        cnf_clauses.push_back(cnf_inclause);
        cnf_inclause.clear();
    }

    breakid.start_dynamic_cnf(nVars);
    for(auto cnf_cl : cnf_clauses)
    {
        breakid.add_clause(cnf_cl.data(), cnf_cl.size());
    }

    breakid.end_dynamic_cnf();
    breakid.detect_subgroups();
    breakid.break_symm();

#ifdef BA_GRAPH_DEBUG
    assert(breakid.get_num_aux_vars() <= INT_MAX);
#endif

    cnf.first += (int)breakid.get_num_aux_vars();
    auto brk_clauses = breakid.get_brk_cls();
    cnf.second.reserve(cnf.second.size() + brk_clauses.size());
    for(auto& brk_cl : brk_clauses)
    {
        std::vector<Lit> brk_inclause;
        for(auto brk_lit : brk_cl)
        {
#ifdef BA_GRAPH_DEBUG
            assert(brk_lit.var() <= INT_MAX);
#endif
            brk_inclause.push_back(Lit((int)brk_lit.var(), brk_lit.sign()));
        }
        cnf.second.push_back(brk_inclause);
        brk_inclause.clear();
    }

    return cnf;
}
}  // namespace ba_graph
#endif  // PREPROCESS_BREAKID_HPP
