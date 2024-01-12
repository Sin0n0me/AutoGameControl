#pragma once
#include <unordered_map>
#include "../Common/IntAutoMouseOperator.hpp"

class AutoMouseOperatorForWindows : public IntAutoMouseOperator {
private:

public:

	bool initialize(void) override;

	void update(const CommonAutoOperator::ElapsedTime& elapsedTime) override;

	void onCommandMouseClick(const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) override;

	void onCommandMouseCursor(const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) override;

	void onCommandMouseScroll(const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) override;
};
