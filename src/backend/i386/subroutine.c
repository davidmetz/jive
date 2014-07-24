/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/backend/i386/subroutine.h>

#include <stdio.h>

#include <jive/arch/address-transform.h>
#include <jive/arch/addresstype.h>
#include <jive/arch/stackslot.h>
#include <jive/arch/subroutine/nodes.h>
#include <jive/backend/i386/instructionset.h>
#include <jive/backend/i386/registerset.h>
#include <jive/types/function/fctlambda.h>
#include <jive/vsdg.h>
#include <jive/vsdg/splitnode.h>
#include <jive/vsdg/substitution.h>

/* convert according to "default" ABI */
jive_node *
jive_i386_subroutine_convert(jive_region * target_parent, jive_node * lambda_node)
{
	jive_region * src_region = lambda_node->producer(0)->region;
	jive_context * context = target_parent->graph->context;
	
	size_t nparameters = src_region->top->noutputs - 1;
	size_t nreturns = src_region->bottom->ninputs - 1;
	
	size_t nvalue_parameters = 0, nstate_parameters = 0;
	size_t nvalue_returns = 0, nstate_returns = 0;
	jive_argument_type value_parameters[nparameters];
	jive_argument_type value_returns[nreturns];
	
	size_t n;
	for (n = 0; n < nparameters; n++) {
		jive::output * param = src_region->top->outputs[n + 1];
		if (dynamic_cast<jive::value::output*>(param)) {
			value_parameters[nvalue_parameters ++] = jive_argument_long; /* FIXME: pick correct type */
		} else {
			nstate_parameters ++;
		}
	}
	for (n = 0; n < nreturns; n++) {
		jive::input * ret = src_region->bottom->inputs[n + 1];
		if (dynamic_cast<jive::value::input*>(ret)) {
			value_returns[nvalue_returns ++] = jive_argument_long; /* FIXME: pick correct type */
		} else {
			nstate_returns ++;
		}
	}
	
	jive_subroutine sub = jive_i386_subroutine_begin(
		target_parent->graph,
		nvalue_parameters, value_parameters,
		nvalue_returns, value_returns);

	jive_substitution_map * subst = jive_substitution_map_create(context);
	
	/* map all parameters */
	nvalue_parameters = 0;
	for (n = 1; n < src_region->top->noutputs; n++) {
		jive::output * original = src_region->top->outputs[n];
		
		jive::output * substitute;
		if (dynamic_cast<jive::value::output*>(original)) {
			substitute = jive_subroutine_simple_get_argument(sub, nvalue_parameters ++);
		} else {
			substitute = jive_node_add_output(sub.region->top, &original->type());
		}
		
		if(dynamic_cast<jive::addr::output*>(original))
			substitute = jive_bitstring_to_address_create(substitute, 32, &original->type());
		jive_substitution_map_add_output(subst, original, substitute);
	}
	
	/* transfer function body */
	jive_region_copy_substitute(src_region, sub.region, subst, false, false);
	
	/* map all returns */
	nvalue_returns = 0;
	for (n = 1; n < src_region->bottom->ninputs; n++) {
		jive::input * original = src_region->bottom->inputs[n];
		jive::output * retval = jive_substitution_map_lookup_output(
			subst, src_region->bottom->inputs[n]->origin());
		
		if (dynamic_cast<jive::value::input*>(original)) {
			if(dynamic_cast<jive::addr::input*>(original))
				retval = jive_address_to_bitstring_create(retval, 32, &retval->type());
			jive_subroutine_simple_set_result(sub, nvalue_returns ++, retval);
		} else {
			/* FIXME: state returns currently unsupported */
			JIVE_DEBUG_ASSERT(false);
		}
	}
	
	jive_substitution_map_destroy(subst);
	
	return jive_subroutine_end(sub);
}

static void
jive_i386_subroutine_prepare_stackframe_(
	const jive::subroutine_op & op,
	jive_region * region,
	jive_subroutine_stackframe_info * frame,
	const jive_subroutine_late_transforms * xfrm);

static jive::input *
jive_i386_subroutine_add_fp_dependency_(
	const jive::subroutine_op & op, jive_region * region, jive_node * node);

static jive::input *
jive_i386_subroutine_add_sp_dependency_(
	const jive::subroutine_op & op, jive_region * region, jive_node * node);

const jive_subroutine_abi_class JIVE_I386_SUBROUTINE_ABI = {
	prepare_stackframe : jive_i386_subroutine_prepare_stackframe_,
	add_fp_dependency : jive_i386_subroutine_add_fp_dependency_,
	add_sp_dependency : jive_i386_subroutine_add_sp_dependency_,
	instructionset : &jive_i386_instructionset
};

namespace {

class i386_c_builder_interface final : public jive::subroutine_hl_builder_interface {
public:
	virtual
	~i386_c_builder_interface() noexcept
	{
	}

	virtual jive::output *
	value_parameter(
		jive_subroutine & subroutine,
		size_t index) override
	{
		jive::output * o = subroutine.builder_state->arguments[index].output;
	
		const jive::base::type * in_type = &o->type();
		const jive::base::type * out_type = jive_resource_class_get_type(&jive_i386_regcls_gpr.base);
		jive_node * node = jive_splitnode_create(subroutine.region,
			in_type, o, o->gate->required_rescls,
			out_type, &jive_i386_regcls_gpr.base);
		return node->outputs[0];
	}

	virtual void
	value_return(
		jive_subroutine & subroutine,
		size_t index,
		jive::output * value) override
	{
		subroutine.builder_state->results[index].output = value;
	}
	
	virtual jive::output *
	finalize(
		jive_subroutine & subroutine) override
	{
		jive_node * ret_instr = jive_instruction_node_create(
			subroutine.region, &jive_i386_instr_ret, NULL, NULL);
		/* add dependency on return address on stack */
		jive_node_gate_input(
			ret_instr,
			subroutine.builder_state->passthroughs[6].gate,
			subroutine.builder_state->passthroughs[6].output);
		return ret_instr->outputs[0];
	}
};

}

jive_subroutine
jive_i386_subroutine_begin(jive_graph * graph,
	size_t nparameters, const jive_argument_type parameter_types[],
	size_t nreturns, const jive_argument_type return_types[])
{
	jive_context * context = graph->context;
	
	jive::subroutine_machine_signature sig;
	size_t n;
	
	for (size_t n = 0; n < nparameters; n++) {
		char argname[80];
		snprintf(argname, sizeof(argname), "arg%zd", n + 1);
		const jive_resource_class * cls = jive_fixed_stackslot_class_get(4, 4, (n + 1) * 4);
		sig.arguments.emplace_back(jive::subroutine_machine_signature::argument{argname, cls, true});
	}
	
	for (size_t n = 0; n < nreturns; n++) {
		char resname[80];
		snprintf(resname, sizeof(resname), "ret%zd", n + 1);
		const jive_resource_class * cls;
		switch (n) {
			case 0: cls = &jive_i386_regcls_gpr_eax.base; break;
			default: cls = jive_fixed_stackslot_class_get(4, 4, n * 4);
		}
		sig.results.emplace_back(jive::subroutine_machine_signature::result{resname, cls});
	}
	
	const jive_resource_class * stackslot_cls = jive_fixed_stackslot_class_get(4, 4, 0);

	typedef jive::subroutine_machine_signature::passthrough pt;
	sig.passthroughs.emplace_back(
		pt{"mem", nullptr, false});
	sig.passthroughs.emplace_back(
		pt{"saved_esp", &jive_i386_regcls_gpr_esp.base, false});
	sig.passthroughs.emplace_back(
		pt{"saved_ebx", &jive_i386_regcls_gpr_ebx.base, true});
	sig.passthroughs.emplace_back(
		pt{"saved_ebp", &jive_i386_regcls_gpr_ebp.base, true});
	sig.passthroughs.emplace_back(
		pt{"saved_esi", &jive_i386_regcls_gpr_esi.base, true});
	sig.passthroughs.emplace_back(
		pt{"saved_edi", &jive_i386_regcls_gpr_edi.base, true});
	sig.passthroughs.emplace_back(
		pt{"return_addr", stackslot_cls, false});
	
	sig.stack_frame_upper_bound = 4 * (nparameters + 1);
	
	sig.abi_class = &JIVE_I386_SUBROUTINE_ABI;
	
	std::unique_ptr<jive::subroutine_hl_builder_interface> builder(
		new i386_c_builder_interface());
	return jive_subroutine_begin(
		graph,
		std::move(sig),
		std::move(builder));
}

typedef struct jive_i386_stackptr_split_factory {
	jive_value_split_factory base;
	int offset;
} jive_i386_stackptr_split_factory;

static jive::output *
do_stackptr_sub(const jive_value_split_factory * self_, jive::output * value)
{
	const jive_i386_stackptr_split_factory * self = (const jive_i386_stackptr_split_factory *) self_;
	int64_t immediates[1] = {self->offset};
	
	return jive_instruction_node_create_simple(
		value->node()->region,
		&jive_i386_instr_int_sub_immediate,
		&value, immediates)->outputs[0];
}

static jive::output *
do_stackptr_add(const jive_value_split_factory * self_, jive::output * value)
{
	const jive_i386_stackptr_split_factory * self = (const jive_i386_stackptr_split_factory *) self_;
	int64_t immediates[1] = {self->offset};
	
	return jive_instruction_node_create_simple(
		value->node()->region,
		&jive_i386_instr_int_add_immediate,
		&value, immediates)->outputs[0];
}

static void
jive_i386_subroutine_prepare_stackframe_(
	const jive::subroutine_op & op,
	jive_region * region,
	jive_subroutine_stackframe_info * frame,
	const jive_subroutine_late_transforms * xfrm)
{
	frame->lower_bound -= frame->call_area_size;
	
	if (!frame->lower_bound)
		return;
	
	frame->lower_bound = ((frame->lower_bound - 4) & ~15) + 4;
	
	jive_i386_stackptr_split_factory stackptr_sub = {
		{do_stackptr_sub},
		- frame->lower_bound
	};
	jive_i386_stackptr_split_factory stackptr_add = {
		{do_stackptr_add},
		- frame->lower_bound
	};
	
	/* as long as no frame pointer is used, access to stack slots through stack
	pointer must be relocated */
	frame->frame_pointer_offset += frame->lower_bound;
	
	xfrm->value_split(
		xfrm,
		op.get_passthrough_enter_by_index(region, 1),
		op.get_passthrough_leave_by_index(region, 1),
		&stackptr_sub.base,
		&stackptr_add.base);
}

static jive::input *
jive_i386_subroutine_add_fp_dependency_(
	const jive::subroutine_op & op, jive_region * region, jive_node * node)
{
	jive::output * frameptr = op.get_passthrough_enter_by_index(region, 1);
	
	size_t n;
	for (n = 0; n < node->ninputs; n++) {
		jive::input * input = node->inputs[n];
		if (input->origin() == frameptr)
			return NULL;
	}
	return jive_node_add_input(node, &frameptr->type(), frameptr);
}

static jive::input *
jive_i386_subroutine_add_sp_dependency_(
	const jive::subroutine_op & op, jive_region * region, jive_node * node)
{
	jive::output * stackptr = op.get_passthrough_enter_by_index(region, 1);
	
	size_t n;
	for (n = 0; n < node->ninputs; n++) {
		jive::input * input = node->inputs[n];
		if (input->origin() == stackptr)
			return NULL;
	}
	return jive_node_add_input(node, &stackptr->type(), stackptr);
}
