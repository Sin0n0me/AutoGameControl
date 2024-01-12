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
	vJoyDeviceID(1)	// vJoyデバイスID(1 index)
{
}

AutoGamePadOperatorForWIndows::~AutoGamePadOperatorForWIndows(void) noexcept {
	// 使用が終わったらvJoyデバイスの使用権を解放
	RelinquishVJD(this->vJoyDeviceID);
}

bool AutoGamePadOperatorForWIndows::initialize(void) {
	// vJoyドライバが有効かどうか確認
	if(!vJoyEnabled()) {
		std::cerr << "vJoy driver not enabled: Failed Getting vJoy attributes." << std::endl;
		return false;
	}

	// vJoyデバイスが存在し、利用可能かどうか確認
	const VjdStat status = GetVJDStatus(vJoyDeviceID);
	if(status == VJD_STAT_OWN || status == VJD_STAT_FREE) {
		// vJoyデバイスを取得（アクセス権を確保）
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
	// 指定時間が経過したキーを離す
	std::vector<GamePad::Button> releaseList;
	for(const auto& pair : this->buttonDownTimeHashMap) {
		const auto& button = pair.first;
		const auto& time = pair.second;

		// 指定時間を経過したらリリース
		if(time < elapsedTime.count()) {
			SetBtn(FALSE, this->vJoyDeviceID, button);
			releaseList.emplace_back(button);
		}
	}

	// 離したキーはマップ上から削除
	for(const auto& releaseButton : releaseList) {
		const auto& buttonItr = this->buttonDownTimeHashMap.at(releaseButton);
		this->buttonDownTimeHashMap.erase(buttonItr);
	}
}

void AutoGamePadOperatorForWIndows::onCommandGamePadButton(const Commands::Command& command, const Commands::Args& args) {
	// 仮想ボタンの操作を行う
	const auto& splitArgs = GamePad::getButtonArgs(args);
	const auto& button = std::get<0>(splitArgs);
	const auto& time = std::get<1>(splitArgs);
	this->buttonDownTimeHashMap[button] = time;
	SetBtn(TRUE, this->vJoyDeviceID, button);

	std::cout << "button:" << button << " " << time << "ms" << std::endl;
}

void AutoGamePadOperatorForWIndows::onCommandGamePadStick(const Commands::Command& command, const Commands::Args& args) {
	// 仮想ジョイスティックの操作を行う
	const auto& splitArgs = GamePad::getJoyStickArgs(args);
	const auto& joystick = std::get<0>(splitArgs);
	const auto& argsX = std::get<1>(splitArgs);
	const auto& argsY = std::get<2>(splitArgs);

	// -1.0f~1.0fの範囲内なのでshort型の範囲内(0~32767)に修正
	constexpr short centerPos = 0x7fff / 2;
	const short rangeX = static_cast<short>(centerPos * argsX * 0.5f);
	const short rangeY = static_cast<short>(centerPos * argsY * 0.5f);
	const auto x = centerPos + rangeX;
	const auto y = centerPos + rangeY;

	// IDに応じたそれぞれの軸の設定
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