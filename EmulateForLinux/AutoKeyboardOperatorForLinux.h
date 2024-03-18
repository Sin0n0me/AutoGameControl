#pragma once
#include <unordered_map>
#include <X11/Xlib.h>
#include "../Common/IntAutoKeyboardOperator.hpp"

class AutoKeyboardOperatorForLinux : public IntAutoKeyboardOperator {
private:
	static const std::unordered_map<std::string, int> modifyKeyHashMap;

	Display* const display;
	std::unordered_map<unsigned short, CommandSeparator::Common::PushTime> keyDownTimeHashMap;
	int getModifyKey(const CommandSeparator::KeyBoard::Key& modifyKey); 

public:
	AutoKeyboardOperatorForLinux(Display* const display);
	bool initialize(void) override;
	void update(const CommonAutoOperator::ElapsedTime& elapsedTime) override;
	void onCommandKeyDown(const CommonAutoOperator::ElapsedTime& elapsedTime, const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) override;
	void onCommandString(const CommonAutoOperator::ElapsedTime& elapsedTime, const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) override;
};
