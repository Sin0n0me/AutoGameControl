#ifndef AUTO_OPERATOR_AUTO_MOUSE_OPERATOR_FOR_WINDOWS_H
# define AUTO_OPERATOR_AUTO_MOUSE_OPERATOR_FOR_WINDOWS_H
#include <unordered_map>
#include "../Common/IntAutoMouseOperator.hpp"

class AutoMouseOperatorForWindows : public IntAutoMouseOperator {
private:
	std::unordered_map<CommandSeparator::Mouse::ClickButton, CommandSeparator::Common::PushTime> mouseClickTimeHashMap;

public:

	bool initialize(void) override;

	void update(const CommonAutoOperator::ElapsedTime& elapsedTime) override;
	void onCommandMouseClick(const CommonAutoOperator::ElapsedTime& elapsedTime, const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) override;
	void onCommandMouseCursor(const CommonAutoOperator::ElapsedTime& elapsedTime, const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) override;
	void onCommandMouseScroll(const CommonAutoOperator::ElapsedTime& elapsedTime, const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) override;
};
#endif