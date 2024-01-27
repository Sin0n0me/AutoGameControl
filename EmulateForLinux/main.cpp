#include <iostream>
#include "../Common/AutoOperator.hpp"
#include "AutoGamePadOperatorForLinux.h"
#include "AutoKeyboardOperatorForLinux.h"
#include "AutoMouseOperatorForLinux.h"


int main(void) {

	std::cout << "Target DISPLAY: " << getenv("DISPLAY") << std::endl;

	Display* const display = XOpenDisplay(NULL);
	AutoOperator autoOperator;

	autoOperator.registerGamePadOperator<AutoGamePadOperatorForLinux>(display);
	autoOperator.registerKeyboardOperator<AutoKeyboardOperatorForLinux>(display);
	autoOperator.registerMouseOperator<AutoMouseOperatorForLinux>(display);

	if(!autoOperator.initialize()) {
		return 1;
	}

	autoOperator.run();

	XCloseDisplay(display); 

	return 0;
}