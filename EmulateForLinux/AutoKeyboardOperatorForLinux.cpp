#include <iostream>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>
#include "AutoKeyboardOperatorForLinux.h"
#include "../Common/Hash.hpp"

const std::unordered_map<std::string, int> AutoKeyboardOperatorForLinux::modifyKeyHashMap = {
	{"ctrl", XK_Control_L},
	{"shift", XK_Shift_L},
	{"alt", XK_Alt_L},
	{"space", XK_space},
	{"backspace", XK_BackSpace},
	{"delete", XK_Delete},
	{"enter", XK_Return},
	{"esc", XK_Escape},
	{"f1", XK_F1},
	{"f2", XK_F2},
	{"f3", XK_F3},
	{"f4", XK_F4},
	{"f5", XK_F5},
	{"f6", XK_F6},
	{"f7", XK_F7},
	{"f8", XK_F8},
	{"f9", XK_F9},
	{"f10", XK_F10},
	{"f11", XK_F11},
	{"f12", XK_F12},
	{"f13", XK_F13},
	{"f14", XK_F14},
	{"f15", XK_F15},
	{"f16", XK_F16},
	{"f17", XK_F17},
	{"f18", XK_F18},
	{"f19", XK_F19},
	{"f20", XK_F20},
	{"f21", XK_F21},
	{"f22", XK_F22},
	{"f23", XK_F23},
	{"f24", XK_F24},
	{"page_down", XK_Page_Down},
	{"page_up", XK_Page_Up},
	{"home", XK_Home},
	{"end", XK_End},
	{"tab", XK_Tab},
	{"up", XK_Up},
	{"down", XK_Down},
	{"left", XK_Left},
	{"right", XK_Right},
	{"alt_gr", XK_ISO_Level3_Shift},
	{"caps_lock", XK_Caps_Lock},
	{"num_lock", XK_Num_Lock},
	{"scroll_lock", XK_Scroll_Lock},
	{"print_screen", XK_Print},
	{"pause", XK_Pause},
	{"insert", XK_Insert},
	{"menu", XK_Menu},
};


AutoKeyboardOperatorForLinux::AutoKeyboardOperatorForLinux(Display* const display) : display(display){

}

int AutoKeyboardOperatorForLinux::getModifyKey(const CommandSeparator::KeyBoard::Key& modifyKey) {
	const auto& itr = AutoKeyboardOperatorForLinux::modifyKeyHashMap.find(modifyKey);
	if(itr == AutoKeyboardOperatorForLinux::modifyKeyHashMap.end()) {
		return 0;
	}
	return itr->second;
}

bool AutoKeyboardOperatorForLinux::initialize(void) {
	if(this->display == NULL) {
		std::cout << "���ϐ� DISPLAY ���ݒ肳��Ă��܂���." << std::endl;
		return false;
	}

	return true;
}

void AutoKeyboardOperatorForLinux::update(const CommonAutoOperator::ElapsedTime& elapsedTime) {
	// �w�莞�Ԃ��o�߂����L�[�𗣂�
	std::vector<unsigned short> releaseList;
	for(const auto& pair : this->keyDownTimeHashMap) {
		const auto& keycode = pair.first;
		const auto& time = pair.second;

		// �w�莞�Ԃ��o�߂����烊���[�X
		if(time < elapsedTime.count()) {
			// �L�[�����[�X
			XTestFakeKeyEvent(this->display, keycode, false, CurrentTime);
			XFlush(this->display);
		}
	}

	// �������L�[�̓}�b�v�ォ��폜
	for(const auto& releaseButton : releaseList) {
		const auto& buttonItr = this->keyDownTimeHashMap.find(releaseButton);
		this->keyDownTimeHashMap.erase(buttonItr);
	}
}

void AutoKeyboardOperatorForLinux::onCommandKeyDown(const CommonAutoOperator::ElapsedTime& elapsedTime, const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) {
	const auto& splitArgs = CommandSeparator::KeyBoard::getKeyArgs(args);
	const auto& key = std::get<0>(splitArgs);
	const auto& modifyKeys = std::get<1>(splitArgs);
	const auto& time = std::get<2>(splitArgs);

	// �ʏ�L�[
	const auto commandHash = Hash::getHash(command.c_str()); 
	if(commandHash == static_cast<Hash::HashType>(CommandSeparator::ControlCommand::KEY_DOWN)) {
		const auto keycode = XKeysymToKeycode(this->display, key[0]);
		this->keyDownTimeHashMap[keycode] = time;
		XTestFakeKeyEvent(this->display, keycode, true, CurrentTime); 
		XFlush(this->display);
	}
	// �C���L�[
	if(commandHash == static_cast<Hash::HashType>(CommandSeparator::ControlCommand::SPECIAL_KEY_DOWN)) {
		const auto keycode = getModifyKey(key);
		this->keyDownTimeHashMap[keycode] = time;  
		XTestFakeKeyEvent(this->display, keycode, true, CurrentTime);
		XFlush(this->display);
	}

	// �ǉ��̏C���L�[
	for(const auto& modifyKey : modifyKeys) {
		const auto keycode = getModifyKey(modifyKey);
		this->keyDownTimeHashMap[keycode] = time;
		XTestFakeKeyEvent(this->display, keycode, true, CurrentTime);
		XFlush(this->display);
	}

}

void AutoKeyboardOperatorForLinux::onCommandString(const CommonAutoOperator::ElapsedTime& elapsedTime, const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) {
}