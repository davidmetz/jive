/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>

#include <jive/context.h>
#include <jive/vsdg.h>
#include <jive/types/bitstring.h>
#include <jive/types/record/rcdgroup.h>

static int test_main(void)
{
	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	static const jive_bitstring_type bits8 = {{{&JIVE_BITSTRING_TYPE}}, 8};
	
	static const jive_value_type * l_elements[] = { &bits8.base };
	static const jive_record_declaration l = {1, l_elements};

	jive_output * c = jive_bitconstant(graph, 8, "11110000");
	jive_output * g = jive_group_create(&l, 1, &c);

	const jive_group_node_attrs * attrs = (const jive_group_node_attrs *)
		jive_node_get_attrs(g->node);
	assert(attrs);
	assert(attrs->decl);

	jive_graph_destroy(graph);

	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("record/test-group-attrs", test_main);
