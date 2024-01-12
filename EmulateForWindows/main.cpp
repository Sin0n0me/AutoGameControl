#include "../Common/AutoOperator.hpp"
#include "AutoGamePadOperatorForWIndows.h"
#include "AutoKeyboardOperatorForWindows.h"
#include "AutoMouseOperatorForWindows.h"

int main(void) {
	AutoOperator autoOperator;

	autoOperator.registerGamePadOperator<AutoGamePadOperatorForWIndows>();
	autoOperator.registerKeyboardOperator<AutoKeyboardOperatorForWindows>();
	autoOperator.registerMouseOperator<AutoMouseOperatorForWindows>();

	if(!autoOperator.initialize()) {
		return 1;
	}

	autoOperator.run();

	return 0;
}