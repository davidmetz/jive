/*
 * Copyright 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * Copyright 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */
#include "test-registry.h"

#include <assert.h>
#include <locale.h>

#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/phi.h>
#include <jive/types/function/fctlambda.h>
#include <jive/types/function/fctapply.h>
#include <jive/vsdg/node-private.h>

static int test_main()
{
	setlocale(LC_ALL, "");

	jive_context * context = jive_context_create();
	jive_graph * graph = jive_graph_create(context);

	JIVE_DECLARE_VALUE_TYPE(vtype);
	jive_function_type * f0type = jive_function_type_create(context, 0, NULL, 0, NULL);
	jive_function_type * f1type = jive_function_type_create(context, 0, NULL, 0, NULL);	
	jive_function_type * f2type = jive_function_type_create(context, 1, &vtype,
		1, (const jive_type *[]){vtype});

	jive_phi phi = jive_phi_begin(graph);
	jive_phi_fixvar fns[3];
	fns[0] = jive_phi_fixvar_enter(phi, &f0type->base.base);
	fns[1] = jive_phi_fixvar_enter(phi, &f1type->base.base);
	fns[2] = jive_phi_fixvar_enter(phi, &f2type->base.base);

	jive_lambda * l0 = jive_lambda_begin(graph, 0, NULL, NULL);
	jive_lambda * l1 = jive_lambda_begin(graph, 0, NULL, NULL);
	jive_lambda * l2 = jive_lambda_begin(graph, 1, &vtype, (const char *[]){"arg"});

	jive_output * lambda0 = jive_lambda_end(l0, 0, NULL, NULL);
	jive_output * lambda1 = jive_lambda_end(l1, 0, NULL, NULL);

	jive_output * ret;
	jive_apply_create(fns[2].value, 1, l2->arguments, &ret);

	jive_output * lambda2 = jive_lambda_end(l2, 1, &vtype, &ret);

	jive_phi_fixvar_leave(phi, fns[0].gate, lambda0);
	jive_phi_fixvar_leave(phi, fns[1].gate, lambda1);
	jive_phi_fixvar_leave(phi, fns[2].gate, lambda2);

	jive_phi_end(phi, 3, fns);

	jive_output * results[3] = {fns[0].value, fns[1].value, fns[2].value};

	jive_node * bottom = jive_node_create(graph->root_region,
		3, (const jive_type *[]){&f0type->base.base, &f1type->base.base, &f2type->base.base}, results,
		1, &vtype);
	jive_graph_export(graph, bottom->outputs[0]);

	jive_graph_normalize(graph);
	jive_graph_prune(graph);

	jive_view(graph, stderr);

	const jive_lambda_node * lambda_node2;
	lambda_node2 = jive_lambda_node_const_cast(phi.region->bottom->inputs[3]->origin->node);
	assert(jive_lambda_is_self_recursive(lambda_node2));
	assert(jive_input_isinstance(phi.region->bottom->inputs[0], &JIVE_CONTROL_INPUT));

	jive_function_type_destroy(f0type);
	jive_function_type_destroy(f1type);
	jive_function_type_destroy(f2type);
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(context));
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/test-phi", test_main);
