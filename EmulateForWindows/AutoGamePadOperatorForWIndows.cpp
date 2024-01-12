#include <Windows.h>
#include <iostream>
#include "../Common/CommandSeparater.hpp"
#include "vJoy/SDK/inc/public.h"
#include "vJoy/SDK/inc/vjoyinterface.h"
#include "AutoGamePadOperatorForWIndows.h"

#ifdef _WIN64
#pragma comment (lib, "vJoy/SDK/lib/amd64/vJoyInterface.lib")
#else
#pragma comment (lib, "vJoy/SDK/lib/vJoyInterface.lib")
#endif // _WIN64

using namespace CommandSeparator;

AutoGamePadOperatorForWIndows::AutoGamePadOperatorForWIndows(void) :
	vJoyDeviceID(1)	// vJoy�f�o�C�XID(1 index)
{
}

AutoGamePadOperatorForWIndows::~AutoGamePadOperatorForWIndows(void) noexcept {
	// �g�p���I�������vJoy�f�o�C�X�̎g�p�������
	RelinquishVJD(this->vJoyDeviceID);
}

bool AutoGamePadOperatorForWIndows::initialize(void) {
	// vJoy�h���C�o���L�����ǂ����m�F
	if(!vJoyEnabled()) {
		std::cerr << "vJoy driver not enabled: Failed Getting vJoy attributes." << std::endl;
		return false;
	}

	// vJoy�f�o�C�X�����݂��A���p�\���ǂ����m�F
	const VjdStat status = GetVJDStatus(vJoyDeviceID);
	if(status == VJD_STAT_OWN || status == VJD_STAT_FREE) {
		// vJoy�f�o�C�X���擾�i�A�N�Z�X�����m�ہj
		if(!AcquireVJD(vJoyDeviceID)) {
			std::cerr << "Failed to acquire vJoy device number " << vJoyDeviceID << std::endl;
			return false;
		}
	} else {
		std::cerr << "vJoy device " << vJoyDeviceID << " is not available." << std::endl;
		return false;
	}

	return true;
}

void AutoGamePadOperatorForWIndows::update(const CommonAutoOperator::ElapsedTime& elapsedTime) {
	// �w�莞�Ԃ��o�߂����L�[�𗣂�
	std::vector<GamePad::Button> releaseList;
	for(const auto& pair : this->buttonDownTimeHashMap) {
		const auto& button = pair.first;
		const auto& time = pair.second;

		// �w�莞�Ԃ��o�߂����烊���[�X
		if(time < elapsedTime.count()) {
			SetBtn(FALSE, this->vJoyDeviceID, button);
			releaseList.emplace_back(button);
		}
	}

	// �������L�[�̓}�b�v�ォ��폜
	for(const auto& releaseButton : releaseList) {
		const auto& buttonItr = this->buttonDownTimeHashMap.at(releaseButton);
		this->buttonDownTimeHashMap.erase(buttonItr);
	}
}

void AutoGamePadOperatorForWIndows::onCommandGamePadButton(const Commands::Command& command, const Commands::Args& args) {
	// ���z�{�^���̑�����s��
	const auto& splitArgs = GamePad::getButtonArgs(args);
	const auto& button = std::get<0>(splitArgs);
	const auto& time = std::get<1>(splitArgs);
	this->buttonDownTimeHashMap[button] = time;
	SetBtn(TRUE, this->vJoyDeviceID, button);

	std::cout << "button:" << button << " " << time << "ms" << std::endl;
}

void AutoGamePadOperatorForWIndows::onCommandGamePadStick(const Commands::Command& command, const Commands::Args& args) {
	// ���z�W���C�X�e�B�b�N�̑�����s��
	const auto& splitArgs = GamePad::getJoyStickArgs(args);
	const auto& joystick = std::get<0>(splitArgs);
	const auto& argsX = std::get<1>(splitArgs);
	const auto& argsY = std::get<2>(splitArgs);

	// -1.0f~1.0f�͈͓̔��Ȃ̂�short�^�͈͓̔�(0~32767)�ɏC��
	constexpr short centerPos = 0x7fff / 2;
	const short rangeX = static_cast<short>(centerPos * argsX * 0.5f);
	const short rangeY = static_cast<short>(centerPos * argsY * 0.5f);
	const auto x = centerPos + rangeX;
	const auto y = centerPos + rangeY;

	// ID�ɉ��������ꂼ��̎��̐ݒ�
	if(joystick == static_cast<int>(GamePad::JoyStickID::LEFT_STICK)) {
		SetAxis(x, this->vJoyDeviceID, HID_USAGE_X);
		SetAxis(y, this->vJoyDeviceID, HID_USAGE_Y);
	}
	if(joystick == static_cast<int>(GamePad::JoyStickID::RIGHT_STICK)) {
		SetAxis(x, this->vJoyDeviceID, HID_USAGE_RX);
		SetAxis(y, this->vJoyDeviceID, HID_USAGE_RY);
	}
	if(joystick == static_cast<int>(GamePad::JoyStickID::TRIGGER)) {
		SetAxis(x, this->vJoyDeviceID, HID_USAGE_Z);
		SetAxis(y, this->vJoyDeviceID, HID_USAGE_RZ);
	}

	std::cout << "joystick:" << joystick << " x:" << argsX << " y:" << argsY << std::endl;
}