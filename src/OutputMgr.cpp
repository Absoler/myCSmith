// -*- mode: C++ -*-
//
// Copyright (c) 2007, 2008, 2009, 2010, 2011, 2013, 2014, 2015, 2017 The University of Utah
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

#include "OutputMgr.h"

#include <cassert>
#include <sstream>
#include "Common.h"
#include "CGOptions.h"
#include "platform.h"
#include "Bookkeeper.h"
#include "FunctionInvocation.h"
#include "Function.h"
#include "VariableSelector.h"
#include "CGContext.h"
#include "ExtensionMgr.h"
#include "Constant.h"
#include "ArrayVariable.h"
#include "git_version.h"
#include "random.h"
#include "util.h"

const char *OutputMgr::hash_func_name = "csmith_compute_hash";

const char *OutputMgr::step_hash_func_name = "step_hash";

static const char runtime_include[] = "\
#include \"csmith.h\"\n\
";

static const char volatile_include[] = "\
/* To use wrapper functions, compile this program with -DWRAP_VOLATILES=1. */\n\
#include \"volatile_runtime.h\"\n\
";

static const char access_once_macro[] = "\
#ifndef ACCESS_ONCE\n\
#define ACCESS_ONCE(v) (*(volatile typeof(v) *)&(v))\n\
#endif\n\
";

using namespace std;

vector<string> OutputMgr::monitored_funcs_;

std::string OutputMgr::curr_func_ = "";

void
OutputMgr::set_curr_func(const std::string &fname)
{
	OutputMgr::curr_func_ = fname;
}

bool
OutputMgr::is_monitored_func(void)
{
	if (OutputMgr::monitored_funcs_.empty())
		return true;
	std::vector<string>::iterator result =
		find(monitored_funcs_.begin(), monitored_funcs_.end(), curr_func_);
	return (result != monitored_funcs_.end());
}

OutputMgr::OutputMgr()
{

}

OutputMgr::~OutputMgr()
{

}

// void 
// OutputMgr::Output_getInfoFunc(std::ostream &out){
// 	out<<"void getInfo(unsigned long addr, unsigned int size){"<<endl;
// 	out<<"    side=(side+addr%1000+size%1000)%1000;"<<endl;
// 	out<<"}"<<endl;
// }
void 
OutputMgr::Output_getInfoFunc(std::ostream &out){
	out<<"void __attribute__ ((noinline)) getInfo(unsigned long varInfosAddr, int varCnt, unsigned long accessesAddr, int accessCnt){"<<endl;
	out<<"    side=(side+(varInfosAddr+varCnt)%1000 + (accessesAddr+accessCnt)%1000)%1000;"<<endl;
	out<<"}"<<endl;
}

void
OutputMgr::Output_extraVariables(std::ostream &out){
	out<<"unsigned long side=0;"<<endl;	//create side-effect for target behaviors

    out<<"#define MAX_VAR_NUM 1000"<<endl;
	out<<"struct VarInfo{"<<endl;
	out<<"	unsigned long addr;"<<endl;
	out<<"	unsigned int size;"<<endl;
	out<<"	char name[10];"<<endl;
	out<<"}varInfos[1000];"<<endl;
	out<<"int global_cnt=0;"<<endl;

    out<<"#define MAX_ACCESS_NUM 2000"<<endl;
    out<<"struct AccessInfo{"<<endl;
    out<<"    unsigned long addr;"<<endl;
    out<<"    unsigned int length;"<<endl;
    out<<"    int cnt;"<<endl;
    out<<"}accesses[MAX_ACCESS_NUM];"<<endl;
    out<<"int access_cnt=0;"<<endl;
}

void
OutputMgr::Output_setInfoFunc(std::ostream &out){
	out<<"void __attribute__ ((noinline)) setInfo(unsigned long addr, unsigned int size, char* name){"<<endl;
	out<<"	varInfos[global_cnt].addr=addr;"<<endl;
    out<<"	varInfos[global_cnt].size=size;"<<endl;
	out<<"	strcpy(varInfos[global_cnt].name,name);"<<endl;
    out<<"	global_cnt++;"<<endl;
    out<<"  side=(side+size)%1000;"<<endl;
    out<<"}"<<endl;
}

void 
OutputMgr::Output_setReadCntFunc(std::ostream &out){
	out<<"void __attribute__ ((noinline)) setReadCnt(unsigned long addr, unsigned int length, int cnt){"<<endl;
    out<<"    accesses[access_cnt].addr=addr;"<<endl;
    out<<"    accesses[access_cnt].length = length;"<<endl;
    out<<"    accesses[access_cnt].cnt =cnt;"<<endl;
    out<<"    access_cnt++;"<<endl;
	out<<"}"<<endl;
}

void
OutputMgr::Output_handleTimeout(std::ostream &out){
	out << "void handleSignal(int val){"<<endl;
	out << "	if(val==SIGTERM){"<<endl;
	out << "        printf(\"timeout\\n\");"<<endl;
	out << "        FILE* resfile=fopen(\"timeout.out\",\"a\");"<<endl;
	// out << "        fprintf(resfile, \"timeout in "+std::to_string(seed)+"\\n\" );"<<endl;	// this was originally written in output_header which has seed as parm
	out << "        fclose(resfile);"<<endl;
	out << "        exit(0);" <<endl;
	out << "	}"<<endl;
	out << "}"<<endl;
}

void
OutputMgr::OutputMain(std::ostream &out)
{
	CGContext cg_context(GetFirstFunction() /* BOGUS -- not in first func. */,
						 Effect::get_empty_effect(),
						 0);

	FunctionInvocation *invoke = NULL;
	invoke = ExtensionMgr::MakeFuncInvocation(GetFirstFunction(), cg_context);
	out << endl << endl;
	output_comment_line(out, "----------------------------------------");

	ExtensionMgr::OutputInit(out);

	// output initializers for global array variables
	OutputArrayInitializers(*VariableSelector::GetGlobalVariables(), out, 1);

	// output set signal
	out << "	signal(SIGTERM, handleSignal);"<<endl;

	// in case of target functions are inlined
	vector<string> useFuncs=generate_useFuncs();
	for(string useStmt:useFuncs){
		out<<useStmt<<endl;
	}
	// send var info (name, length) to pintool
	VariableSelector::generate_setGlobalInfos(out);
	output_tab(out, 1);
	
	// send globals' read counter to pintool
	Function::generate_setReadCnt(out);
	out<<"getInfo((unsigned long)varInfos, global_cnt, (unsigned long)accesses, access_cnt);"<<endl;	//transfer varInfos and accessInfos to pintool

	
	// if (CGOptions::blind_check_global()) {
	// 	ExtensionMgr::OutputFirstFunInvocation(out, invoke);
	// 	std::vector<Variable *>& vars = *VariableSelector::GetGlobalVariables();
	// 	for (size_t i=0; i<vars.size(); i++) {
	// 		vars[i]->output_value_dump(out, "checksum ", 1);
	// 	}
	// }
	// else {
	// 	// set up a global variable that controls if we print the hash value after computing it for each global
	// 	out << "    int print_hash_value = 0;" << endl;
	// 	if (CGOptions::accept_argc()) {
	// 		out << "    if (argc == 2 && strcmp(argv[1], \"1\") == 0) print_hash_value = 1;" << endl;
	// 	}

	// 	out << "    platform_main_begin();" << endl;
	// 	if (CGOptions::compute_hash()) {
	// 		out << "    crc32_gentab();" << endl;
	// 	}

	// 	ExtensionMgr::OutputFirstFunInvocation(out, invoke);

	// #if 0
		out << "    ";
		invoke->Output(out);
		out << ";" << endl;
	// #endif
	// 	// resetting all global dangling pointer to null per Rohit's request
	// 	if (!CGOptions::dangling_global_ptrs()) {
	// 		OutputPtrResets(out, GetFirstFunction()->dead_globals);
	// 	}

	// 	if (CGOptions::step_hash_by_stmt())
	// 		OutputMgr::OutputHashFuncInvocation(out, 1);
	// 	else
	// 		HashGlobalVariables(out);
	// 	if (CGOptions::compute_hash()) {
	// 		out << "    platform_main_end(crc32_context ^ 0xFFFFFFFFUL, print_hash_value);" << endl;
	// 	} else {
	// 		out << "    platform_main_end(0,0);" << endl;
	// 	}
	// }
	outputln(out);
	VariableSelector::generate_setVarSide(out);
	outputln(out);
	out<<"	printf(\"%d\\n\",side);"<<endl;	//create side effect
	ExtensionMgr::OutputTail(out);
	out << "}" << endl;
	delete invoke;
}

void
OutputMgr::OutputHashFuncInvocation(std::ostream &out, int indent)
{
	OutputMgr::output_tab_(out, indent);
	out << OutputMgr::hash_func_name << "();" << std::endl;
}

void
OutputMgr::OutputStepHashFuncInvocation(std::ostream &out, int indent, int stmt_id)
{
	if (is_monitored_func()) {
		OutputMgr::output_tab_(out, indent);
		out << OutputMgr::step_hash_func_name << "(" << stmt_id << ");" << std::endl;
	}
}

void
OutputMgr::OutputStepHashFuncDef(std::ostream &out)
{
	out << std::endl;
	out << "void " << OutputMgr::step_hash_func_name << "(int stmt_id)" << std::endl;
	out << "{" << std::endl;

	int indent = 1;
	OutputMgr::output_tab_(out, indent);
	out << "int i = 0;" << std::endl;
	OutputMgr::OutputHashFuncInvocation(out, indent);
	OutputMgr::output_tab_(out, indent);
	out << "printf(\"before stmt(%d): ";
	out << "checksum = %X\\n\", stmt_id, crc32_context ^ 0xFFFFFFFFUL);" << std::endl;

	OutputMgr::output_tab_(out, indent);
	out << "crc32_context = 0xFFFFFFFFUL; " << std::endl;

	OutputMgr::output_tab_(out, indent);
	out << "for (i = 0; i < 256; i++) { " << std::endl;
	OutputMgr::output_tab_(out, indent+1);
	out << "crc32_tab[i] = 0;" << std::endl;
	OutputMgr::output_tab_(out, indent);
	out << "}" << std::endl;
	OutputMgr::output_tab_(out, indent);
	out << "crc32_gentab();" << endl;
	out << "}" << std::endl;
}

void
OutputMgr::OutputHashFuncDecl(std::ostream &out)
{
	out << "void " << OutputMgr::hash_func_name << "(void);";
	out << std::endl << std::endl;
}

void
OutputMgr::OutputStepHashFuncDecl(std::ostream &out)
{
	out << "void " << OutputMgr::step_hash_func_name << "(int stmt_id);";
	out << std::endl << std::endl;
}

void
OutputMgr::OutputHashFuncDef(std::ostream &out)
{
	out << "void " << OutputMgr::hash_func_name << "(void)" << std::endl;
	out << "{" << std::endl;

	size_t dimen = Variable::GetMaxArrayDimension(*VariableSelector::GetGlobalVariables());
	if (dimen) {
		vector <const Variable*> &ctrl_vars = Variable::get_new_ctrl_vars();
		OutputArrayCtrlVars(ctrl_vars, out, dimen, 1);
	}
	HashGlobalVariables(out);
	out << "}" << std::endl;
}

void
OutputMgr::OutputTail(std::ostream &out)
{
	if (!CGOptions::concise()) {
		out << endl << "/************************ statistics *************************" << endl;
		Bookkeeper::output_statistics(out);
		out << "********************* end of statistics **********************/" << endl;
		out << endl;
	}
}

void
OutputMgr::OutputHeader(int argc, char *argv[], unsigned long seed)
{
	std::ostream &out = get_main_out();
	if (CGOptions::concise()) {
		out << "// Options:  ";
		if (argc <= 1) {
			out << " (none)";
		} else {
			for (int i = 1; i < argc; ++i) {
				out << " " << argv[i];
			}
		}
		out << endl;
	}
	else {
		out << "/*" << endl;
		out << " * This is a RANDOMLY GENERATED PROGRAM." << endl;
		out << " *" << endl;
		out << " * Generator: " << PACKAGE_STRING << endl;
		out << " * Git version: " << git_version << endl;
		out << " * Options:  ";
		if (argc <= 1) {
			out << " (none)";
		} else {
			for (int i = 1; i < argc; ++i) {
				out << " " << argv[i];
			}
		}
		out << endl;
		out << " * Seed:      " << seed << endl;
		out << " */" << endl;
		out << endl;
	}

	if (!CGOptions::longlong()) {
		out << endl;
		out << "#define NO_LONGLONG" << std::endl;
		out << endl;
	}
	if (CGOptions::enable_float()) {
		out << "#include <float.h>\n";
		out << "#include <math.h>\n";
	}

	ExtensionMgr::OutputHeader(out);

	// out << runtime_include << endl;
	out<<"typedef signed char int8_t;\n\
typedef unsigned char uint8_t;\n\
typedef signed short int int16_t;\n\
typedef unsigned short int uint16_t;\n\
typedef signed int int32_t;\n\
typedef unsigned int uint32_t;\n\
typedef signed long int int64_t;\n\
typedef unsigned long int uint64_t;\n";

 	if (!CGOptions::compute_hash()) {
		if (CGOptions::allow_int64())
			out << "volatile uint64_t " << Variable::sink_var_name << " = 0;" << endl;
		else
			out << "volatile uint32_t " << Variable::sink_var_name << " = 0;" << endl;
	}
	out << endl;

	// out << "static long __undefined;" << endl;
	out << endl;

	if (CGOptions::depth_protect()) {
		out << "#define MAX_DEPTH (5)" << endl;
		// Make depth signed, to cover our tails.
		out << "int32_t DEPTH = 0;" << endl;
		out << endl;
	}

	// out << platform_include << endl;
	if (CGOptions::wrap_volatiles()) {
		out << volatile_include << endl;
	}

	if (CGOptions::access_once()) {
		out << access_once_macro << endl;
	}

	if (CGOptions::step_hash_by_stmt()) {
		OutputMgr::OutputHashFuncDecl(out);
		OutputMgr::OutputStepHashFuncDecl(out);
	}


	//output signal handler
	//now for record of infinite loop
	out << "#include<stdlib.h>\n";
	out << "#include<signal.h>\n";
	out << "#include<stdio.h>\n";
	out << "#include<string.h>\n";
}

void
OutputMgr::output_comment_line(ostream &out, const std::string &comment)
{
	if (CGOptions::quiet() || CGOptions::concise()) {
		outputln(out);
	}
	else {
		out << "/* " << comment << " */";
		outputln(out);
	}
}

/*
 * resetting pointers to null by outputing "p = 0;"
 */
void
OutputMgr::OutputPtrResets(ostream &out, const vector<const Variable*>& ptrs)
{
	size_t i;
	for (i=0; i<ptrs.size(); i++) {
		const Variable* v = ptrs[i];
		if (v->isArray) {
			const ArrayVariable* av = (const ArrayVariable*)v;
			Constant zero(get_int_type(), "0");
			vector<const Variable *> &ctrl_vars = Variable::get_last_ctrl_vars();
			av->output_init(out, &zero, ctrl_vars, 1);
		}
		else {
			output_tab(out, 1);
			v->Output(out);
			out << " = 0;";
			outputln(out);
		}
	}
}

void
OutputMgr::output_tab_(ostream &out, int indent)
{
	while (indent--) {
		out << TAB;
	}
}

void
OutputMgr::output_tab(ostream &out, int indent)
{
	OutputMgr::output_tab_(out, indent);
}

void
OutputMgr::really_outputln(ostream &out)
{
	out << std::endl;
}

//////////////////////////////////////////////////////////////////
