#include <X11/Xlib.h>
#include "AutoGamePadOperatorForLinux.h"

AutoGamePadOperatorForLinux::AutoGamePadOperatorForLinux(Display* const display): display(display){
}

bool AutoGamePadOperatorForLinux::initialize(void) {
	return true;
}

void AutoGamePadOperatorForLinux::update(const CommonAutoOperator::ElapsedTime& elapsedTime) {

}

void AutoGamePadOperatorForLinux::onCommandGamePadButton(const CommonAutoOperator::ElapsedTime& elapsedTime, const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) {
}

void AutoGamePadOperatorForLinux::onCommandGamePadStick(const CommonAutoOperator::ElapsedTime& elapsedTime, const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) {
}