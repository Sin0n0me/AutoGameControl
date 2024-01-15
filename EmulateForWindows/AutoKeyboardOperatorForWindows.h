#ifndef AUTO_OPERATOR_AUTO_KEYBOARD_OPERATOR_FOR_WINDOWS_H
# define AUTO_OPERATOR_AUTO_KEYBOARD_OPERATOR_FOR_WINDOWS_H
#include <unordered_map>
#include "../Common/IntAutoKeyboardOperator.hpp"

class AutoKeyboardOperatorForWindows : public IntAutoKeyboardOperator {
private:
	std::unordered_map<unsigned short, CommandSeparator::Common::PushTime> keyDownTimeHashMap;

public:

	bool initialize(void) override;
	void update(const CommonAutoOperator::ElapsedTime& elapsedTime) override;
	void onCommandKeyDown(const CommonAutoOperator::ElapsedTime& elapsedTime, const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) override;
	void onCommandString(const CommonAutoOperator::ElapsedTime& elapsedTime, const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) override;
};

#endif