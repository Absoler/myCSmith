#include "StatementSwitch.h"
#include "DepthSpec.h"
#include "FactMgr.h"
#include "Function.h"
#include "ExpressionVariable.h"
#include "ArrayVariable.h"
#include "Constant.h"
#include "StringUtils.h"
#include "CGOptions.h"
#include "Block.h"

#include <set>

using namespace std;

void
StatementSwitch::get_blocks(vector<const Block*>& blks) const
{
    for (int i = 0; i < case_num; i++)
    {
        blks.push_back(blocks[i]);
    }
}

StatementSwitch::StatementSwitch(Block *b, const Expression &cond,
                    const std::vector<int>  &cases,
                    const std::vector<Block *> &blocks,
                    const std::vector<bool> &breaks,
                    const Block &default_block)
        : Statement(eSwitch, b),
          cond(cond),
          cases(cases),
          blocks(blocks),
          breaks(breaks),
          default_block(default_block)
{
    case_num = cases.size();
}

StatementSwitch::~StatementSwitch(void)
{
    for (int i = 0; i < case_num; i++) {
        delete blocks[i];
    }
    delete &cond;
    delete &default_block;
}

StatementSwitch *
StatementSwitch::make_random(CGContext &cg_context)
{
    DEPTH_GUARD_BY_TYPE_RETURN(dtStatementSwitch, NULL);
    FactMgr *fm = get_fact_mgr(&cg_context);
    FactVec pre_facts;
    Effect pre_effect;

    const Type *type = get_int_type();
    ExpressionVariable *ev = ExpressionVariable::make_random(cg_context, type, NULL, false, false, NULL, eMustInt);

    int case_num;
    vector<int> cases = make_cases(ev);
    case_num = cases.size();
    vector<Block *> blocks(case_num, nullptr);
    vector<bool> breaks(case_num, false);

    for (int i = 0; i < case_num; i++) {
        Block *bi = Block::make_random(cg_context);
        blocks[i] = bi;
        breaks[i] = rnd_flipcoin(50);
    }

    Block *default_block = Block::make_random(cg_context);
    
    StatementSwitch *sw = new StatementSwitch(cg_context.get_current_block(), 
        *ev,
        cases,
        blocks,
        breaks,
        *default_block);

    return sw;
}


vector<int>
StatementSwitch::make_cases(ExpressionVariable *ev)
{
    // vector<int> cases;
    vector<const Expression *> init_values;
    set<int> cases;
    const Variable *var = ev->get_var();
    if (var->isArray) {
        const ArrayVariable *av = dynamic_cast<const ArrayVariable*>(var);
        init_values.assign(av->get_init_values().begin(), av->get_init_values().end());
    } else if (!var->is_argument()) {
        init_values.push_back(var->init);
    }

    for (const Expression *exp : init_values) {
        if (exp && exp->term_type == eConstant) {
            const Constant *c = dynamic_cast<const Constant*>(exp);
            assert(c->get_type().is_int());
            cases.insert(StringUtils::str2int(c->get_value()));

            if (cases.size() >= CGOptions::max_switch_case_num()) {
                break;
            }
        }
    }

    int random_limit = CGOptions::max_switch_case_num() - cases.size();
    if (random_limit > 0) {
        int random_cnt = rnd_upto(random_limit);
        for (int i = 0; i < random_cnt; i++) {
            // set random case value less than 100
            int rand_case;
            do {
                rand_case = rnd_upto(100);
            } while (cases.find(rand_case) != cases.end());
            cases.insert(rand_case);
        }
    }
    
    return vector<int>(cases.begin(), cases.end());
}


bool
StatementSwitch::visit_facts(vector<const Fact*>& inputs, CGContext& cg_context) const
{
    FactVec inputs_copy = inputs;
    if (!cond.visit_facts(inputs, cg_context)) {
        return false;
    }
    FactVec inputs_cases = inputs, inputs_cases_copy = inputs;
    
    for (int i = 0; i < case_num; i++) {
        if (!blocks[i]->visit_facts(inputs_cases, cg_context)) {
            return false;
        }

        
        if (breaks[i]) {
            merge_facts(inputs, inputs_cases);
            inputs_cases = inputs_cases_copy;
        }
    }

    if (!default_block.visit_facts(inputs_cases, cg_context)) {
        return false;
    }
    merge_facts(inputs, inputs_cases);

    return true;
}

void
StatementSwitch::Output(std::ostream &out, FactMgr* fm, int indent) const
{
    // output selection part
    output_tab(out, indent);
    out << "switch (";
    cond.Output(out);
    out << ")";
    outputln(out);

    output_tab(out, indent);
    out << "{";
    outputln(out);
    // output cases body
    for (int i = 0; i < case_num; i++) {
        // output case header
        output_tab(out, indent + 1);
        out << "case " << cases[i] << ":";
        outputln(out);

        // output case body
        blocks[i]->Output(out, fm, indent + 1);

        // output break
        if (breaks[i]) {
            output_tab(out, indent + 2);
            out << "break;";
            outputln(out);
        }
        outputln(out);
    }

    // output default block
    output_tab(out, indent + 1);
    out << "default:";
    outputln(out);
    default_block.Output(out, fm, indent + 1);

    output_tab(out, indent);
    out << "}";
    outputln(out);
}
