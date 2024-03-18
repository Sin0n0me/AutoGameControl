#include "AutoKeyboardOperatorForWindows.h"
#include <windows.h>

using namespace CommandSeparator;

bool AutoKeyboardOperatorForWindows::initialize(void) {
	return true;
}

void AutoKeyboardOperatorForWindows::update(const CommonAutoOperator::ElapsedTime& elapsedTime) {
	// �w�莞�Ԃ��o�߂����L�[�𗣂�
	std::vector<unsigned short> releaseList;
	for(const auto& pair : this->keyDownTimeHashMap) {
		const auto& key = pair.first;
		const auto& time = pair.second;

		// �w�莞�Ԃ��o�߂����烊���[�X
		if(time < elapsedTime.count()) {
			INPUT input = {};
			input.type = INPUT_KEYBOARD;
			input.ki.wVk = key;
			input.ki.dwFlags = KEYEVENTF_KEYUP;
			releaseList.emplace_back(key);
		}
	}

	// �������L�[�̓}�b�v�ォ��폜
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

	// �L�[����
	INPUT input = {};
	input.type = INPUT_KEYBOARD;
	input.ki.wVk = vk;
	SendInput(1, &input, sizeof(INPUT));
}

void AutoKeyboardOperatorForWindows::onCommandString(const CommonAutoOperator::ElapsedTime& elapsedTime, const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) {
}