/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/statetype.h>

#include <jive/common.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/simple_node.h>

namespace jive {
namespace state {

mux_op::~mux_op() noexcept {}

bool
mux_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const mux_op*>(&other);
	return op
	    && op->narguments_ == narguments_
	    && op->nresults_ == nresults_
	    && op->port_ == port_;
}

size_t
mux_op::narguments() const noexcept
{
	return narguments_;
}

const jive::port &
mux_op::argument(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < narguments());
	return port_;
}

size_t
mux_op::nresults() const noexcept
{
	return nresults_;
}

const jive::port &
mux_op::result(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	return port_;
}

std::string
mux_op::debug_string() const
{
	return "STATEMUX";
}

std::unique_ptr<jive::operation>
mux_op::copy() const
{
	return std::unique_ptr<jive::operation>(new mux_op(*this));
}

}
}

jive::output *
jive_state_merge(const jive::state::type * statetype, size_t nstates, jive::output * const states[])
{
	jive::region * region = states[0]->region();

	jive::state::mux_op op(*statetype, nstates, 1);
	return jive::create_normalized(region, op,
		std::vector<jive::output*>(states, states + nstates))[0];
}

std::vector<jive::output*>
jive_state_split(const jive::state::type * statetype, jive::output * state, size_t nstates)
{
	jive::state::mux_op op(*statetype, 1, nstates);
	return jive::create_normalized(state->region(), op, {state});
}
