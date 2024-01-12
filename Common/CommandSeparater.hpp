#ifndef AUTO_OPERATOR_COMMAND_SEPARATER_H
# define AUTO_OPERATOR_COMMAND_SEPARATER_H

#include <string>
#include <vector>
#include <tuple>
#include <functional>
#include <chrono>
#include "Hash.hpp"

namespace CommonAutoOperator {
	using ElapsedTime = const std::chrono::milliseconds;
}

namespace CommandSeparator {
	enum class ControlCommand : Hash::HashType {
		KEY_DOWN = Hash::getHash("KeyDown"),
		SPECIAL_KEY_DOWN = Hash::getHash("SPKeyDown"),
		STRING = Hash::getHash("String"),
		GAMEPAD_BUTTON = Hash::getHash("GamePadButton"),
		GAMEPAD_STICK = Hash::getHash("GamePadStick"),
		MOUSE_CLICK = Hash::getHash("MouseClick"),
		MOUSE_CURSOR = Hash::getHash("MouseCursor"),
		MOUSE_SCROLL = Hash::getHash("MouseScroll"),
		CAPTURE = Hash::getHash("Capture"),
		IF = Hash::getHash("if"),
		ELSE = Hash::getHash("else"),
		FI = Hash::getHash("fi"),
	};

	std::vector<ControlCommand> getControlCommandList(void) {
		std::vector<ControlCommand> vec = {
			ControlCommand::GAMEPAD_STICK,
			ControlCommand::GAMEPAD_BUTTON,
			ControlCommand::KEY_DOWN,
			ControlCommand::SPECIAL_KEY_DOWN,
			ControlCommand::MOUSE_CURSOR,
			ControlCommand::MOUSE_CLICK,
			ControlCommand::MOUSE_SCROLL,
			ControlCommand::IF,
			ControlCommand::FI,
			ControlCommand::ELSE,
			ControlCommand::CAPTURE,
			ControlCommand::STRING,
		};
		return vec;
	}

	namespace Common {
		using PushTime = int;
		using CommandLine = std::string;
		using CommandLines = std::vector<std::string>;
	}

	// コマンド取得関連
	namespace Commands {
		using StartTime = int;
		using FullCommand = std::string;
		using Command = std::string;
		using Args = std::string;
		using SplitCommand = std::tuple<StartTime, Command, Args>;

		static constexpr char COMMENT_IDENTIFIER = '#';

		SplitCommand getCommand(const Common::CommandLine& line);
		bool isCommand(const Command& command);
		std::tuple<StartTime, FullCommand> splitTime(const Common::CommandLine& line);
		Args sliceComment(const Args& args);
	}

	// ゲームパッド関連
	namespace GamePad {
		using Button = int;
		using JoyStick = int;
		using JoyStickPosX = float;
		using JoyStickPosY = float;

		using ButtonArgs = std::tuple<Button, Common::PushTime>;
		using JoyStickArgs = std::tuple<JoyStick, JoyStickPosX, JoyStickPosY>;

		enum class JoyStickID {
			LEFT_STICK = 0,
			RIGHT_STICK = 1,
			TRIGGER = 2,
		};

		ButtonArgs getButtonArgs(const Commands::Args& args);
		JoyStickArgs getJoyStickArgs(const Commands::Args& args);
	}

	// キーボード関連
	namespace KeyBoard {
		using Key = std::string;
		using ModifyKeys = std::vector<std::string>;

		using KeyArgs = std::tuple<Key, ModifyKeys, Common::PushTime>;

		static constexpr char COMBINATION_IDENTIFIER = '+';

		KeyArgs getKeyArgs(const Commands::Args& args);
	}

	// マウス関連
	namespace Mouse {
		using Acction = std::string;
		using ClickButton = std::string;
		using CursolPosX = int;
		using CursolPosY = int;

		using ClickButtonArgs = std::tuple<ClickButton, CursolPosX, CursolPosY, Common::PushTime>;
		using CursolArgs = std::tuple<Acction, CursolPosX, CursolPosY, Common::PushTime>;

		ClickButtonArgs getClickButtonArgs(const Commands::Args& args);
		CursolArgs getCursorArgs(const Commands::Args& args);
	}
}
#endif