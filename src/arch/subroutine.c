/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/subroutine.h>

#include <string.h>

#include <jive/arch/memorytype.h>
#include <jive/arch/subroutine/nodes.h>
#include <jive/common.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/resource.h>
#include <jive/vsdg/simple_node.h>
#include <jive/vsdg/structural_node.h>

namespace jive {

subroutine_hl_builder_interface::~subroutine_hl_builder_interface() noexcept
{
}

}

void
jive_subroutine_node_prepare_stackframe(
	jive::node * self,
	const jive::subroutine_op & op,
	jive_subroutine_stackframe_info * frame,
	const jive_subroutine_late_transforms * xfrm)
{
	jive::region * region = self->input(0)->origin()->region();
	return op.signature().abi_class->prepare_stackframe(
		op, region, frame, xfrm);
}

jive::simple_input *
jive_subroutine_node_add_fp_dependency(
	const jive::node * self,
	const jive::subroutine_op & op,
	jive::node * node)
{
	jive::region * region = self->input(0)->origin()->region();
	return op.signature().abi_class->add_fp_dependency(
		op, region, node);
}

jive::simple_input *
jive_subroutine_node_add_sp_dependency(
	const jive::node * self,
	const jive::subroutine_op & op,
	jive::node * node)
{
	jive::region * region = self->input(0)->origin()->region();
	return op.signature().abi_class->add_sp_dependency(
		op, region, node);
}

jive::node *
jive_region_get_subroutine_node(const jive::region * region)
{
	for (; region->node(); region = region->node()->region()) {
		if (dynamic_cast<const jive::subroutine_op*>(&region->node()->operation()))
			return region->node();
	}

	return nullptr;
}

const struct jive_instructionset *
jive_region_get_instructionset(const jive::region * region)
{
	jive::node * sub = jive_region_get_subroutine_node(region);
	if (sub) {
		return static_cast<const jive::subroutine_op &>(sub->operation())
			.signature().abi_class->instructionset;
	} else {
		return NULL;
	}
}

jive::simple_output *
jive_subroutine_node_get_sp(const jive::node * self)
{
	jive::region * region = self->input(0)->origin()->region();
	return static_cast<const jive::subroutine_op &>(self->operation())
		.get_passthrough_enter_by_index(region, 1);
}

jive::simple_output *
jive_subroutine_node_get_fp(const jive::node * self)
{
	jive::region * region = self->input(0)->origin()->region();
	/* FIXME: this is only correct if we are compiling "omit-framepointer",
	but it is only a transitionary stage during subroutine refactoring */
	return static_cast<const jive::subroutine_op &>(self->operation())
		.get_passthrough_enter_by_index(region, 1);
}

jive_subroutine
jive_subroutine_begin(
	jive::graph * graph,
	jive::subroutine_machine_signature sig,
	std::unique_ptr<jive::subroutine_hl_builder_interface> hl_builder)
{
	jive_subroutine sub;
	sub.hl_builder = std::move(hl_builder);
	sub.builder_state.reset(new jive::subroutine_builder_state(sig));
	sub.node = graph->root()->add_structural_node(jive::subroutine_op(std::move(sig)), 1);
	sub.signature = static_cast<const jive::subroutine_op*>(&sub.node->operation())->signature();
	sub.region = sub.node->subregion(0);

	for (size_t n = 0; n < sub.signature.arguments.size(); n++) {
		auto rescls = sub.signature.arguments[n].rescls;
		sub.builder_state->arguments[n].gate = graph->create_gate(sub.signature.arguments[n].name,
			rescls);
		sub.builder_state->arguments[n].output = sub.region->add_argument(nullptr,
			sub.builder_state->arguments[n].gate);
	}

	for (size_t n = 0; n < sub.signature.results.size(); n++) {
		auto rescls = sub.signature.results[n].rescls;
		sub.builder_state->results[n].gate = graph->create_gate(sub.signature.results[n].name, rescls);
	}

	for (size_t n = 0; n < sub.signature.passthroughs.size(); n++) {
		if (sub.signature.passthroughs[n].rescls) {
			auto rescls = sub.signature.passthroughs[n].rescls;
			sub.builder_state->passthroughs[n].gate = graph->create_gate(
				sub.signature.passthroughs[n].name, rescls);
		} else {
			sub.builder_state->passthroughs[n].gate = graph->create_gate(
				sub.signature.passthroughs[n].name, jive::mem::type::instance());
		}

		sub.builder_state->passthroughs[n].gate->may_spill = sub.signature.passthroughs[n].may_spill;
		sub.builder_state->passthroughs[n].output = sub.region->add_argument(nullptr,
			sub.builder_state->passthroughs[n].gate);
	}

	return sub;
}

jive::node *
jive_subroutine_end(jive_subroutine & self)
{
	auto ctl_return = self.hl_builder->finalize(self);
	self.region->add_result(ctl_return, nullptr, ctl_return->type());

	for (const auto & pt : self.builder_state->passthroughs)
		self.region->add_result(pt.output, nullptr, pt.gate);

	for (const auto & res : self.builder_state->results)
		self.region->add_result(res.output, nullptr, res.gate);

	self.node->add_output(&jive::mem::type::instance());

	return self.node;
}

jive::oport *
jive_subroutine_simple_get_argument(
	jive_subroutine & self,
	size_t index)
{
	return self.hl_builder->value_parameter(self, index);
}

void
jive_subroutine_simple_set_result(
	jive_subroutine & self,
	size_t index,
	jive::simple_output * value)
{
	self.hl_builder->value_return(self, index, value);
}

jive::oport *
jive_subroutine_simple_get_global_state(const jive_subroutine & self)
{
	return self.builder_state->passthroughs[0].output;
}

void
jive_subroutine_simple_set_global_state(jive_subroutine & self, jive::simple_output * state)
{
	self.builder_state->passthroughs[0].output = state;
}
