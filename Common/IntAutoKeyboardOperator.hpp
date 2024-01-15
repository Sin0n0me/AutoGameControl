#ifndef AUTO_OPERATOR_INTERFACE_AUTO_KEYBOARD_OPERATOR_H
# define AUTO_OPERATOR_INTERFACE_AUTO_KEYBOARD_OPERATOR_H

#include "IntAutoOperator.hpp"

class IntAutoKeyboardOperator : public IntAutoOperator {
public:
	virtual ~IntAutoKeyboardOperator(void) noexcept = default;
	virtual void onCommandKeyDown(const CommonAutoOperator::ElapsedTime& elapsedTime, const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) = 0;
	virtual void onCommandString(const CommonAutoOperator::ElapsedTime& elapsedTime, const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) = 0;
};

#endif