#pragma once
#include <unordered_map>
#include <X11/Xlib.h>
#include "../Common/IntAutoMouseOperator.hpp"

class AutoMouseOperatorForLinux : public IntAutoMouseOperator {
private:

	Display* const display;
public:
	AutoMouseOperatorForLinux(Display* const display);
	bool initialize(void) override;
	void update(const CommonAutoOperator::ElapsedTime& elapsedTime) override;
	void onCommandMouseClick(const CommonAutoOperator::ElapsedTime& elapsedTime, const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) override;
	void onCommandMouseCursor(const CommonAutoOperator::ElapsedTime& elapsedTime, const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) override;
	void onCommandMouseScroll(const CommonAutoOperator::ElapsedTime& elapsedTime, const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) override;
};
