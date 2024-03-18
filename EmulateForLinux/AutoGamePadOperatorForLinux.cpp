#include <X11/Xlib.h>
#include <iostream>
#include "AutoGamePadOperatorForLinux.h"

AutoGamePadOperatorForLinux::AutoGamePadOperatorForLinux(Display* const display) : display(display) {
}

bool AutoGamePadOperatorForLinux::initialize(void) {
	if(this->display == NULL) {
		std::cout << "ŠÂ‹«•Ï” DISPLAY ‚ªÝ’è‚³‚ê‚Ä‚¢‚Ü‚¹‚ñ." << std::endl;
		return false;
	}

	return true;
}

void AutoGamePadOperatorForLinux::update(const CommonAutoOperator::ElapsedTime& elapsedTime) {
}

void AutoGamePadOperatorForLinux::onCommandGamePadButton(const CommonAutoOperator::ElapsedTime& elapsedTime, const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) {
}

void AutoGamePadOperatorForLinux::onCommandGamePadStick(const CommonAutoOperator::ElapsedTime& elapsedTime, const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) {
}