#include "AutoMouseOperatorForWindows.h"
#include <Windows.h>
using namespace CommandSeparator;

bool AutoMouseOperatorForWindows::initialize(void) {
	return true;
}

void AutoMouseOperatorForWindows::update(const CommonAutoOperator::ElapsedTime& elapsedTime) {
	// �w�莞�Ԃ��o�߂����L�[�𗣂�
	std::vector<Mouse::ClickButton> releaseList;
	for(const auto& pair : this->mouseClickTimeHashMap) {
		const auto& button = pair.first;
		const auto& time = pair.second;

		// �w�莞�Ԃ��o�߂����烊���[�X
		if(time < elapsedTime.count()) {
			INPUT input = {};
			input.type = INPUT_KEYBOARD;

			if(button == "left") {
				input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
			}
			if(button == "right") {
				input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
			}

			releaseList.emplace_back(button);
		}
	}

	// �������L�[�̓}�b�v�ォ��폜
	for(const auto& releaseButton : releaseList) {
		const auto& buttonItr = this->mouseClickTimeHashMap.find(releaseButton);
		this->mouseClickTimeHashMap.erase(buttonItr);
	}
}

void AutoMouseOperatorForWindows::onCommandMouseClick(const CommonAutoOperator::ElapsedTime& elapsedTime, const Commands::Command& command, const Commands::Args& args) {
	const auto& splitArgs = Mouse::getClickButtonArgs(args);
	const auto& button = std::get<0>(splitArgs);
	const auto& x = std::get<1>(splitArgs);
	const auto& y = std::get<2>(splitArgs);
	const auto& time = std::get<3>(splitArgs);

	this->mouseClickTimeHashMap[button] = time;

	// �L�[����
	INPUT input = {};
	input.type = INPUT_MOUSE;

	if(button == "left") {
		input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	}
	if(button == "right") {
		input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
	}

	SendInput(1, &input, sizeof(INPUT));
}

void AutoMouseOperatorForWindows::onCommandMouseCursor(const CommonAutoOperator::ElapsedTime& elapsedTime, const Commands::Command& command, const Commands::Args& args) {
	const auto& splitArgs = Mouse::getCursorArgs(args);
	const auto& action = std::get<0>(splitArgs);
	const auto& x = std::get<1>(splitArgs);
	const auto& y = std::get<2>(splitArgs);
	const auto& time = std::get<3>(splitArgs);

	if(action == "Move") {
		SetCursorPos(x, y);
	}
}

void AutoMouseOperatorForWindows::onCommandMouseScroll(const CommonAutoOperator::ElapsedTime& elapsedTime, const Commands::Command& command, const Commands::Args& args) {
}