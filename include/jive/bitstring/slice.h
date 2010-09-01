#ifndef JIVE_BITSTRING_SLICE_H
#define JIVE_BITSTRING_SLICE_H

#include <jive/vsdg/node.h>
#include <jive/bitstring/type.h>

extern const jive_node_class JIVE_BITSLICE_NODE;

typedef struct jive_bitslice_node jive_bitslice_node;
typedef struct jive_bitslice_node_attrs jive_bitslice_node_attrs;

struct jive_bitslice_node_attrs {
	jive_node_attrs base;
	size_t low, high;
};

struct jive_bitslice_node {
	jive_node base;
	jive_bitslice_node_attrs attrs;
};

/**
	\brief Create bitslice
	\param region Region to put node into
	\param origin Input value
	\param low Low bit
	\param high High bit
	\returns Bitstring value representing slice
	
	Create new bitslice node. Takes the bits <TT>[low:high)</TT> out
	of the input bits.
*/
jive_bitslice_node *
jive_bitslice_node_create(struct jive_region * region, jive_output * origin, size_t low, size_t high);

/**
	\brief Create bitslice
	\param operand Input value
	\param low Low bit
	\param high High bit
	\returns Bitstring value representing slice
	
	Convenience function that either creates a new slice or
	returns the output handle of an existing slice.
*/
jive_bitstring *
jive_bitslice(jive_bitstring * operand, size_t low, size_t high);

static inline jive_bitslice_node *
jive_bitslice_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITSLICE_NODE) return (jive_bitslice_node *) node;
	else return 0;
}

#endif
