#include <jive/vsdg/valuetype.h>
#include <jive/vsdg/valuetype-private.h>
#include <jive/vsdg/basetype-private.h>

#include <jive/vsdg/node.h>
#include <jive/vsdg/graph.h>

const jive_type_class JIVE_VALUE_TYPE = {
	.parent = &JIVE_TYPE,
	.get_label = _jive_type_get_label, /* inherit */
	.create_input = _jive_value_type_create_input, /* override */
	.create_output = _jive_value_type_create_output, /* override */
	.create_resource = _jive_value_type_create_resource, /* override */
	.create_gate = _jive_value_type_create_gate, /* override */
	.equals = _jive_type_equals, /* inherit */
	.accepts = _jive_type_accepts /* inherit */
};

const jive_type jive_value_type_singleton = {
	.class_ = &JIVE_VALUE_TYPE
};

const jive_input_class JIVE_VALUE_INPUT = {
	.parent = &JIVE_INPUT,
	.fini = _jive_input_fini, /* inherit */
	.get_label = _jive_input_get_label, /* inherit */
	.get_type = _jive_value_input_get_type, /* override */
	.get_constraint = _jive_value_input_get_constraint /* override */
};

const jive_output_class JIVE_VALUE_OUTPUT = {
	.parent = &JIVE_OUTPUT,
	.fini = _jive_output_fini, /* inherit */
	.get_label = _jive_output_get_label, /* inherit */
	.get_type = _jive_value_output_get_type, /* override */
	.get_constraint = _jive_value_output_get_constraint /* override */
};

const jive_gate_class JIVE_VALUE_GATE = {
	.parent = &JIVE_GATE,
	.fini = _jive_gate_fini, /* inherit */
	.get_label = _jive_gate_get_label, /* inherit */
	.get_type = _jive_value_gate_get_type /* override */
};

const jive_resource_class JIVE_VALUE_RESOURCE = {
	.parent = &JIVE_RESOURCE,
	.fini = _jive_resource_fini, /* inherit */
	.get_label = _jive_resource_get_label, /* inherit */
	.get_type = _jive_value_resource_get_type, /* override */
	.can_merge = _jive_resource_can_merge, /* inherit */
	.merge = _jive_value_resource_merge, /* override */
	.get_cpureg = _jive_value_resource_get_cpureg, /* override */
	.get_regcls = _jive_value_resource_get_regcls, /* override */
	.get_real_regcls = _jive_value_resource_get_real_regcls /* override */
};

jive_input *
_jive_value_type_create_input(const jive_type * self, struct jive_node * node, size_t index, jive_output * initial_operand)
{
	jive_value_input * input = jive_context_malloc(node->graph->context, sizeof(*input));
	input->base.class_ = &JIVE_VALUE_INPUT;
	_jive_value_input_init(input, node, index, initial_operand);
	return &input->base; 
}

jive_output *
_jive_value_type_create_output(const jive_type * self, struct jive_node * node, size_t index)
{
	jive_value_output * output = jive_context_malloc(node->graph->context, sizeof(*output));
	output->base.class_ = &JIVE_VALUE_OUTPUT;
	_jive_value_output_init(output, node, index);
	return &output->base; 
}

jive_resource *
_jive_value_type_create_resource(const jive_type * self, struct jive_graph * graph)
{
	jive_value_resource * resource = jive_context_malloc(graph->context, sizeof(*resource));
	resource->base.class_ = &JIVE_VALUE_RESOURCE;
	_jive_value_resource_init(resource, graph);
	return &resource->base; 
}

jive_gate *
_jive_value_type_create_gate(const jive_type * self, struct jive_graph * graph, const char * name)
{
	jive_value_gate * gate = jive_context_malloc(graph->context, sizeof(*gate));
	gate->base.class_ = &JIVE_VALUE_GATE;
	_jive_value_gate_init(gate, graph, name);
	return &gate->base; 
}

/* value inputs */

void
_jive_value_input_init(jive_value_input * self, struct jive_node * node, size_t index, jive_output * origin)
{
	_jive_input_init(&self->base, node, index, origin);
	self->required_regcls = 0;
}

const jive_type *
_jive_value_input_get_type(const jive_input * self)
{
	return &jive_value_type_singleton;
}

jive_resource *
_jive_value_input_get_constraint(const jive_input * self_)
{
	jive_value_input * self = (jive_value_input *) self_;
	jive_value_resource * resource = (jive_value_resource *) _jive_input_get_constraint(self_);
	/* don't overwrite regcls specified by gate (should be done differently though) */
	if (!self->base.gate) jive_value_resource_set_regcls(resource, self->required_regcls);
	return &resource->base;
}

/* value outputs */

void
_jive_value_output_init(jive_value_output * self, struct jive_node * node, size_t index)
{
	_jive_output_init(&self->base, node, index);
	self->required_regcls = 0;
}

const jive_type *
_jive_value_output_get_type(const jive_output * self)
{
	return &jive_value_type_singleton;
}

jive_resource *
_jive_value_output_get_constraint(const jive_output * self_)
{
	jive_value_output * self = (jive_value_output *) self_;
	jive_value_resource * resource = (jive_value_resource *) _jive_output_get_constraint(self_);
	/* don't overwrite regcls specified by gate (should be done differently though) */
	if (!self->base.gate) jive_value_resource_set_regcls(resource, self->required_regcls);
	return &resource->base;
}

/* value resource, i.e. registers */

void
_jive_value_resource_init(jive_value_resource * self, struct jive_graph * graph)
{
	_jive_resource_init(&self->base, graph);
	self->cpureg = 0;
	self->regcls = 0;
}

const jive_type *
_jive_value_resource_get_type(const jive_resource * self)
{
	return &jive_value_type_singleton;
}

bool
_jive_value_resource_can_merge(const jive_resource * self, const jive_resource * other);

void
_jive_value_resource_merge(jive_resource * self, jive_resource * other);

const struct jive_cpureg *
_jive_value_resource_get_cpureg(const jive_resource * self_)
{
	jive_value_resource * self = (jive_value_resource *) self_;
	return self->cpureg;
}

const struct jive_regcls *
_jive_value_resource_get_regcls(const jive_resource * self_)
{
	jive_value_resource * self = (jive_value_resource *) self_;
	return self->regcls;
}

const struct jive_regcls *
_jive_value_resource_get_real_regcls(const jive_resource * self_)
{
	jive_value_resource * self = (jive_value_resource *) self_;
	if (self->cpureg) return self->cpureg->regcls;
	else return self->regcls;
}

/* value gates */

void
_jive_value_gate_init(jive_value_gate * self, struct jive_graph * graph, const char * name)
{
	_jive_gate_init(&self->base, graph, name);
	self->required_regcls = 0;
}

const jive_type *
_jive_value_gate_get_type(const jive_gate * self)
{
	return &jive_value_type_singleton;
}

