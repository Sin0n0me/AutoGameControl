#ifndef AUTO_OPERATOR_INTERFACE_AUTO_MOUSE_OPERATOR_H
# define AUTO_OPERATOR_INTERFACE_AUTO_MOUSE_OPERATOR_H
#include "IntAutoOperator.hpp"

class IntAutoMouseOperator : public IntAutoOperator {
public:
	virtual ~IntAutoMouseOperator(void) noexcept = default;
	virtual void onCommandMouseClick(const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) = 0;
	virtual void onCommandMouseCursor(const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) = 0;
	virtual void onCommandMouseScroll(const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) = 0;
};
#endif