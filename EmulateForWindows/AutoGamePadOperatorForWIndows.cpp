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
	vJoyDeviceID(1)	// vJoyデバイスID(1 index)
{
	this->maxButtonNum = 0;
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

	// vJoyデバイスが存在し利用可能かどうか確認
	const VjdStat status = GetVJDStatus(this->vJoyDeviceID);
	if(status == VJD_STAT_OWN || status == VJD_STAT_FREE) {
		// vJoyデバイスを取得(アクセス権を確保)
		if(!AcquireVJD(this->vJoyDeviceID)) {
			std::cerr << "Failed to acquire vJoy device number " << this->vJoyDeviceID << std::endl;
			return false;
		}
	} else {
		std::cerr << "vJoy device " << this->vJoyDeviceID << " is not available." << std::endl;
		return false;
	}

	this->maxButtonNum = GetVJDButtonNumber(this->vJoyDeviceID);

	// vJoyデバイスの情報表示
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
	// 指定時間が経過したボタンを離す
	std::vector<GamePad::Button> releaseList;
	for(const auto& pair : this->buttonDownTimeHashMap) {
		const auto& button = pair.first;
		const auto& time = pair.second;

		// 指定時間を経過したらリリース
		if(time < elapsedTime.count()) {
			releaseList.emplace_back(button);

			if(button < this->maxButtonNum) {
				SetBtn(FALSE, this->vJoyDeviceID, button);
			} else {
				// 十字キーはいずれかのキーが押下されていなければ中心に戻す
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

	// 離したボタンはマップ上から削除
	for(const auto& releaseButton : releaseList) {
		const auto& buttonItr = this->buttonDownTimeHashMap.at(releaseButton);
		this->buttonDownTimeHashMap.erase(buttonItr);
	}
}

void AutoGamePadOperatorForWIndows::onCommandGamePadButton(const CommonAutoOperator::ElapsedTime& elapsedTime, const Commands::Command& command, const Commands::Args& args) {
	// 仮想ボタンの操作を行う
	const auto& splitArgs = GamePad::getButtonArgs(args);
	const auto& button = std::get<0>(splitArgs);
	const auto& time = std::get<1>(splitArgs);
	this->buttonDownTimeHashMap[button] = elapsedTime.count() + time;

	if(button < this->maxButtonNum) {
		SetBtn(TRUE, this->vJoyDeviceID, button);
	} else {
		// PSコントローラー
		// 離散型を選択
		// 上
		if(button == static_cast<int>(PSButtonDiscPov::UP)) {
			SetDiscPov(static_cast<int>(DiscPov::UP), this->vJoyDeviceID, 1);
		}
		// 下
		if(button == static_cast<int>(PSButtonDiscPov::Down)) {
			SetDiscPov(static_cast<int>(DiscPov::Down), this->vJoyDeviceID, 1);
		}
		// 左
		if(button == static_cast<int>(PSButtonDiscPov::Left)) {
			SetDiscPov(static_cast<int>(DiscPov::Left), this->vJoyDeviceID, 1);
		}
		// 右
		if(button == static_cast<int>(PSButtonDiscPov::Right)) {
			SetDiscPov(static_cast<int>(DiscPov::Right), this->vJoyDeviceID, 1);
		}

		this->buttonList.emplace_back(button);
	}

	std::cout << "button:" << button << " " << time << "ms" << std::endl;
}

void AutoGamePadOperatorForWIndows::onCommandGamePadStick(const CommonAutoOperator::ElapsedTime& elapsedTime, const Commands::Command& command, const Commands::Args& args) {
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

	std::cout << "joystick: " << joystick << " x: " << argsX << " y: " << argsY << std::endl;
}