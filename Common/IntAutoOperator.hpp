#ifndef AUTO_OPERATOR_INTERFACE_AUTO_OPERATOR_H
# define AUTO_OPERATOR_INTERFACE_AUTO_OPERATOR_H

#include <chrono>
#include "CommandSeparater.hpp"

class IntAutoOperator {
public:
	virtual ~IntAutoOperator(void) noexcept = default;
	virtual bool initialize(void) = 0;
	virtual void update(const CommonAutoOperator::ElapsedTime& elapsedTime) = 0;
};

#endif