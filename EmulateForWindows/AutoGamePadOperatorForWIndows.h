#pragma once
#include <unordered_map>
#include "../Common/IntAutoGamePadOperator.hpp"

class AutoGamePadOperatorForWIndows : public IntAutoGamePadOperator {
private:
	const int vJoyDeviceID;
	std::unordered_map<CommandSeparator::GamePad::Button, CommandSeparator::Common::PushTime> buttonDownTimeHashMap;

public:
	AutoGamePadOperatorForWIndows(void);
	~AutoGamePadOperatorForWIndows(void) noexcept;
	bool initialize(void) override;
	void update(const CommonAutoOperator::ElapsedTime& elapsedTime) override;
	void onCommandGamePadButton(const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) override;
	void onCommandGamePadStick(const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) override;
};
