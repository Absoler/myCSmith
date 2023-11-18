// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2010, 2011, 2013, 2014, 2015, 2017 The University of Utah
// All rights reserved.
//
// This file is part of `csmith', a random generator of C programs.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include "ExpressionVariable.h"
#include <cassert>

#include "CGContext.h"
#include "CGOptions.h"
#include "Function.h"
#include "VariableSelector.h"
#include "Fact.h"
#include "FactPointTo.h"
#include "FactMgr.h"
#include "Bookkeeper.h"
#include "Error.h"
#include "DepthSpec.h"
#include "ArrayVariable.h"

#include "random.h"


///////////////////////////////////////////////////////////////////////////////

/*
 *
 */
ExpressionVariable *
ExpressionVariable::make_random(CGContext &cg_context, const Type* type, const CVQualifiers* qfer, bool as_param, bool as_return, map<int,int> *use_counter, genGuide guide)
{
	// use_counter belong to the func which chooses this current var as an arg
	DEPTH_GUARD_BY_TYPE_RETURN(dtExpressionVariable, NULL);
	Function *curr_func = cg_context.get_current_func();
	FactMgr* fm = get_fact_mgr_for_func(curr_func);
	vector<const Variable*> dummy;

	// save current effects, in case we need to reset
	Effect eff_accum = cg_context.get_accum_effect();
	Effect eff_stmt = cg_context.get_effect_stm();

	ExpressionVariable *ev = 0;
	do {
        eVariableScope scope = MAX_VAR_SCOPE;
        if(guide == eGlobalVar && rnd_flipcoin(80)){
            /*
                if always set `eGlobal`, dead loop will happen for seed = 1660405260
                
                when
                1) GlobalList only has one suitable var whose need deref and use
                2) this var's pointToSet is invalid, can't pass 
                    `FactPointTo::opportunistic_validate`
                3) this var is a collective array

                based on 1), given to "artificially increase the odds of dereferencing a pointer",
                the arrayVar will be chosen, and itemized in `choose_ok_var`, the son array will
                be saved in `invalid_vars`, however, when next time the father array is chosen as
                ok_var, it won't be filtered by some son array.

                So for avoidance of add extra check of invalid_var's collective, (maybe some 
                unexpected problem) we just randomize our guide policy
            */
            // PRINT_LOC("make global rval start")
            scope = eGlobal;
        }
		const Variable* var = 0;
		// try to use one of must_read_vars in CGContext
        // PRINT_LOC()
		var = VariableSelector::select_must_use_var(Effect::READ, cg_context, type, qfer);
		if (var == NULL) {
            eMatchType mt = (guide == eMustInt ? eConvert : eFlexible);
			var = VariableSelector::select(Effect::READ, cg_context, type, qfer, dummy, mt, scope);
		}
		ERROR_GUARD(NULL);
		if (!var)
			continue;
		if (!type->is_float() && var->type->is_float())
			continue;
		// forbid a parameter to take the address of an argument
		// this is to simplify the path shortcutting delta
		if (as_param && var->is_argument() && var->type->is_dereferenced_from(type)) {
			continue;
		}
		if (!CGOptions::addr_taken_of_locals()
			&& var->type->is_dereferenced_from(type)
			&& (var->is_argument() || var->is_local())) {
			continue;
		}
        // PRINT_LOC()
		// forbid a escaping pointer to take the address of an argument or a local variable
		int indirection = var->type->get_indirect_level() - type->get_indirect_level();
		if(indirection<0){
			// printf("");
		}
		if (as_return && CGOptions::no_return_dead_ptr() &&
			FactPointTo::is_pointing_to_locals(var, cg_context.get_current_block(), indirection, fm->global_facts)) {
			continue;
		}
		int valid = FactPointTo::opportunistic_validate(var, type, fm->global_facts);
		
		// PRINT_LOC()
		// if this var will be an arg, check whether it contain global that will be read/written in target function (with the same deref-level) 
		VariableSet used_globals_in_param;	// these vars will be read/written if this var is qualified for an arg
		if(as_param && use_counter!=NULL && valid){
            //  read-test and write-test share the same check process
			int indirect=var->type->get_indirect_level() - type->get_indirect_level();
			int end_level=var->type->get_indirect_level();
			
            // if(CGOptions::test_introduce_store()){
                
            // }else{
            // PRINT_LOC()
                map<int, VariableSet> varsMap=FactPointTo::get_pointees_in_range(var, fm->global_facts, indirect, end_level);
                for(int i=1; i<=end_level&&valid; i++){
                    if(use_counter->find(i)==use_counter->end()){
                        //接下来的解引用不会在函数中被访问
                        break;
                    }
                    assert(i<=end_level-indirect);  // 如果没问题，把上面的循环上界改成它
                    for(const Variable* v:varsMap[i]){
                        if(v->is_global()){
                            if((*use_counter)[i]>1 || ((*use_counter)[i]==1&&VariableSelector::check_var_used(v))){	//read/write more than once in the func or once and it's already used
                                valid=false;
                                break;
                            }else{
                                if((*use_counter)[i]==1)
                                    used_globals_in_param.push_back(v);
                            }
                        }
                    }
                }
            // }

		}
		// PRINT_LOC("%s %d", var->name.c_str(), valid)
		if (valid) {
			ExpressionVariable tmp(*var, type);
			if (tmp.visit_facts(fm->global_facts, cg_context)) {
				ev = tmp.get_indirect_level() == 0 ? new ExpressionVariable(*var) : new ExpressionVariable(*var, type);
				bool get=false;
				if (var->name=="g_1343.f3.f1") { 
					get = true;
				}
                // PRINT_LOC()
				int indirect = tmp.get_indirect_level();
				cg_context.curr_blk = cg_context.get_current_block();

				if(var->is_argument()&&indirect>0)
					printf("3");
                
                if(CGOptions::test_introduce_store()){
                    // for store-test, only need restriction on arg, (check whether dereferences this arg will be written in callee)
                    for(const Variable* v:used_globals_in_param){
                        VariableSelector::set_used(v, cg_context);
                    }
                }else{

                    map<int, VariableSet> targets=FactPointTo::get_pointees_under_level(var,indirect,fm->global_facts);
                    
                    for(int i=0; i<=indirect; i++){
                        for(const Variable* var:targets[i]){
                            if(var->is_argument()){
                                VariableSelector::record_paramRead(var, cg_context, indirect-i, as_return); //这个地方添加次数可能过多导致可以用的参数也不能用了
                            }
                            if(get&&var->name=="g_1343.f3.f1"){
                                printf("will set_used\n");
                            }
                            VariableSelector::set_used(var, cg_context, as_return);
                        }
                    }
                    // var is selected as arg and these globals will be deref-ed and read in function
                    // if(used_globals_in_param.size()>0)
                    // 	printf("in %s\n",var->name.c_str());
                    for(const Variable* v:used_globals_in_param){
                        // printf("will read %s\n", v->name.c_str());
                        VariableSelector::set_used(v, cg_context);	//如果以后允许参数执行次数大于一次，这里set_used记录次数时要记录那个次数
                    }

                    if(ev->get_indirect_level()!=0){
                        // printf("%d %s\n",ev->get_indirect_level(),ev->get_var()->get_actual_name().c_str());
                    }
                }
                break;  //success
                
			}else {
                // PRINT_LOC("variable")
				cg_context.reset_effect_accum(eff_accum);
				cg_context.reset_effect_stm(eff_stmt);
			}
		}
		dummy.push_back(var);   //$invalid vars
	} while (true);

	// statistics
	int deref_level = ev->get_indirect_level();
	if (deref_level > 0) {
		incr_counter(Bookkeeper::read_dereference_cnts, deref_level);
	}
	if (deref_level < 0) {
		Bookkeeper::record_address_taken(ev->get_var());
	}
	Bookkeeper::record_volatile_access(ev->get_var(), deref_level, false);
    // if(guide == eGlobalVar){
    //     PRINT_LOC("make global rval done")
    // }
	return ev;
}

ExpressionVariable *
ExpressionVariable::make_new(CGContext &cg_context, const Type* type, const CVQualifiers* qfer, bool as_param, bool as_return, map<int,int> *use_counter){
    DEPTH_GUARD_BY_TYPE_RETURN(dtExpressionVariable, NULL);
	Function *curr_func = cg_context.get_current_func();
	FactMgr* fm = get_fact_mgr_for_func(curr_func);

    // save current effects, in case we need to reset
	Effect eff_accum = cg_context.get_accum_effect();
	Effect eff_stmt = cg_context.get_effect_stm();

    ExpressionVariable *ev = 0;

    const Type *ntype = Type::random_type_from_type(type);
    Variable *var = VariableSelector::genNewGlobalWrapper(Effect::READ, cg_context, ntype, qfer);
    if(!var){
        return ev;
    }
    
    int indirection = var->type->get_indirect_level() - type->get_indirect_level();
    int valid = FactPointTo::opportunistic_validate(var, type, fm->global_facts);
    if(!valid){
        return ev;
    }

    ExpressionVariable tmp(*var, type);
    if(!tmp.visit_facts(fm->global_facts, cg_context)){
        // need reset
        cg_context.reset_effect_accum(eff_accum);
        cg_context.reset_effect_stm(eff_stmt);
        return ev;
    }

    ev = tmp.get_indirect_level() == 0 ? new ExpressionVariable(*var) : new ExpressionVariable(*var, type);
    
    cg_context.curr_blk = cg_context.get_current_block();

    VariableSelector::addToCopyVec(var);
    
    if(!CGOptions::test_introduce_store()){
        map<int, VariableSet> targets=FactPointTo::get_pointees_under_level(var,indirection,fm->global_facts);
        
        for(int i=0; i<=indirection; i++){
            for(const Variable* var:targets[i]){
                if(var->is_argument()){
                    VariableSelector::record_paramRead(var, cg_context, indirection-i, as_return); //这个地方添加次数可能过多导致可以用的参数也不能用了
                }
                VariableSelector::set_used(var, cg_context, as_return);
            }
        }
    
    }

    // statistics
    int deref_level = ev->get_indirect_level();
	if (deref_level > 0) {
		incr_counter(Bookkeeper::read_dereference_cnts, deref_level);
	}
	if (deref_level < 0) {
		Bookkeeper::record_address_taken(ev->get_var());
	}
	Bookkeeper::record_volatile_access(ev->get_var(), deref_level, false);

    
    return ev;
}
/*
 *
 */
ExpressionVariable::ExpressionVariable(const Variable &v)
	: Expression(eVariable),
      var(v),
      type(v.type)
{
}

/*
 *
 */
ExpressionVariable::ExpressionVariable(const Variable &v, const Type* t)
	: Expression(eVariable),
	  var(v),
	  type(t)
{
}

/*
 *
 */
ExpressionVariable::ExpressionVariable(const ExpressionVariable &expr)
	: Expression(eVariable),
	  var(expr.var),
	  type(expr.type)
{
	// Nothing to do
}

Expression *
ExpressionVariable::clone() const
{
	return new ExpressionVariable(*this);
}

/*
 *
 */
ExpressionVariable::~ExpressionVariable(void)
{
	// Nothing to do.
}

///////////////////////////////////////////////////////////////////////////////

/*
 *
 */
const Type &
ExpressionVariable::get_type(void) const
{
	return *(type);
}

/*
 *
 */
int
ExpressionVariable::get_indirect_level(void) const
{
	return var.type->get_indirect_level() - type->get_indirect_level();
}

/*
 *
 */
CVQualifiers
ExpressionVariable::get_qualifiers(void) const
{
	int indirect = get_indirect_level();
	return var.qfer.indirect_qualifiers(indirect);
}

/*
 *
 */
void
ExpressionVariable::Output(std::ostream &out) const
{
	output_cast(out);
	Reducer* reducer = CGOptions::get_reducer();
	if (reducer && reducer->output_expr(this, out)) {
		return;
	}
	int i;
    int indirect_level = get_indirect_level();
    if (indirect_level > 0) {
        out << "(";
		for (i=0; i<indirect_level; i++) {
			out << "*";
		}
	}
	else if (indirect_level < 0) {
		assert(indirect_level == -1);
		out << "&";
    }
	var.Output(out);
    if (indirect_level > 0) {
        out << ")";
    }
}

std::vector<const ExpressionVariable*>
ExpressionVariable::get_dereferenced_ptrs(void) const
{
	// return a empty vector by default
	std::vector<const ExpressionVariable*> refs;
	if (get_indirect_level() > 0) {
		refs.push_back(this);
	}
	return refs;
}

void
ExpressionVariable::get_referenced_ptrs(std::vector<const Variable*>& ptrs) const
{
	if (var.is_pointer()) {
		ptrs.push_back(&var);
	}
}

bool
ExpressionVariable::visit_facts(vector<const Fact*>& inputs, CGContext& cg_context) const
{
	int deref_level = get_indirect_level();
	const Variable* v = get_var();
	if (deref_level > 0) {
		if (!FactPointTo::is_valid_ptr(v, inputs)) {
			return false;
		}
		// Yang: do we need to consider the deref_level?
		bool valid = cg_context.check_read_var(v, inputs) && cg_context.read_pointed(this, inputs) &&
			cg_context.check_deref_volatile(v, deref_level);
		return valid;
	}
	// we filter out bitfield
	if (deref_level < 0) {
		if (v->isBitfield_)
			return false;
		// it's actually valid to take address of a null/dead pointer
		return true;
	}
	else {
		return cg_context.check_read_var(v, inputs);
	}
}

/*
 *
 */
bool
ExpressionVariable::compatible(const Expression *exp) const
{
	assert(exp);

	//if (!(exp->term_type == eVariable))
		//return false;

	return exp->compatible(&var);
}

/*
 *
 */
bool
ExpressionVariable::compatible(const Variable *v) const
{
	assert(v);

	return this->var.compatible(v);
}

///////////////////////////////////////////////////////////////////////////////

// Local Variables:
// c-basic-offset: 4
// tab-width: 4
// End:

// End of file.
