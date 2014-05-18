/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/vsdg.h>
#include <jive/view.h>
#include <jive/types/bitstring.h>

#define ZERO_64 "00000000" "00000000" "00000000" "00000000" "00000000" "00000000" "00000000" "00000000"
#define ONE_64  "10000000" "00000000" "00000000" "00000000" "00000000" "00000000" "00000000" "00000000"
#define MONE_64 "11111111" "11111111" "11111111" "11111111" "11111111" "11111111" "11111111" "11111111"

static int test_main(void)
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	jive_output * b1 = jive_bitconstant(graph, 8, "00110011");
	jive_output * b2 = jive_bitconstant_unsigned(graph, 8, 204);
	jive_output * b3 = jive_bitconstant_signed(graph, 8, 204);
	jive_output * b4 = jive_bitconstant(graph, 9, "001100110");
	
	assert(jive_bitconstant_equals_unsigned((jive_bitconstant_node *) b1->node, 204));
	assert(jive_bitconstant_equals_signed((jive_bitconstant_node *) b1->node, -52));
	assert(!jive_bitconstant_equals_signed((jive_bitconstant_node *) b1->node, 204));

	assert(b1->node == b2->node);
	assert(b1->node == b3->node);
	
	assert(jive_bitconstant_node_to_unsigned(dynamic_cast<jive_bitconstant_node *>(b1->node)) == 204);
	assert(jive_bitconstant_node_to_signed(dynamic_cast<jive_bitconstant_node *>(b1->node)) == -52);
	assert(jive_bitconstant_node_to_unsigned(dynamic_cast<jive_bitconstant_node *>(b4->node)) == 204);
	assert(jive_bitconstant_node_to_signed(dynamic_cast<jive_bitconstant_node *>(b4->node)) == 204);
	
	jive_output * plus_one_128 = jive_bitconstant(graph, 128, ONE_64 ZERO_64);
	assert(jive_bitconstant_equals_unsigned((jive_bitconstant_node *) plus_one_128->node, 1));
	assert(jive_bitconstant_equals_signed((jive_bitconstant_node *) plus_one_128->node, 1));

	jive_output * minus_one_128 = jive_bitconstant(graph, 128, MONE_64 MONE_64);
	assert(!jive_bitconstant_equals_unsigned((jive_bitconstant_node *) minus_one_128->node,
		(uint64_t) -1LL));
	assert(jive_bitconstant_equals_signed((jive_bitconstant_node *) minus_one_128->node,
		(uint64_t) -1LL));

	jive_view(graph, stdout);
	jive_graph_destroy(graph);	
	
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/test-constant", test_main);
