#ifndef STATEMENT_SWITCH_H
#define STATEMENT_SWITCH_H

#include <vector>

#include "Statement.h"

class Block;

class StatementSwitch : public Statement
{
public:
    static StatementSwitch *make_random(CGContext &cg_context);
    static std::vector<int> make_cases(ExpressionVariable *ev);
    StatementSwitch(Block *b, const Expression &exp,
                    const std::vector<int>  &cases,
                    const std::vector<Block *>  &blocks,
                    const std::vector<bool> &breaks,
                    const Block &default_block);
    StatementSwitch(const StatementSwitch& sw);
    virtual ~StatementSwitch(void);

    virtual void get_exprs(std::vector<const Expression*>& exps) const {exps.push_back(&cond);}
    virtual void get_blocks(std::vector<const Block*>& blks) const;

    virtual bool visit_facts(vector<const Fact*>& inputs, CGContext& cg_context) const;

    virtual void Output(std::ostream &out, FactMgr* fm, int indent = 0) const;
    

private:
    const Expression &cond;
    int case_num;
    std::vector<int> cases;
    std::vector<Block *> blocks;
    const Block &default_block;
    std::vector<bool> breaks;
};


#endif