/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_REGALLOC_NOTIFIERS_H
#define JIVE_REGALLOC_NOTIFIERS_H

#include <jive/common.h>

#include <jive/vsdg/notifiers.h>

struct jive_shaped_region;
struct jive_shaped_ssavar;

/* region/ssavar notifiers */

typedef void (*jive_shaped_region_ssavar_notifier_function)(void * closure,
	struct jive_shaped_region * shaped_region, struct jive_shaped_ssavar * shaped_ssavar);
typedef struct jive_shaped_region_ssavar_notifier jive_shaped_region_ssavar_notifier;
typedef struct jive_shaped_region_ssavar_notifier_slot jive_shaped_region_ssavar_notifier_slot;

struct jive_shaped_region_ssavar_notifier_slot {
	struct {
		jive_shaped_region_ssavar_notifier * first;
		jive_shaped_region_ssavar_notifier * last;
	} notifiers;
};

JIVE_EXPORTED_INLINE void
jive_shaped_region_ssavar_notifier_slot_init(jive_shaped_region_ssavar_notifier_slot * self)
{
	self->notifiers.first = self->notifiers.last = 0;
}

void
jive_shaped_region_ssavar_notifier_slot_fini(jive_shaped_region_ssavar_notifier_slot * self);

jive_notifier *
jive_shaped_region_ssavar_notifier_slot_connect(jive_shaped_region_ssavar_notifier_slot * self,
	jive_shaped_region_ssavar_notifier_function function, void * closure);

void
jive_shaped_region_ssavar_notifier_slot_call(const jive_shaped_region_ssavar_notifier_slot * self,
	struct jive_shaped_region * shaped_region, struct jive_shaped_ssavar * shaped_ssavar);

#endif
