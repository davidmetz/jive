/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>

#include <jive/vsdg.h>
#include <jive/view.h>
#include <jive/types/bitstring.h>

static void
assert_constant(jive_output * bitstr, size_t nbits, const char bits[])
{
	jive_bitconstant_node * node = jive_bitconstant_node_cast(bitstr->node);
	assert(node);
	
	assert(node->attrs.nbits == nbits);
	assert(strncmp(node->attrs.bits, bits, nbits) == 0);
}

static int test_main(void)
{
	setlocale(LC_ALL, "");
	
	jive_context * context = jive_context_create();
	
	jive_graph * graph = jive_graph_create(context);
	
	jive_output * a = jive_bitconstant(graph, 4, "1100");
	jive_output * b = jive_bitconstant(graph, 4, "1010");
	
	jive_output * ops[] = {a, b};
	
	assert_constant(jive_bitand(2, ops), 4, "1000");
	assert_constant(jive_bitor(2, ops), 4, "1110");
	assert_constant(jive_bitxor(2, ops), 4, "0110");
	assert_constant(jive_bitsum(2, ops), 4, "0001");
	assert_constant(jive_bitmultiply(2, ops), 4, "1111");
	assert_constant(jive_bitconcat(2, ops), 8, "11001010");
	assert_constant(jive_bitnegate(a), 4, "1011");
	assert_constant(jive_bitnegate(b), 4, "1101");
	
	jive_graph_prune(graph);
	
	jive_output * x = jive_bitsymbolicconstant(graph, 16, "x");
	jive_output * y = jive_bitsymbolicconstant(graph, 16, "y");
	
	{
		jive_output * concat = jive_bitconcat(2, (jive_output * []){x, y});
		jive_output * slice = jive_bitslice(concat, 8, 24);
		jive_node * node = ((jive_output *) slice)->node;
		assert(node->class_ == &JIVE_BITCONCAT_NODE);
		assert(node->ninputs == 2);
		assert(node->inputs[0]->origin->node->class_ == &JIVE_BITSLICE_NODE);
		assert(node->inputs[1]->origin->node->class_ == &JIVE_BITSLICE_NODE);
		
		const jive_bitslice_node_attrs * attrs;
		attrs = (const jive_bitslice_node_attrs *) jive_node_get_attrs(node->inputs[0]->origin->node);
		assert( (attrs->low == 8) && (attrs->high == 16) );
		attrs = (const jive_bitslice_node_attrs *) jive_node_get_attrs(node->inputs[1]->origin->node);
		assert( (attrs->low == 0) && (attrs->high == 8) );
		
		assert(node->inputs[0]->origin->node->inputs[0]->origin == x);
		assert(node->inputs[1]->origin->node->inputs[0]->origin == y);
	}
	
	{
		jive_output * slice1 = jive_bitslice(x, 0, 8);
		jive_output * slice2 = jive_bitslice(x, 8, 16);
		jive_output * concat = jive_bitconcat(2, (jive_output *[]){slice1, slice2});
		assert(concat == x);
	}
	
	jive_graph_destroy(graph);
	
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("types/bitstring/test-reduction", test_main);
