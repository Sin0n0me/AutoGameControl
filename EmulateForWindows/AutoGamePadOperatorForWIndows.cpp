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

enum class DiscPov {
	Center = -1,
	UP = 0,
	Right = 1,
	Down = 2,
	Left = 3,
};

enum class PSButtonDiscPov {
	UP = 11,
	Down = 12,
	Left = 13,
	Right = 14,
};

AutoGamePadOperatorForWIndows::AutoGamePadOperatorForWIndows(void) :
	vJoyDeviceID(1)	// vJoy�f�o�C�XID(1 index)
{
	this->maxButtonNum = 0;
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

	// vJoy�f�o�C�X�����݂����p�\���ǂ����m�F
	const VjdStat status = GetVJDStatus(this->vJoyDeviceID);
	if(status == VJD_STAT_OWN || status == VJD_STAT_FREE) {
		// vJoy�f�o�C�X���擾(�A�N�Z�X�����m��)
		if(!AcquireVJD(this->vJoyDeviceID)) {
			std::cerr << "Failed to acquire vJoy device number " << this->vJoyDeviceID << std::endl;
			return false;
		}
	} else {
		std::cerr << "vJoy device " << this->vJoyDeviceID << " is not available." << std::endl;
		return false;
	}

	this->maxButtonNum = GetVJDButtonNumber(this->vJoyDeviceID);

	// vJoy�f�o�C�X�̏��\��
	std::cout << "================================================================" << std::endl;
	std::cout << "> vJoy Version: " << GetvJoyVersion() << std::endl;
	std::wcout << "> vJoy Product: " << static_cast<TCHAR*>(GetvJoyProductString()) << std::endl;
	std::wcout << "> vJoy Manufacturer: " << static_cast<TCHAR*>(GetvJoyManufacturerString()) << std::endl;
	std::wcout << "> vJoy SerialNumber: " << static_cast<TCHAR*>(GetvJoySerialNumberString()) << std::endl;
	std::cout << "> vJoy ButtonNumber: " << this->maxButtonNum << std::endl;
	std::cout << "> vJoy DiscPovNumber: " << GetVJDDiscPovNumber(this->vJoyDeviceID) << std::endl;
	std::cout << "> vJoy ContPovNumber: " << GetVJDContPovNumber(this->vJoyDeviceID) << std::endl;
	std::cout << "================================================================" << std::endl;

	return true;
}

void AutoGamePadOperatorForWIndows::update(const CommonAutoOperator::ElapsedTime& elapsedTime) {
	// �w�莞�Ԃ��o�߂����{�^���𗣂�
	std::vector<GamePad::Button> releaseList;
	for(const auto& pair : this->buttonDownTimeHashMap) {
		const auto& button = pair.first;
		const auto& time = pair.second;

		// �w�莞�Ԃ��o�߂����烊���[�X
		if(time < elapsedTime.count()) {
			releaseList.emplace_back(button);

			if(button < this->maxButtonNum) {
				SetBtn(FALSE, this->vJoyDeviceID, button);
			} else {
				// �\���L�[�͂����ꂩ�̃L�[����������Ă��Ȃ���Β��S�ɖ߂�
				for(auto itr = this->buttonList.begin(); itr != this->buttonList.end();) {
					if(*itr == button) {
						itr = this->buttonList.erase(itr);
					} else {
						++itr;
					}
				}
				if(this->buttonList.empty()) {
					SetDiscPov(static_cast<int>(DiscPov::Center), this->vJoyDeviceID, 1);
				}
			}
		}
	}

	// �������{�^���̓}�b�v�ォ��폜
	for(const auto& releaseButton : releaseList) {
		const auto& buttonItr = this->buttonDownTimeHashMap.at(releaseButton);
		this->buttonDownTimeHashMap.erase(buttonItr);
	}
}

void AutoGamePadOperatorForWIndows::onCommandGamePadButton(const CommonAutoOperator::ElapsedTime& elapsedTime, const Commands::Command& command, const Commands::Args& args) {
	// ���z�{�^���̑�����s��
	const auto& splitArgs = GamePad::getButtonArgs(args);
	const auto& button = std::get<0>(splitArgs);
	const auto& time = std::get<1>(splitArgs);
	this->buttonDownTimeHashMap[button] = elapsedTime.count() + time;

	if(button < this->maxButtonNum) {
		SetBtn(TRUE, this->vJoyDeviceID, button);
	} else {
		// PS�R���g���[���[
		// ���U�^��I��
		// ��
		if(button == static_cast<int>(PSButtonDiscPov::UP)) {
			SetDiscPov(static_cast<int>(DiscPov::UP), this->vJoyDeviceID, 1);
		}
		// ��
		if(button == static_cast<int>(PSButtonDiscPov::Down)) {
			SetDiscPov(static_cast<int>(DiscPov::Down), this->vJoyDeviceID, 1);
		}
		// ��
		if(button == static_cast<int>(PSButtonDiscPov::Left)) {
			SetDiscPov(static_cast<int>(DiscPov::Left), this->vJoyDeviceID, 1);
		}
		// �E
		if(button == static_cast<int>(PSButtonDiscPov::Right)) {
			SetDiscPov(static_cast<int>(DiscPov::Right), this->vJoyDeviceID, 1);
		}

		this->buttonList.emplace_back(button);
	}

	std::cout << "button:" << button << " " << time << "ms" << std::endl;
}

void AutoGamePadOperatorForWIndows::onCommandGamePadStick(const CommonAutoOperator::ElapsedTime& elapsedTime, const Commands::Command& command, const Commands::Args& args) {
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

	std::cout << "joystick: " << joystick << " x: " << argsX << " y: " << argsY << std::endl;
}