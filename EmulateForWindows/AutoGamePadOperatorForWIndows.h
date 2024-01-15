#ifndef AUTO_OPERATOR_AUTO_GAMEPAD_OPERATOR_FOR_WINDOWS_H
# define AUTO_OPERATOR_AUTO_GAMEPAD_OPERATOR_FOR_WINDOWS_H

#include <vector>
#include <unordered_map>
#include "../Common/IntAutoGamePadOperator.hpp"

class AutoGamePadOperatorForWIndows : public IntAutoGamePadOperator {
private:

	const int vJoyDeviceID;
	int maxButtonNum;
	std::vector<int> buttonList;
	std::unordered_map<CommandSeparator::GamePad::Button, CommandSeparator::Common::PushTime> buttonDownTimeHashMap;

public:
	AutoGamePadOperatorForWIndows(void);
	~AutoGamePadOperatorForWIndows(void) noexcept;
	bool initialize(void) override;
	void update(const CommonAutoOperator::ElapsedTime& elapsedTime) override;
	void onCommandGamePadButton(const CommonAutoOperator::ElapsedTime& elapsedTime, const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) override;
	void onCommandGamePadStick(const CommonAutoOperator::ElapsedTime& elapsedTime, const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) override;
};

#endif 