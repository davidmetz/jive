/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/comparison/bituless.h>

#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace bits {

ult_op::~ult_op() noexcept {}

bool
ult_op::operator==(const operation & other) const noexcept
{
	const ult_op * o = dynamic_cast<const ult_op *>(&other);
	return o && o->type() == type();
}
compare_result
ult_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	switch (arg1.ult(arg2)) {
		case '0': return compare_result::static_false;
		case '1': return compare_result::static_true;
		default: return compare_result::undecidable;
	}
}

jive_binary_operation_flags
ult_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
ult_op::debug_string() const
{
	return "BITULESS";
}

std::unique_ptr<jive::operation>
ult_op::copy() const
{
	return std::unique_ptr<jive::operation>(new ult_op(*this));
}

}
}

jive::oport *
jive_bituless(jive::oport * op1, jive::oport * op2)
{
	jive::region * region = op1->region();
	const jive::bits::type & type = dynamic_cast<const jive::bits::type &>(op1->type());
	return jive_node_create_normalized(region, jive::bits::ult_op(type), {op1, op2})[0];
}
