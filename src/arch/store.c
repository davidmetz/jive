/*
 * Copyright 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/store.h>

#include <jive/arch/address-transform.h>
#include <jive/arch/address.h>
#include <jive/arch/addresstype.h>
#include <jive/rvsdg/graph.h>
#include <jive/types/bitstring/type.h>

namespace jive {

/* store operator */

store_op::~store_op() noexcept
{}

bool
store_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const store_op *>(&other);
	return op
	    && op->addresstype() == addresstype()
	    && op->valuetype() == valuetype()
	    && op->nresults() == nresults();
}

std::string
store_op::debug_string() const
{
	return "STORE";
}

std::unique_ptr<jive::operation>
store_op::copy() const
{
	return std::unique_ptr<jive::operation>(new store_op(*this));
}

std::vector<jive::port>
store_op::create_operands(
	const jive::valuetype & address,
	const jive::valuetype & value,
	size_t nstates)
{
	std::vector<jive::port> operands({address});
	operands.push_back(value);
	for (size_t n = 0; n < nstates; n++)
		operands.push_back({memtype()});

	return operands;
}

/* address store operator */

addrstore_op::~addrstore_op()
{}

/* bitstring store operator */

bitstore_op::~bitstore_op()
{}

}
