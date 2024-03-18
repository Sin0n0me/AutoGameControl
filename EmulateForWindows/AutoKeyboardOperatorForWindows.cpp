#include "AutoKeyboardOperatorForWindows.h"
#include <windows.h>

using namespace CommandSeparator;

bool AutoKeyboardOperatorForWindows::initialize(void) {
	return true;
}

void AutoKeyboardOperatorForWindows::update(const CommonAutoOperator::ElapsedTime& elapsedTime) {
	// 指定時間が経過したキーを離す
	std::vector<unsigned short> releaseList;
	for(const auto& pair : this->keyDownTimeHashMap) {
		const auto& key = pair.first;
		const auto& time = pair.second;

		// 指定時間を経過したらリリース
		if(time < elapsedTime.count()) {
			INPUT input = {};
			input.type = INPUT_KEYBOARD;
			input.ki.wVk = key;
			input.ki.dwFlags = KEYEVENTF_KEYUP;
			releaseList.emplace_back(key);
		}
	}

	// 離したキーはマップ上から削除
	for(const auto& releaseButton : releaseList) {
		const auto& buttonItr = this->keyDownTimeHashMap.find(releaseButton);
		this->keyDownTimeHashMap.erase(buttonItr);
	}
}

void AutoKeyboardOperatorForWindows::onCommandKeyDown(const CommonAutoOperator::ElapsedTime& elapsedTime, const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) {
	const auto& splitArgs = CommandSeparator::KeyBoard::getKeyArgs(args);
	const auto& key = std::get<0>(splitArgs);
	const auto& modifyKeys = std::get<1>(splitArgs);
	const auto& time = std::get<2>(splitArgs);

	const auto vk = VkKeyScan(key[0]);
	this->keyDownTimeHashMap[vk] = time;

	// キー押下
	INPUT input = {};
	input.type = INPUT_KEYBOARD;
	input.ki.wVk = vk;
	SendInput(1, &input, sizeof(INPUT));
}

void AutoKeyboardOperatorForWindows::onCommandString(const CommonAutoOperator::ElapsedTime& elapsedTime, const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) {
}