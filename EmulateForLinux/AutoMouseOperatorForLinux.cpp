#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <iostream>
#include "AutoMouseOperatorForLinux.h"

using namespace CommandSeparator;

AutoMouseOperatorForLinux::AutoMouseOperatorForLinux(Display* const display) : display(display) {
}

bool AutoMouseOperatorForLinux::initialize(void) {
	if(this->display == NULL) {
		std::cout << "環境変数 DISPLAY が設定されていません." << std::endl;
		return false;
	}

	return true;
}

void AutoMouseOperatorForLinux::update(const CommonAutoOperator::ElapsedTime& elapsedTime) {

}

void AutoMouseOperatorForLinux::onCommandMouseClick(const CommonAutoOperator::ElapsedTime& elapsedTime, const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) {

}

void AutoMouseOperatorForLinux::onCommandMouseCursor(const CommonAutoOperator::ElapsedTime& elapsedTime, const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) {
	const auto& splitArgs = Mouse::getCursorArgs(args);
	const auto& action = std::get<0>(splitArgs);
	const auto& x = std::get<1>(splitArgs);
	const auto& y = std::get<2>(splitArgs);
	const auto& time = std::get<3>(splitArgs);

	if(action == "Move") {
		XWarpPointer(this->display, None, XRootWindow(this->display, 0), 0, 0, 0, 0, x, y); 
		XFlush(this->display);
	}

}

void AutoMouseOperatorForLinux::onCommandMouseScroll(const CommonAutoOperator::ElapsedTime& elapsedTime, const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) {
}