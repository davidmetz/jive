#ifndef JIVE_VSDG_NODE_H
#define JIVE_VSDG_NODE_H

#include <stdlib.h>
#include <stdbool.h>

#include <jive/vsdg/crossings.h>
#include <jive/vsdg/regcls-count.h>

typedef struct jive_node jive_node;
typedef struct jive_node_attrs jive_node_attrs;
typedef struct jive_node_class jive_node_class;

struct jive_graph;
struct jive_input;
struct jive_type;
struct jive_output;
struct jive_gate;
struct jive_region;
struct jive_traversal_nodestate;
struct jive_node_location;

struct jive_node {
	const struct jive_node_class * class_;
	
	struct jive_graph * graph;
	
	struct jive_region * region;
	
	int depth_from_root;
	size_t nsuccessors;
	size_t ninputs;
	size_t noperands;
	size_t noutputs;
	size_t reserved;
	
	struct jive_input ** inputs;
	struct jive_output ** outputs;
	
	struct jive_node_location * shape_location;
	
	jive_resource_interaction resource_interaction;
	jive_regcls_count use_count_before;
	jive_regcls_count use_count_after;
	
	struct {
		struct jive_region * first;
		struct jive_region * last;
	} anchored_regions;
	
	struct {
		jive_node * prev;
		jive_node * next;
	} region_nodes_list;
	
	struct {
		jive_node * prev;
		jive_node * next;
	} graph_top_list;
	
	struct {
		jive_node * prev;
		jive_node * next;
	} graph_bottom_list;
	
	size_t ntraverser_slots;
	struct jive_traversal_nodestate ** traverser_slots;
};

extern const jive_node_class JIVE_NODE;

struct jive_node_attrs {
	/* empty, need override */
};

typedef enum jive_node_type_flags {
	jive_node_class_associative = 1,
	jive_node_class_commutative = 2
} jive_node_class_flags;

struct jive_node_class {
	const struct jive_node_class * parent;
	const char * name;
	jive_node_class_flags flags;
	
	void (*fini)(jive_node * self);
	
	/** \brief Give textual representation of node (for debugging) */
	char * (*get_label)(const jive_node * self);
	
	/** \brief Retrieve attributes of node */
	const jive_node_attrs * (*get_attrs)(const jive_node * self);
	
	/** \brief Class method, create node with given attributes */
	jive_node * (*create)(const jive_node_attrs * attrs,
		struct jive_region * region,
		size_t noperands, struct jive_output * operands[]);
	
	/** \brief Class method, compare attributes for equality */
	bool (*equiv)(const jive_node_attrs * first, const jive_node_attrs * second);
	
	/** \brief Class method, determine whether operands can be reduced */
	bool (*can_reduce)(const struct jive_output * first, const struct jive_output * second);
	
	/** \brief Class method, reduce operands */
	struct jive_output * (*reduce)(struct jive_output * first, struct jive_output * second);
	
	const struct jive_regcls * (*get_aux_regcls)(const jive_node * self);
	
#if 0
	/** \brief Invalidate any computed state depending on inputs (i.e. value range) */
	void (*invalidate_inputs)(jive_node * self);
	
	/** \brief Recompute any auxiliary state of outputs */
	void (*revalidate_outputs)(jive_node * self);
#endif
};

static inline bool
jive_node_isinstance(const jive_node * self, const jive_node_class * class_)
{
	const jive_node_class * c = self->class_;
	while(c) {
		if (c == class_) return true;
		c = c->parent;
	}
	return false;
}

/* returned string allocated with malloc */
static inline char *
jive_node_get_label(const jive_node * self)
{
	return self->class_->get_label(self);
}

static inline const jive_node_attrs *
jive_node_get_attrs(const jive_node * self)
{
	return self->class_->get_attrs(self);
}

static inline bool
jive_node_equiv(const jive_node * self, const jive_node_attrs * first, const jive_node_attrs * second)
{
	return self->class_->equiv(first, second);
}

static inline const struct jive_regcls *
jive_node_get_aux_regcls(const jive_node * self)
{
	return self->class_->get_aux_regcls(self);
}

static inline jive_node *
jive_node_copy(const jive_node * self,
	struct jive_region * region,
	struct jive_output * operands[])
{
	return self->class_->create(jive_node_get_attrs(self), region, self->noperands, operands);
}

static inline void
jive_node_reserve(jive_node * self)
{
	self->reserved ++;
}

static inline void
jive_node_unreserve(jive_node * self)
{
	self->reserved ++;
}

struct jive_input *
jive_node_add_input(jive_node * self, const struct jive_type * type, struct jive_output * initial_operand);

struct jive_output *
jive_node_add_output(jive_node * self, const struct jive_type * type);

struct jive_input *
jive_node_gate_input(jive_node * self, struct jive_gate * gate, struct jive_output * initial_operand);

struct jive_output *
jive_node_gate_output(jive_node * self, struct jive_gate * gate);

void
jive_node_destroy(jive_node * self);

#endif
