/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_VALUETYPE_H
#define JIVE_VSDG_VALUETYPE_H

#include <jive/vsdg/basetype.h>

typedef struct jive_value_type jive_value_type;

extern const jive_type_class JIVE_VALUE_TYPE;
class jive_value_type : public jive_type {
public:
	virtual ~jive_value_type() noexcept;

protected:
	jive_value_type(const jive_type_class * class_) noexcept;
};

class jive_value_input : public jive_input {
public:
	virtual ~jive_value_input() noexcept;

protected:
	jive_value_input(struct jive_node * node, size_t index, jive_output * initial_operand);
};

class jive_value_output : public jive_output {
public:
	virtual ~jive_value_output() noexcept;

protected:
	jive_value_output(struct jive_node * node, size_t index);
};

class jive_value_gate : public jive_gate {
public:
	virtual ~jive_value_gate() noexcept;

protected:
	jive_value_gate(jive_graph * graph, const char name[]);
};

#endif
