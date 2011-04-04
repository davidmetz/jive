#include <jive/vsdg/function.h>

#include <stdio.h>
#include <string.h>

#include <jive/common.h>
#include <jive/context.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/substitution.h>
#include <jive/vsdg/traverser.h>

static void
_jive_apply_node_init(jive_apply_node * self, struct jive_region * region, jive_output * function,
	size_t narguments, jive_output * const arguments[]);

static jive_node *
_jive_apply_node_create(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

static char *
_jive_apply_node_get_label(const jive_node * self_);

const jive_node_class JIVE_APPLY_NODE = {
	.parent = &JIVE_NODE,
	.fini = _jive_node_fini, /* inherit */
	.get_label = _jive_apply_node_get_label, /* override */
	.get_attrs = _jive_node_get_attrs, /* inherit */
	.match_attrs = _jive_node_match_attrs, /* inherit */
	.create = _jive_apply_node_create, /* override */
	.get_aux_rescls = _jive_node_get_aux_rescls /* inherit */
};

static char *
_jive_apply_node_get_label(const jive_node * self_)
{
	return strdup("APPLY");
}

static jive_node *
_jive_apply_node_create(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[])
{
	return jive_apply_node_create(region, operands[0], noperands - 1, &operands[1]);
}

static void
_jive_apply_node_init(
	jive_apply_node * self,
	struct jive_region * region,
	jive_output * function,
	size_t narguments,
	jive_output * const arguments[])
{
	//FIXME: this is ugly, some other sort of error would be nice
	JIVE_DEBUG_ASSERT(function->class_ == &JIVE_FUNCTION_OUTPUT);
	jive_function_output * fct = (jive_function_output *) function;

	//FIXME: this is ugly, some other sort of error would be nice
	JIVE_DEBUG_ASSERT(fct->type.narguments == narguments);

	jive_output * args[narguments+1];
	const jive_type * argument_types[narguments+1];
	args[0] = function;
	argument_types[0] = &fct->type.base.base; 
 
	size_t i;
	for(i = 0; i < fct->type.narguments; i++){
		argument_types[i+1] = fct->type.argument_types[i];
		args[i+1] = arguments[i];
	}
	
	_jive_node_init(self, region,
		narguments + 1, argument_types, args,
		fct->type.nreturns, (const jive_type * const *) fct->type.return_types);
}

jive_node *
jive_apply_node_create(struct jive_region * region, jive_output * function,
	size_t narguments, jive_output * const arguments[])
{
	jive_apply_node * node = jive_context_malloc(region->graph->context, sizeof( * node));

	node->class_ = &JIVE_APPLY_NODE;
	_jive_apply_node_init(node, region, function, narguments, arguments);

	return node; 
}

static void
_jive_symbolicfunction_node_fini(jive_node * self_);

static char *
_jive_symbolicfunction_node_get_label(const jive_node * self_);

static const jive_node_attrs *
_jive_symbolicfunction_node_get_attrs(const jive_node * self);

static bool
_jive_symbolicfunction_node_match_attrs(const jive_node * self, const jive_node_attrs * attrs);

static jive_node *
_jive_symbolicfunction_node_create(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[]);

const jive_node_class JIVE_SYMBOLICFUNCTION_NODE = {
	.parent = &JIVE_NODE,
	.fini = _jive_symbolicfunction_node_fini, /* override */
	.get_label = _jive_symbolicfunction_node_get_label, /* override */
	.get_attrs = _jive_symbolicfunction_node_get_attrs, /* inherit */
	.match_attrs = _jive_symbolicfunction_node_match_attrs, /* override */
	.create = _jive_symbolicfunction_node_create, /* override */
	.get_aux_rescls = _jive_node_get_aux_rescls /* inherit */
};

static void
_jive_symbolicfunction_node_init(
	jive_symbolicfunction_node * node,
	jive_graph * graph,
	const char * fctname,
	const jive_function_type * type)
{
	node->attrs.name = jive_context_strdup(graph->context, fctname);
	jive_function_type_init(&node->attrs.type, graph->context,
		type->narguments, (const jive_type **) type->argument_types,
		type->nreturns, (const jive_type **) type->return_types);

	const jive_type * rtype = &type->base.base;
	_jive_node_init(&node->base, graph->root_region,
		0, NULL, NULL,
		1, &rtype);
}

static void
_jive_symbolicfunction_node_fini(jive_node * self_)
{
	jive_symbolicfunction_node * self = (jive_symbolicfunction_node *) self_;
	
	jive_context_free(self_->graph->context, (char *)self->attrs.name);
	
	jive_function_type_fini(&self->attrs.type);
	
	_jive_node_fini(&self->base);
}

static char *
_jive_symbolicfunction_node_get_label(const jive_node * self_)
{
	const jive_symbolicfunction_node * self = (const jive_symbolicfunction_node *) self_;
	
	return strdup(self->attrs.name);
}

static jive_node *
_jive_symbolicfunction_node_create(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	const jive_symbolicfunction_node_attrs * attrs = (const jive_symbolicfunction_node_attrs *) attrs_;
	return jive_symbolicfunction_node_create(region->graph, attrs->name, &attrs->type);
}

static const jive_node_attrs *
_jive_symbolicfunction_node_get_attrs(const jive_node * self_)
{
	const jive_symbolicfunction_node * self = (const jive_symbolicfunction_node *) self_;
	
	return &self->attrs.base;
}

static bool
_jive_symbolicfunction_node_match_attrs(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive_symbolicfunction_node_attrs * first = &((const jive_symbolicfunction_node *)self)->attrs;
	const jive_symbolicfunction_node_attrs * second = (const jive_symbolicfunction_node_attrs *) attrs;

	if (!jive_type_equals(&first->type.base.base, &second->type.base.base)) return false;
	if (strcmp(first->name, second->name)) return false;

	return true;
}

jive_node *
jive_symbolicfunction_node_create(struct jive_graph * graph, const char * name, const jive_function_type * type) 
{
	jive_symbolicfunction_node * node = jive_context_malloc(graph->context, sizeof(* node));
	node->base.class_ = &JIVE_SYMBOLICFUNCTION_NODE;
	_jive_symbolicfunction_node_init(node, graph, name, type);
	return &node->base;
} 

jive_output *
jive_symbolicfunction_create(struct jive_graph * graph, const char * name, const jive_function_type * type)
{
	return jive_symbolicfunction_node_create(graph, name, type)->outputs[0];
}

jive_node *
jive_enter_node_create(jive_region * region);

jive_node *
jive_leave_node_create(jive_output * output);

static jive_node *
jive_enter_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[])
{
	return jive_enter_node_create(region);
}

static jive_node *
jive_leave_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[])
{
	return jive_leave_node_create(operands[0]);
}

const jive_node_class JIVE_ENTER_NODE = {
	.parent = &JIVE_NODE,
	.name = "ENTER",
	.fini = _jive_node_fini, /* inherit */
	.get_label = _jive_node_get_label, /* inherit */
	.get_attrs = _jive_node_get_attrs, /* inherit */
	.match_attrs = _jive_node_match_attrs, /* inherit */
	.create = jive_enter_node_create_, /* override */
	.get_aux_rescls = _jive_node_get_aux_rescls /* inherit */
};

const jive_node_class JIVE_LEAVE_NODE = {
	.parent = &JIVE_NODE,
	.name = "LEAVE",
	.fini = _jive_node_fini, /* inherit */
	.get_label = _jive_node_get_label, /* inherit */
	.get_attrs = _jive_node_get_attrs, /* inherit */
	.match_attrs = _jive_node_match_attrs, /* inherit */
	.create = jive_leave_node_create_, /* override */
	.get_aux_rescls = _jive_node_get_aux_rescls /* inherit */
};

jive_node *
jive_enter_node_create(jive_region * region)
{
	JIVE_DEBUG_ASSERT(region->top == NULL && region->bottom == NULL);
	jive_node * node = jive_context_malloc(region->graph->context, sizeof(*node));
	
	node->class_ = &JIVE_ENTER_NODE;
	JIVE_DECLARE_CONTROL_TYPE(ctl);
	_jive_node_init(node, region,
		0, NULL, NULL,
		1, &ctl);
	region->top = node;
	
	return node;
}

jive_node *
jive_leave_node_create(jive_output * output)
{
	jive_node * node = jive_context_malloc(output->node->graph->context, sizeof(*node));
	
	node->class_ = &JIVE_LEAVE_NODE;
	JIVE_DECLARE_CONTROL_TYPE(ctl);
	JIVE_DECLARE_ANCHOR_TYPE(anchor);
	_jive_node_init(node, output->node->region,
		1, &ctl, &output,
		1, &anchor);
	output->node->region->bottom = node;
	
	return node;
}

jive_region *
jive_function_region_create(jive_region * parent)
{
	jive_region * region = jive_region_create_subregion(parent);
	jive_node * enter = jive_enter_node_create(region);
	jive_leave_node_create(enter->outputs[0]);
	
	return region;
}

static void
jive_lambda_node_fini_(jive_node * self_);

static const jive_node_attrs *
jive_lambda_node_get_attrs_(const jive_node * self);

static bool
jive_lambda_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs);

static jive_node *
jive_lambda_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[]);

const jive_node_class JIVE_LAMBDA_NODE = {
	.parent = &JIVE_NODE,
	.name = "LAMBDA",
	.fini = jive_lambda_node_fini_, /* override */
	.get_label = _jive_node_get_label, /* inherit */
	.get_attrs = jive_lambda_node_get_attrs_, /* inherit */
	.match_attrs = jive_lambda_node_match_attrs_, /* override */
	.create = jive_lambda_node_create_, /* override */
	.get_aux_rescls = _jive_node_get_aux_rescls /* inherit */
};

static void
jive_lambda_node_init_(jive_lambda_node * self, jive_region * function_region)
{
	jive_region * region = function_region->parent;
	jive_context * context = function_region->graph->context;
	
	size_t narguments = function_region->top->noutputs - 1;
	size_t nreturns = function_region->bottom->ninputs - 1;
	size_t n;
	
	const jive_type * argument_types[narguments];
	for(n = 0; n < narguments; n++)
		argument_types[n] = jive_output_get_type(function_region->top->outputs[n+1]);
	const jive_type * return_types[nreturns];
	for(n = 0; n < nreturns; n++)
		return_types[n] = jive_input_get_type(function_region->bottom->inputs[n+1]);
	
	jive_function_type_init(&self->attrs.function_type, context,
		narguments, argument_types,
		nreturns, return_types);
	
	JIVE_DECLARE_ANCHOR_TYPE(anchor_type);
	
	const jive_type * function_type = &self->attrs.function_type.base.base;
	_jive_node_init(&self->base, region,
		1, &anchor_type, &function_region->bottom->outputs[0],
		1, &function_type);
	
	self->attrs.argument_gates = jive_context_malloc(context, narguments * sizeof(jive_gate *));
	self->attrs.return_gates = jive_context_malloc(context, nreturns * sizeof(jive_gate *));
	for(n = 0; n < narguments; n++)
		self->attrs.argument_gates[n] = function_region->top->outputs[n+1]->gate;
	for(n = 0; n < nreturns; n++)
		self->attrs.return_gates[n] = function_region->bottom->inputs[n+1]->gate;
}

static void
jive_lambda_node_fini_(jive_node * self_)
{
	jive_context * context = self_->graph->context;
	jive_lambda_node * self = (jive_lambda_node *) self_;
	
	jive_function_type_fini(&self->attrs.function_type);
	jive_context_free(context, self->attrs.argument_gates);
	jive_context_free(context, self->attrs.return_gates);
	
	_jive_node_fini(&self->base);
}

static jive_node *
jive_lambda_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	return jive_lambda_node_create(operands[0]->node->region);
}

static const jive_node_attrs *
jive_lambda_node_get_attrs_(const jive_node * self_)
{
	const jive_lambda_node * self = (const jive_lambda_node *) self_;
	
	return &self->attrs.base;
}

static bool
jive_lambda_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive_lambda_node_attrs * first = &((const jive_lambda_node *)self)->attrs;
	const jive_lambda_node_attrs * second = (const jive_lambda_node_attrs *) attrs;

	if (!jive_type_equals(&first->function_type.base.base, &second->function_type.base.base)) return false;
	size_t n;
	for(n = 0; n < first->function_type.narguments; n++)
		if (first->argument_gates[n] != second->argument_gates[n]) return false;
	for(n = 0; n < first->function_type.nreturns; n++)
		if (first->return_gates[n] != second->return_gates[n]) return false;

	return true;
}

jive_node *
jive_lambda_node_create(jive_region * function_region)
{
	jive_lambda_node * node = jive_context_malloc(function_region->graph->context, sizeof(*node));
	node->base.class_ = &JIVE_LAMBDA_NODE;
	jive_lambda_node_init_(node, function_region);
	return &node->base;
}

jive_output *
jive_lambda_create(jive_region * function_region)
{
	return jive_lambda_node_create(function_region)->outputs[0];
}

void
jive_inline_lambda_apply(jive_node * apply_node)
{
	jive_lambda_node * lambda_node = jive_lambda_node_cast(apply_node->inputs[0]->origin->node);
	assert(lambda_node);
	if (!lambda_node)
		return;
	
	jive_region * function_region = lambda_node->base.inputs[0]->origin->node->region;
	jive_node * head = function_region->top;
	jive_node * tail = function_region->bottom;
	
	jive_substitution_map * substitution = jive_substitution_map_create(apply_node->graph->context);
	
	size_t n;
	for(n = 0; n < lambda_node->attrs.function_type.narguments; n++) {
		jive_gate * gate = lambda_node->attrs.argument_gates[n];
		jive_output * output = jive_node_get_gate_output(head, gate);
		jive_substitution_map_add_output(substitution, output, apply_node->inputs[n+1]->origin);
	}
	
	jive_region_copy_substitute(function_region,
		apply_node->region, substitution, false, false);
	
	for(n = 0; n < lambda_node->attrs.function_type.nreturns; n++) {
		jive_gate * gate = lambda_node->attrs.return_gates[n];
		jive_input * input = jive_node_get_gate_input(tail, gate);
		jive_output * substituted = jive_substitution_map_lookup_output(substitution, input->origin);
		jive_output * output = apply_node->outputs[n];
		jive_output_replace(output, substituted);
	}
	
	jive_substitution_map_destroy(substitution);
}
