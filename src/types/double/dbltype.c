/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/double/dbltype.h>

#include <jive/vsdg/basetype-private.h>
#include <jive/vsdg/valuetype-private.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>

static void
jive_double_gate_init_(jive_double_gate * self, struct jive_graph * graph, const char name[]);

/* double type */

jive_double_type::~jive_double_type() noexcept {}

jive_double_type::jive_double_type() noexcept
	: jive_value_type(&JIVE_DOUBLE_TYPE)
{}

static jive_input *
jive_double_type_create_input_(const jive_type * self_, struct jive_node * node, size_t index,
	jive_output * initial_operand)
{
	return new jive_double_input(node, index, initial_operand);
}

static jive_output *
jive_double_type_create_output_(const jive_type * self_, struct jive_node * node, size_t index)
{
	return new jive_double_output(node, index);
}

static jive_gate *
jive_double_type_create_gate_(const jive_type * self_, struct jive_graph * graph, const char * name)
{
	return new jive_double_gate(graph, name);
}

static jive_type *
jive_double_type_copy_(const jive_type * self_)
{
	const jive_double_type * self = (const jive_double_type *) self_;
	jive_double_type * type = new jive_double_type;
	*type = *self;
	return type;
}

static inline void
jive_double_type_init_(jive_double_type * self)
{
	self->class_ = &JIVE_DOUBLE_TYPE;
}

const jive_type_class JIVE_DOUBLE_TYPE = {
	parent : &JIVE_VALUE_TYPE,
	name : "dbl",
	fini : jive_value_type_fini_, /* inherit */
	get_label : jive_type_get_label_, /* inherit */
	create_input : jive_double_type_create_input_, /* override */
	.create_output  = jive_double_type_create_output_, /* override */
	create_gate : jive_double_type_create_gate_, /* override */
	equals : jive_type_equals_, /* override */
	copy : jive_double_type_copy_, /* override */
};

/* double input */

jive_double_input::~jive_double_input() noexcept {}

jive_double_input::jive_double_input(struct jive_node * node, size_t index,
	jive_output * origin) noexcept
	: jive_value_input(&JIVE_DOUBLE_INPUT, node, index, origin)
{}

static const jive_type *
jive_double_input_get_type_(const jive_input * self_)
{
	const jive_double_input * self = (const jive_double_input *) self_;
	return &self->type();
}

const jive_input_class JIVE_DOUBLE_INPUT = {
	parent : &JIVE_VALUE_INPUT,
	fini : jive_input_fini_, /* inherit */
	get_label : jive_input_get_label_, /* inherit */
	get_type : jive_double_input_get_type_, /* override */
};

/* double output */

jive_double_output::~jive_double_output() noexcept {}

jive_double_output::jive_double_output(struct jive_node * node, size_t index)
	: jive_value_output(&JIVE_DOUBLE_OUTPUT, node, index)
{}

static const jive_type *
jive_double_output_get_type_(const jive_output * self_)
{
	const jive_double_output * self = (const jive_double_output *) self_;
	return &self->type();
}

const jive_output_class JIVE_DOUBLE_OUTPUT = {
	parent : &JIVE_VALUE_OUTPUT,
	fini : jive_output_fini_, /* inherit */
	get_label : jive_output_get_label_, /* inherit */
	get_type : jive_double_output_get_type_, /* override */
};

/* double gate */

jive_double_gate::~jive_double_gate() noexcept {}

jive_double_gate::jive_double_gate(jive_graph * graph, const char name[])
	: jive_value_gate(&JIVE_DOUBLE_GATE, graph, name)
{}

static const jive_type *
jive_double_gate_get_type_(const jive_gate * self_)
{
	const jive_double_gate * self = (const jive_double_gate *) self_;
	return &self->type();
}

const jive_gate_class JIVE_DOUBLE_GATE = {
	parent : &JIVE_VALUE_GATE,
	fini : jive_gate_fini_, /* inherit */
	get_label : jive_gate_get_label_, /* inherit */
	get_type : jive_double_gate_get_type_, /* override */
};
