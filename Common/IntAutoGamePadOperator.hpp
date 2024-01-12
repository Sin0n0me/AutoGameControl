#ifndef AUTO_OPERATOR_INTERFACE_AUTO_GAME_PAD_OPERATOR_H
# define AUTO_OPERATOR_INTERFACE_AUTO_GAME_PAD_OPERATOR_H

#include "IntAutoOperator.hpp"

class IntAutoGamePadOperator : public IntAutoOperator {
public:
	virtual ~IntAutoGamePadOperator(void) noexcept = default;
	virtual void onCommandGamePadButton(const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) = 0;
	virtual void onCommandGamePadStick(const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) = 0;
};

#endif