#include <regex>
#include "Common.hpp"
#include "CommandSeparater.hpp"

using namespace CommandSeparator;

std::tuple<Commands::StartTime, Commands::Command, Commands::Args> Commands::getCommand(const Common::CommandLine& line) {
	Commands::StartTime startTime = -1;
	Commands::Command command = "";
	Commands::Args args;

	// コマンドの定義に従っているかチェック
	const auto semicolon_pos = line.find(';');
	if(semicolon_pos == std::string::npos && line.find(':') == std::string::npos) {
		return std::make_tuple(startTime, command, args);
	}

	// 開始時間, コマンド, 引数に分割する
	// セミコロンがない場合は条件式
	std::string fullCommand;
	if(semicolon_pos == std::string::npos) {
		fullCommand = line;
	} else {
		const std::regex time_match(R"(^(\d+):(\d+).(\d+)$)");
		const std::string startTimeString = line.substr(0, semicolon_pos);
		std::smatch match;
		fullCommand = line.substr(semicolon_pos + 1);

		if(std::regex_match(startTimeString, match, time_match)) {
			const int minutes = std::stoi(match[1]);
			const int seconds = std::stoi(match[2]);
			const int milli_seconds = std::stoi(match[3]);
			startTime = minutes * 60000 + seconds * 1000 + milli_seconds;
		} else {
			return std::make_tuple(startTime, command, args);
		}
	}

	// コマンド, 引数に分割する
	const auto colon_pos = fullCommand.find(':');
	if(colon_pos != std::string::npos) {
		command = fullCommand.substr(0, colon_pos);
		args = fullCommand.substr(colon_pos + 1);
	} else {
		command = fullCommand;
	}

	// コメント部分の切り取り
	args = Commands::sliceComment(args);

	return std::make_tuple(startTime, command, args);
}

bool CommandSeparator::Commands::isCommand(const Command& command) {
	const auto& commandList = getControlCommandList();
	const auto hash = Hash::getHash(command.c_str());
	for(const auto& command : commandList) {
		if(hash == static_cast<decltype(hash)>(command)) {
			return true;
		}
	}

	return false;
}

std::tuple<Commands::StartTime, Commands::FullCommand> Commands::splitTime(const Common::CommandLine& line) {
	Commands::StartTime startTime = -1;
	Commands::FullCommand fullCommand;

	if(line.find(';') == std::string::npos) {
		return std::make_tuple(startTime, fullCommand);
	}

	return std::make_tuple(startTime, fullCommand);
}

Commands::Args Commands::sliceComment(const Commands::Args& args) {
	// コメントアウトチェック
	// 引数部分はキーかコメントか不明なので切り分け
	// 1つの場合は:直後に存在するか(スペースは含まない)で判定する
	const auto  argsSharpPos = args.find(Commands::COMMENT_IDENTIFIER);
	if(argsSharpPos == std::string::npos) {
		return args;
	}

	// 2つ以上#が存在する場合は2つ目以降をコメントアウトとする
	auto slicePos = args.length();
	const auto argsSharpPos2 = args.find(Commands::COMMENT_IDENTIFIER, argsSharpPos + 1);
	if(argsSharpPos2 == std::string::npos) {
		// タブもしくはスペース以外が存在した場合は#押下ではない
		bool is_key_down_shape = true;
		for(size_t i = 0; i < argsSharpPos; ++i) {
			if(args[i] != ' ' && args[i] != '\t') {
				is_key_down_shape = false;
				break;
			}
		}

		// #キーでない場合は#以降を切り取り
		if(!is_key_down_shape) {
			slicePos = argsSharpPos;
		}
	} else {
		slicePos = argsSharpPos + argsSharpPos2 + 1;
	}

	return args.substr(0, slicePos);
}

// args format:
// <button:int> <[option] push time(ms):int>
// example:
// 0
// 5 300
GamePad::ButtonArgs GamePad::getButtonArgs(const Commands::Args& args) {
	const auto& splitArgs = split(strip(args));
	const Button& button = std::stoi(splitArgs[0]); // button = args.strip().split()[0]
	const Common::PushTime& pushTime = splitArgs.size() > 1 ? std::stoi(splitArgs[1]) : 0; // duration = args.split()[1] if len(args.split()) > 1 else 0
	return ButtonArgs(button, pushTime);
}

// args format:
// <joystick:int> <x:float> <y:float>
// example:
// 0 0.125 0.0625
GamePad::JoyStickArgs GamePad::getJoyStickArgs(const Commands::Args& args) {
	// stick, x, y = args.strip().split()
	const auto& splitArgs = split(strip(args));
	const JoyStick& joyStick = std::stoi(splitArgs[0]);
	const JoyStickPosX& joyStickPosX = std::stof(splitArgs[1]);
	const JoyStickPosY& joyStickPosY = std::stof(splitArgs[2]);
	return JoyStickArgs(joyStick, joyStickPosX, joyStickPosY);
}

// args format:
// <key:string><[option] +modify keys:string> <[option] push time(ms):int>
// example:
// x
// z 1500
// t+shift 200
KeyBoard::KeyArgs KeyBoard::getKeyArgs(const Commands::Args& args) {
	// keys = args.strip().split()[0]
	const auto& splitArgs = split(strip(args));
	const auto& keys = splitArgs[0];
	Key key;
	ModifyKeys modifyKeys;
	Common::PushTime time = -1;

	// 押下するキーの切り抜き
	const auto identifirePos = keys.find(COMBINATION_IDENTIFIER);
	if(identifirePos == std::string::npos) {
		key = keys;
	} else if(identifirePos == 0) {
		// 修飾キーの切り抜き
		key = COMBINATION_IDENTIFIER;
		if(keys.find('+', 1) != std::string::npos) {
			modifyKeys = split(keys.substr(1), COMBINATION_IDENTIFIER);
		}
	} else {
		const auto splitKeys = split(keys, COMBINATION_IDENTIFIER);
		key = splitKeys[0];
		modifyKeys = splitKeys;
	}

	// 押下時間のチェック
	const auto size = splitArgs.size();
	if(size > 1) {
		time = std::stoi(splitArgs[size - 1]);
	}

	return KeyArgs(key, modifyKeys, time);
}

// args format:
// <click button:int> <x:int> <y:int> <[option] push time(ms):int>
// example:
// left 100 200
// right 200 400 800
Mouse::ClickButtonArgs Mouse::getClickButtonArgs(const Commands::Args& args) {
	const auto& splitArgs = split(strip(args));
	const ClickButton& button = splitArgs[0];
	const CursolPosX& x = std::stoi(splitArgs[1]);
	const CursolPosY& y = std::stoi(splitArgs[2]);
	const Common::PushTime time = splitArgs.size() > 3 ? std::stoi(splitArgs[3]) : -1;
	return ClickButtonArgs(button, x, y, time);
}

// args format:
// <action> <x:int> <y:int> <[option] push time(ms):int>
// example:
// Move 100 200
// Move 200 400 800
Mouse::CursolArgs Mouse::getCursorArgs(const Commands::Args& args) {
	const auto& splitArgs = split(strip(args));
	const Acction& action = splitArgs[0];
	const CursolPosX& x = std::stoi(splitArgs[1]);
	const CursolPosY& y = std::stoi(splitArgs[2]);
	const Common::PushTime time = splitArgs.size() > 3 ? std::stoi(splitArgs[3]) : -1;
	return CursolArgs(action, x, y, time);
}