#pragma once
#include <unordered_map>
#include "../Common/IntAutoKeyboardOperator.hpp"

class AutoKeyboardOperatorForWindows : public IntAutoKeyboardOperator {
private:

public:

	bool initialize(void) override;
	void update(const CommonAutoOperator::ElapsedTime& elapsedTime) override;
	void onCommandKeyDown(const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) override;
	void onCommandString(const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) override;
};
