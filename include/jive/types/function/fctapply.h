/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FUNCTION_FCTAPPLY_H
#define JIVE_TYPES_FUNCTION_FCTAPPLY_H

#include <jive/types/function/fcttype.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/simple.h>

namespace jive {
namespace fct {

class apply_op final : public jive::simple_op {
public:
	virtual
	~apply_op() noexcept;

	explicit apply_op(const type & function_type);

	virtual bool
	operator==(const operation & other) const noexcept override;

	virtual size_t
	narguments() const noexcept override;

	virtual const jive::port &
	argument(size_t index) const noexcept override;

	virtual size_t
	nresults() const noexcept override;

	virtual const jive::port &
	result(size_t index) const noexcept override;

	virtual std::string
	debug_string() const override;

	inline const type &
	function_type() const noexcept
	{
		return *static_cast<const jive::fct::type*>(&argument(0).type());
	}

	virtual std::unique_ptr<jive::operation>
	copy() const override;

private:
	std::vector<jive::port> results_;
	std::vector<jive::port> arguments_;
};

}
}

std::vector<jive::output*>
jive_apply_create(jive::output * function, size_t narguments, jive::output * const arguments[]);

#endif
