/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TESTS_TYPES_BITSTRING_COMPARISON_BITCMP_TEST_HELPERS_H
#define JIVE_TESTS_TYPES_BITSTRING_COMPARISON_BITCMP_TEST_HELPERS_H

#include <jive/vsdg/control.h>

static inline void
expect_static_true(jive::output * output)
{
	const jive::ctl::constant_op * op;
	op  = dynamic_cast<const jive::ctl::constant_op*>(&output->node()->operation());
	assert(op && op->value().nalternatives() == 2 && op->value().alternative() == 1);
}

static inline void
expect_static_false(jive::output * output)
{
	const jive::ctl::constant_op * op;
	op  = dynamic_cast<const jive::ctl::constant_op*>(&output->node()->operation());
	assert(op && op->value().nalternatives() == 2 && op->value().alternative() == 0);
}

#endif
