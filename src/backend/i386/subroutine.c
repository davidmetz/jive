/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/backend/i386/subroutine.h>

#include <stdio.h>

#include <jive/arch/stackslot.h>
#include <jive/arch/subroutine-private.h>
#include <jive/backend/i386/registerset.h>
#include <jive/backend/i386/instructionset.h>
#include <jive/vsdg.h>
#include <jive/vsdg/function.h>
#include <jive/vsdg/splitnode.h>
#include <jive/vsdg/substitution.h>
#include <jive/arch/address-transform.h>
#include <jive/arch/addresstype.h>

static inline void
jive_node_add_register_input(jive_node * node, const jive_register_class * regcls, jive_output * origin)
{
	const jive_type * type = jive_register_class_get_type(regcls);
	jive_input * retval = jive_node_add_input(node, type, origin);
	retval->required_rescls = &regcls->base;
}

static inline void
jive_function_region_callee_saved(jive_region * region, const jive_register_class * regcls)
{
	jive_gate * save_gate = jive_register_class_create_gate(regcls, region->graph, regcls->base.name);
	jive_node_gate_input(region->bottom, save_gate, jive_node_gate_output(region->top, save_gate));
}

/* convert according to "default" ABI */
jive_subroutine *
jive_i386_subroutine_convert(jive_region * target_parent, jive_node * lambda_node)
{
	jive_region * src_region = lambda_node->inputs[0]->origin->node->region;
	jive_context * context = target_parent->graph->context;
	
	size_t nparameters = src_region->top->noutputs - 1;
	size_t nreturns = src_region->bottom->ninputs - 1;
	
	size_t nvalue_parameters = 0, nstate_parameters = 0;
	size_t nvalue_returns = 0, nstate_returns = 0;
	jive_argument_type value_parameters[nparameters];
	jive_argument_type value_returns[nreturns];
	
	size_t n;
	for (n = 0; n < nparameters; n++) {
		jive_output * param = src_region->top->outputs[n + 1];
		if (jive_output_isinstance(param, &JIVE_VALUE_OUTPUT)) {
			value_parameters[nvalue_parameters ++] = jive_argument_long; /* FIXME: pick correct type */
		} else {
			nstate_parameters ++;
		}
	}
	for (n = 0; n < nreturns; n++) {
		jive_input * ret = src_region->bottom->inputs[n + 1];
		if (jive_input_isinstance(ret, &JIVE_VALUE_INPUT)) {
			value_returns[nvalue_returns ++] = jive_argument_long; /* FIXME: pick correct type */
		} else {
			nstate_returns ++;
		}
	}
	
	jive_subroutine * subroutine = jive_i386_subroutine_create(target_parent,
		nvalue_parameters, value_parameters,
		nvalue_returns, value_returns);

	jive_substitution_map * subst = jive_substitution_map_create(context);
	
	/* map all parameters */
	nvalue_parameters = 0;
	for (n = 1; n < src_region->top->noutputs; n++) {
		jive_output * original = src_region->top->outputs[n];
		
		jive_output * substitute;
		if (jive_output_isinstance(original, &JIVE_VALUE_OUTPUT)) {
			substitute = jive_subroutine_value_parameter(subroutine, nvalue_parameters ++);
		} else {
			substitute = jive_node_add_output(&subroutine->enter->base, jive_output_get_type(original));
		}
		
		if(jive_output_isinstance(original, &JIVE_ADDRESS_OUTPUT))
			substitute = jive_bitstring_to_address_create(substitute, 32);
		jive_substitution_map_add_output(subst, original, substitute);
	}
	
	/* transfer function body */
	jive_region_copy_substitute(src_region, subroutine->region, subst, false, false);
	
	/* map all returns */
	nvalue_returns = 0;
	for (n = 1; n < src_region->bottom->ninputs; n++) {
		jive_input * original = src_region->bottom->inputs[n];
		jive_output * retval = jive_substitution_map_lookup_output(subst, src_region->bottom->inputs[n]->origin);
		
		if (jive_input_isinstance(original, &JIVE_VALUE_INPUT)) {
			if(jive_input_isinstance(original, &JIVE_ADDRESS_INPUT))
				retval = jive_address_to_bitstring_create(retval, 32);
			jive_subroutine_value_return(subroutine, nvalue_returns ++, retval);
		} else {
			jive_node_add_input(&subroutine->leave->base, jive_input_get_type(original), retval);
		}
	}
	
	jive_substitution_map_destroy(subst);
	
	return subroutine;
}

static jive_output *
jive_i386_subroutine_value_parameter_(jive_subroutine * self_, size_t index);

static jive_input *
jive_i386_subroutine_value_return_(jive_subroutine * self_, size_t index, jive_output * value);

static jive_subroutine *
jive_i386_subroutine_copy_(const jive_subroutine * self_,
	jive_node * new_enter_node, jive_node * new_leave_node);

static void
jive_i386_subroutine_prepare_stackframe_(jive_subroutine * self_, const jive_subroutine_late_transforms * xfrm);

static jive_input *
jive_i386_subroutine_add_fp_dependency_(const jive_subroutine * self, jive_node * node);

static jive_input *
jive_i386_subroutine_add_sp_dependency_(const jive_subroutine * self, jive_node * node);

const jive_subroutine_class JIVE_I386_SUBROUTINE = {
	.fini = jive_subroutine_fini_,
	.value_parameter = jive_i386_subroutine_value_parameter_,
	.value_return = jive_i386_subroutine_value_return_,
	.copy = jive_i386_subroutine_copy_,
	.prepare_stackframe = jive_i386_subroutine_prepare_stackframe_,
	.add_fp_dependency = jive_i386_subroutine_add_fp_dependency_,
	.add_sp_dependency = jive_i386_subroutine_add_sp_dependency_
};

static jive_output *
jive_i386_subroutine_value_parameter_(jive_subroutine * self_, size_t index)
{
	jive_i386_subroutine * self = (jive_i386_subroutine *) self_;
	jive_gate * gate = self->base.parameters[index];
	jive_output * output = jive_node_gate_output(&self->base.enter->base, gate);
	
	const jive_type * in_type = jive_gate_get_type(gate);
	const jive_type * out_type = jive_resource_class_get_type(&jive_i386_regcls[jive_i386_gpr].base);
	jive_node * node = jive_splitnode_create(self->base.enter->base.region,
		in_type, output, gate->required_rescls,
		out_type, &jive_i386_regcls[jive_i386_gpr].base);
	output = node->outputs[0];
	
	return output;
}

static jive_input *
jive_i386_subroutine_value_return_(jive_subroutine * self_, size_t index, jive_output * value)
{
	jive_i386_subroutine * self = (jive_i386_subroutine *) self_;
	jive_gate * gate = self->base.returns[index];
	return jive_node_gate_input(&self->base.leave->base, gate, value);
}

static jive_subroutine *
jive_i386_subroutine_copy_(const jive_subroutine * self_,
	jive_node * new_enter_node, jive_node * new_leave_node)
{
	jive_graph * graph = new_enter_node->region->graph;
	jive_context * context = graph->context;
	jive_i386_subroutine * self = (jive_i386_subroutine *) self_;
	
	jive_i386_subroutine * other = jive_context_malloc(context, sizeof(*other));
	jive_subroutine_init_(&other->base, &JIVE_I386_SUBROUTINE, context, &jive_i386_instructionset,
		self->base.nparameters, self->base.nreturns, 5);
	
	other->base.enter = (jive_subroutine_enter_node *) new_enter_node;
	other->base.leave = (jive_subroutine_leave_node *) new_leave_node;
	
	size_t n;
	
	for (n = 0; n < self->base.nparameters; n++) {
		jive_gate * old_gate = self->base.parameters[n];
		jive_gate * new_gate = NULL;
		
		if (!new_gate)
			new_gate = jive_subroutine_match_gate(old_gate, &self->base.enter->base, new_enter_node);
		if (!new_gate)
			new_gate = jive_subroutine_match_gate(old_gate, &self->base.leave->base, new_leave_node);
		if (!new_gate)
			new_gate = jive_resource_class_create_gate(old_gate->required_rescls, graph, old_gate->name);
		
		other->base.parameters[n] = new_gate;
	}
	
	for (n = 0; n < self->base.nreturns; n++) {
		jive_gate * old_gate = self->base.returns[n];
		jive_gate * new_gate = NULL;
		
		if (!new_gate)
			new_gate = jive_subroutine_match_gate(old_gate, &self->base.enter->base, new_enter_node);
		if (!new_gate)
			new_gate = jive_subroutine_match_gate(old_gate, &self->base.leave->base, new_leave_node);
		if (!new_gate)
			new_gate = jive_resource_class_create_gate(old_gate->required_rescls, graph, old_gate->name);
		
		other->base.returns[n] = new_gate;
	}
	
	for (n = 0; n < 5; ++n) {
		jive_subroutine_match_passthrough(
			&self->base, &self->base.passthroughs[n],
			&other->base, &self->base.passthroughs[n]);
	}
	
	return &other->base;
}

jive_subroutine *
jive_i386_subroutine_create(jive_region * region,
	size_t nparameters, const jive_argument_type parameters[],
	size_t nreturns, const jive_argument_type returns[])
{
	jive_graph * graph = region->graph;
	jive_context * context = graph->context;
	jive_i386_subroutine * self = jive_context_malloc(context, sizeof(*self));
	jive_subroutine_init_(&self->base, &JIVE_I386_SUBROUTINE, context, &jive_i386_instructionset,
		nparameters, nreturns, 5);
	self->base.frame.upper_bound = 4;
	
	size_t n;
	
	for (n = 0; n < nparameters; n++) {
		char argname[80];
		snprintf(argname, sizeof(argname), "arg%zd", n + 1);
		const jive_resource_class * cls;
		cls = jive_fixed_stackslot_class_get(4, 4, (n + 1) * 4);
		self->base.parameters[n] = jive_resource_class_create_gate(cls, graph, argname);
		self->base.frame.upper_bound = (n + 2) * 4;
	}
	
	for (n = 0; n < nreturns; n++) {
		char argname[80];
		snprintf(argname, sizeof(argname), "ret%zd", n + 1);
		const jive_resource_class * cls;
		switch (n) {
			case 0: cls = &jive_i386_regcls[jive_i386_gpr_eax].base; break;
			default: cls = jive_fixed_stackslot_class_get(4, 4, n * 4);
		}
		self->base.returns[n] = jive_resource_class_create_gate(cls, graph, argname);
	}
	
	jive_subroutine_create_region_and_nodes(&self->base, region);
	
	self->base.passthroughs[0] = jive_subroutine_create_passthrough(&self->base, &jive_i386_regcls[jive_i386_gpr_esp].base, "saved_esp");
	self->base.passthroughs[0].gate->may_spill = false;
	self->base.passthroughs[1] = jive_subroutine_create_passthrough(&self->base, &jive_i386_regcls[jive_i386_gpr_ebx].base, "saved_ebx");
	self->base.passthroughs[2] = jive_subroutine_create_passthrough(&self->base, &jive_i386_regcls[jive_i386_gpr_ebp].base, "saved_ebp");
	self->base.passthroughs[3] = jive_subroutine_create_passthrough(&self->base, &jive_i386_regcls[jive_i386_gpr_esi].base, "saved_esi");
	self->base.passthroughs[4] = jive_subroutine_create_passthrough(&self->base, &jive_i386_regcls[jive_i386_gpr_edi].base, "saved_edi");
	
	/* return instruction */
	jive_node * ret_instr = jive_instruction_node_create(self->base.region, &jive_i386_instructions[jive_i386_ret], NULL, NULL);
	
	/* add dependency on return address on stack */
	const jive_resource_class * stackslot_cls = jive_fixed_stackslot_class_get(4, 4, 0);
	const jive_type * memory_state_type = jive_resource_class_get_type(stackslot_cls);
	jive_output * retaddr_def = jive_node_add_output(&self->base.enter->base, memory_state_type);
	retaddr_def->required_rescls = stackslot_cls;
	jive_input * retaddr_use = jive_node_add_input(ret_instr, memory_state_type, retaddr_def);
	retaddr_use->required_rescls = stackslot_cls;
	
	/* divert control output of "leave" node */
	jive_input_divert_origin(self->base.leave->base.inputs[0], ret_instr->outputs[0]);

	return &self->base;
}

jive_subroutine *
jive_i386_subroutine_create_takeover(
	jive_context * context,
	size_t nparameters, jive_gate * const parameters[],
	size_t nreturns, jive_gate * const returns[],
	size_t npassthroughs, const jive_subroutine_passthrough passthroughs[])
{
	jive_i386_subroutine * self = jive_context_malloc(context, sizeof(*self));
	jive_subroutine_init_(&self->base, &JIVE_I386_SUBROUTINE, context, &jive_i386_instructionset,
		nparameters, nreturns, npassthroughs);
	self->base.frame.upper_bound = 4;
	
	size_t n;
	
	for (n = 0; n < nparameters; n++)
		self->base.parameters[n] = parameters[n];
	
	for (n = 0; n < nreturns; n++)
		self->base.returns[n] = returns[n];
	
	for (n = 0; n < npassthroughs; n++) {
		self->base.passthroughs[n] = passthroughs[n];
	}

	return &self->base;
}

typedef struct jive_i386_stackptr_split_factory {
	jive_value_split_factory base;
	int offset;
} jive_i386_stackptr_split_factory;

static jive_output *
do_stackptr_sub(const jive_value_split_factory * self_, jive_output * value)
{
	const jive_i386_stackptr_split_factory * self = (const jive_i386_stackptr_split_factory *) self_;
	int64_t immediates[1] = {self->offset};
	
	return jive_instruction_node_create_simple(
		value->node->region,
		&jive_i386_instructions[jive_i386_int_sub_immediate],
		&value, immediates)->outputs[0];
}

static jive_output *
do_stackptr_add(const jive_value_split_factory * self_, jive_output * value)
{
	const jive_i386_stackptr_split_factory * self = (const jive_i386_stackptr_split_factory *) self_;
	int64_t immediates[1] = {self->offset};
	
	return jive_instruction_node_create_simple(
		value->node->region,
		&jive_i386_instructions[jive_i386_int_add_immediate],
		&value, immediates)->outputs[0];
}

static void
jive_i386_subroutine_prepare_stackframe_(jive_subroutine * self_, const jive_subroutine_late_transforms * xfrm)
{
	jive_i386_subroutine * self = (jive_i386_subroutine *) self_;
	self->base.frame.lower_bound -= self->base.frame.call_area_size;
	
	if (!self->base.frame.lower_bound)
		return;
	
	self->base.frame.lower_bound = ((self->base.frame.lower_bound - 4) & ~15) + 4;
	
	jive_i386_stackptr_split_factory stackptr_sub = {
		{do_stackptr_sub},
		- self->base.frame.lower_bound
	};
	jive_i386_stackptr_split_factory stackptr_add = {
		{do_stackptr_add},
		- self->base.frame.lower_bound
	};
	
	/* as long as no frame pointer is used, access to stack slots through stack
	pointer must be relocated */
	self->base.frame.frame_pointer_offset += self->base.frame.lower_bound;
	
	xfrm->value_split(xfrm, self->base.passthroughs[0].output, self->base.passthroughs[0].input, &stackptr_sub.base, &stackptr_add.base);
}

static jive_input *
jive_i386_subroutine_add_fp_dependency_(const jive_subroutine * self_, jive_node * node)
{
	jive_i386_subroutine * self = (jive_i386_subroutine *) self_;
	jive_output * frameptr = self->base.passthroughs[0].output;
	
	size_t n;
	for (n = 0; n < node->ninputs; n++) {
		jive_input * input = node->inputs[n];
		if (input->origin == frameptr)
			return NULL;
	}
	return jive_node_add_input(node, jive_output_get_type(frameptr), frameptr);
}

static jive_input *
jive_i386_subroutine_add_sp_dependency_(const jive_subroutine * self_, jive_node * node)
{
	jive_i386_subroutine * self = (jive_i386_subroutine *) self_;
	jive_output * stackptr = self->base.passthroughs[0].output;
	
	size_t n;
	for (n = 0; n < node->ninputs; n++) {
		jive_input * input = node->inputs[n];
		if (input->origin == stackptr)
			return NULL;
	}
	return jive_node_add_input(node, jive_output_get_type(stackptr), stackptr);
}
