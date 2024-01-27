#pragma once
#include <vector>
#include <unordered_map>
#include <X11/Xlib.h>
#include "../Common/IntAutoGamePadOperator.hpp"

class AutoGamePadOperatorForLinux : public IntAutoGamePadOperator {
private:

	Display* const display;
public:
	AutoGamePadOperatorForLinux(Display* const display);
	bool initialize(void) override;
	void update(const CommonAutoOperator::ElapsedTime& elapsedTime) override;
	void onCommandGamePadButton(const CommonAutoOperator::ElapsedTime& elapsedTime, const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) override;
	void onCommandGamePadStick(const CommonAutoOperator::ElapsedTime& elapsedTime, const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) override;
};
