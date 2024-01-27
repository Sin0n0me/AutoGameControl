#include <iostream>
#include <X11/cursorfont.h>
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

	// カーソル作成
	Window window = XRootWindow(display, 0);
	Pixmap cursorPixmap = XCreatePixmap(display, window, 16, 16, 1);
	XColor fg, bg;
	const int screen = DefaultScreen(display);
	const auto black = BlackPixel(display, screen); 
	const auto white = WhitePixel(display, screen); 
	fg.pixel = black;
	bg.pixel = white;
	Cursor cursor = XCreatePixmapCursor(display, cursorPixmap, cursorPixmap, &fg, &bg, 0, 0);

	// ウィンドウにカーソルをセット
	XDefineCursor(display, window, cursor);

	autoOperator.run();

	XCloseDisplay(display);

	return 0;
}