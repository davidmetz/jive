/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_ARCH_SEQUENCE_H
#define JIVE_ARCH_SEQUENCE_H

#include <jive/arch/instruction.h>
#include <jive/vsdg/label.h>

struct jive_graph;
struct jive_node;
struct jive_notifier;
struct jive_region;

typedef struct jive_seq_data jive_seq_data;
typedef struct jive_seq_dataitem jive_seq_dataitem;
typedef struct jive_seq_graph jive_seq_graph;
typedef struct jive_seq_imm jive_seq_imm;
typedef struct jive_seq_instruction jive_seq_instruction;
typedef struct jive_seq_label jive_seq_label;
typedef struct jive_seq_node jive_seq_node;
typedef struct jive_seq_point jive_seq_point;
typedef struct jive_seq_point_class jive_seq_point_class;
typedef struct jive_seq_region jive_seq_region;

struct jive_seq_point_class {
	const jive_seq_point_class * parent;
	void (*fini)(jive_seq_point * self);
};

struct jive_seq_point {
	const jive_seq_point_class * class_;
	jive_seq_region * seq_region;
	
	struct {
		jive_seq_point * prev;
		jive_seq_point * next;
	} seqpoint_list;
	
	/* The list of named symbols pointing to this sequence point. */
	std::vector<const jive_linker_symbol*> named_symbols;
	
	/* Flag indicating whether a local reference to this point exists. */
	bool local_symbol;
	
	/* allow to lookup point in the sequence by the node it was generated
	from; not all points are generated from a node, but those that are,
	are linked here */
	const jive_node * node;
	
	size_t size;
	jive_address address;

private:
	jive::detail::intrusive_hash_anchor<jive_seq_point> hash_chain;

public:
	typedef jive::detail::intrusive_hash_accessor<
		const jive_node *,
		jive_seq_point,
		&jive_seq_point::node,
		&jive_seq_point::hash_chain
	> hash_chain_accessor;
};

typedef jive::detail::intrusive_hash<
	const jive_node *,
	jive_seq_point,
	jive_seq_point::hash_chain_accessor
> jive_seq_node_hash;

extern const jive_seq_point_class JIVE_SEQ_POINT;


struct jive_seq_node {
	jive_seq_point base;
};

extern const jive_seq_point_class JIVE_SEQ_NODE;

struct jive_seq_region {
	const jive_region * region;
	jive_seq_graph * seq_graph;
	struct {
		jive_seq_region * prev;
		jive_seq_region * next;
	} seqregion_list;
	
	jive_seq_point * first_point;
	jive_seq_point * last_point;
	
	bool inlined;

private:
	jive::detail::intrusive_hash_anchor<jive_seq_region> hash_chain;

public:
	typedef jive::detail::intrusive_hash_accessor<
		const jive_region *,
		jive_seq_region,
		&jive_seq_region::region,
		&jive_seq_region::hash_chain
	> hash_chain_accessor;
};

typedef jive::detail::owner_intrusive_hash<
	const jive_region *,
	jive_seq_region,
	jive_seq_region::hash_chain_accessor
> jive_seq_region_hash;

struct jive_seq_graph {
	struct jive_graph * graph;
	struct {
		jive_seq_point * first;
		jive_seq_point * last;
	} points;

	jive_seq_node_hash node_map;
	jive_seq_region_hash region_map;

	struct jive_notifier * label_notifier;
	
	bool addrs_changed;
};

JIVE_EXPORTED_INLINE jive_stdsectionid
jive_seq_region_map_to_section(const jive_seq_region * seq_region)
{
	return jive_region_map_to_section(seq_region->region);
}

JIVE_EXPORTED_INLINE jive_stdsectionid
jive_seq_point_map_to_section(const jive_seq_point * seq_point)
{
	return jive_seq_region_map_to_section(seq_point->seq_region);
}

void
jive_seq_point_init(jive_seq_point * self, jive_seq_region * seq_region, jive_node * node);

JIVE_EXPORTED_INLINE void
jive_seq_point_destroy(jive_seq_point * self)
{
	self->class_->fini(self);
	delete self;
}

JIVE_EXPORTED_INLINE bool
jive_seq_point_isinstance(const jive_seq_point * self, const jive_seq_point_class * class_)
{
	const jive_seq_point_class * c = self->class_;
	while (c) {
		if (c == class_)
			return true;
		c = c->parent;
	}
	return false;
}

/**
	\brief Sequentialize graph
*/
jive_seq_graph *
jive_graph_sequentialize(struct jive_graph * graph);

void
jive_seq_graph_destroy(jive_seq_graph * seq);

inline jive_seq_point *
jive_seq_graph_map_node(jive_seq_graph * seq, const jive_node * node)
{
	auto i = seq->node_map.find(node);
	return i != seq->node_map.end() ? i.ptr() : nullptr;
}

inline jive_seq_region *
jive_seq_graph_map_region(jive_seq_graph * seq, const jive_region * region)
{
	auto i = seq->region_map.find(region);
	return i != seq->region_map.end() ? i.ptr() : nullptr;
}

inline const jive_seq_point *
jive_seq_graph_map_node(const jive_seq_graph * seq, const jive_node * node)
{
	auto i = seq->node_map.find(node);
	return i != seq->node_map.end() ? i.ptr() : nullptr;
}

inline const jive_seq_region *
jive_seq_graph_map_region(const jive_seq_graph * seq, const jive_region * region)
{
	auto i = seq->region_map.find(region);
	return i != seq->region_map.end() ? i.ptr() : nullptr;
}

/* inheritable methods */
void
jive_seq_point_fini_(jive_seq_point * self);

struct jive_seq_instruction {
	jive_seq_point base;
	const jive_instruction_class * icls;
	const jive_register_name ** inputs;
	const jive_register_name ** outputs;
	jive_seq_imm * imm;
	
	/* FIXME: change type to represent coding options */
	jive_instruction_encoding_flags flags;
};

typedef enum jive_seq_label_type {
	jive_seq_label_type_none = 0,
	jive_seq_label_type_internal = 1,
	jive_seq_label_type_external = 2
} jive_seq_label_type;

struct jive_seq_label {
	jive_seq_label_type type;
	union {
		jive_seq_point * internal;
		const jive_linker_symbol * external;
	};
};

/* immediates as represented in the sequentialized graph */
struct jive_seq_imm {
	jive_immediate_int value;
	jive_seq_label add_label;
	jive_seq_label sub_label;
	const void * modifier;
};

extern const jive_seq_point_class JIVE_SEQ_INSTRUCTION;

jive_seq_instruction *
jive_seq_instruction_create(
	jive_seq_region * seq_region,
	const jive_instruction_class * icls,
	const jive_register_name * const * inputs,
	const jive_register_name * const * outputs,
	const jive_seq_imm immediates[],
	jive_node * node);

jive_seq_instruction *
jive_seq_instruction_create_before(
	jive_seq_point * before,
	const jive_instruction_class * icls,
	const jive_register_name * const * inputs,
	const jive_register_name * const * outputs,
	const jive_seq_imm immediates[]);

jive_seq_instruction *
jive_seq_instruction_create_after(
	jive_seq_point * after,
	const jive_instruction_class * icls,
	const jive_register_name * const * inputs,
	const jive_register_name * const * outputs,
	const jive_seq_imm immediates[]);

JIVE_EXPORTED_INLINE jive_seq_instruction *
jive_seq_instruction_cast(jive_seq_point * self)
{
	if (jive_seq_point_isinstance(self, &JIVE_SEQ_INSTRUCTION))
		return (jive_seq_instruction *) self;
	else
		return 0;
}

extern const jive_seq_point_class JIVE_SEQ_DATA;

enum jive_seq_dataitem_fmt {
	jive_seq_dataitem_fmt_none = 0,
	jive_seq_dataitem_fmt_8 = 1,
	jive_seq_dataitem_fmt_le16 = 2,
	jive_seq_dataitem_fmt_be16 = 3,
	jive_seq_dataitem_fmt_le32 = 4,
	jive_seq_dataitem_fmt_be32 = 5,
	jive_seq_dataitem_fmt_le64 = 6,
	jive_seq_dataitem_fmt_be64 = 7
};

struct jive_seq_dataitem {
	uint64_t value;
	jive_seq_dataitem_fmt format;
};

struct jive_seq_data {
	jive_seq_point base;
	size_t nitems;
	jive_seq_dataitem * items;
};

jive_seq_data *
jive_seq_data_create(
	jive_seq_region * seq_region,
	size_t nitems,
	jive_node * node);

JIVE_EXPORTED_INLINE jive_seq_data *
jive_seq_data_cast(jive_seq_point * self)
{
	if (jive_seq_point_isinstance(self, &JIVE_SEQ_DATA))
		return (jive_seq_data *) self;
	else
		return 0;
}
#endif
