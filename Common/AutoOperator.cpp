#include <iostream>
#include "AutoOperator.hpp"

AutoOperator::AutoOperator(void) : programStartTime(std::chrono::high_resolution_clock::now()) {
	this->useBufferIndex = 0;
	this->currentRow = 0;
	this->isFileLoadingCompleted = false;
	this->ifNestCounter = 0;

	// 初期読み込み&バッファスワップ
	this->loadFile();
	this->swapBuffer();

	// コマンドに応じた関数の登録

	// ゲームパッドのジョイスティック操作
	using namespace CommandSeparator;
	this->registerCommandFunc(ControlCommand::GAMEPAD_STICK,
		[this](const CommonAutoOperator::ElapsedTime& elapsedTime, const Commands::Command& command, const Commands::Args& args) {
			this->gamePadOperator->onCommandGamePadStick(elapsedTime, command, args);
		}
	);

	// ゲームパッドのボタン操作
	this->registerCommandFunc(ControlCommand::GAMEPAD_BUTTON,
		[this](const CommonAutoOperator::ElapsedTime& elapsedTime, const Commands::Command& command, const Commands::Args& args) {
			this->gamePadOperator->onCommandGamePadButton(elapsedTime, command, args);
		}
	);

	// キーボードのキー操作
	this->registerCommandFunc(ControlCommand::KEY_DOWN,
		[this](const CommonAutoOperator::ElapsedTime& elapsedTime, const Commands::Command& command, const Commands::Args& args) {
			this->keyboardOperator->onCommandKeyDown(elapsedTime, command, args);
		}
	);

	// キーボードのキー操作(装飾キー)
	this->registerCommandFunc(ControlCommand::SPECIAL_KEY_DOWN,
		[this](const CommonAutoOperator::ElapsedTime& elapsedTime, const Commands::Command& command, const Commands::Args& args) {
			this->keyboardOperator->onCommandKeyDown(elapsedTime, command, args);
		}
	);

	// キーボードの文字列入力操作
	this->registerCommandFunc(ControlCommand::STRING,
		[this](const CommonAutoOperator::ElapsedTime& elapsedTime, const Commands::Command& command, const Commands::Args& args) {
			this->keyboardOperator->onCommandString(elapsedTime, command, args);
		}
	);

	// マウスのカーソル操作
	this->registerCommandFunc(ControlCommand::MOUSE_CURSOR,
		[this](const CommonAutoOperator::ElapsedTime& elapsedTime, const Commands::Command& command, const Commands::Args& args) {
			this->mouseOperator->onCommandMouseCursor(elapsedTime, command, args);
		}
	);

	// マウスのクリック操作
	this->registerCommandFunc(ControlCommand::MOUSE_CLICK,
		[this](const CommonAutoOperator::ElapsedTime& elapsedTime, const Commands::Command& command, const Commands::Args& args) {
			this->mouseOperator->onCommandMouseClick(elapsedTime, command, args);
		}
	);

	// マウスのスクロール操作
	this->registerCommandFunc(ControlCommand::MOUSE_SCROLL,
		[this](const CommonAutoOperator::ElapsedTime& elapsedTime, const Commands::Command& command, const Commands::Args& args) {
			this->mouseOperator->onCommandMouseScroll(elapsedTime, command, args);
		}
	);
}

AutoOperator::~AutoOperator(void) noexcept {
	if(this->fileLoadThread.joinable()) {
		this->fileLoadThread.join();
	}
}

bool AutoOperator::initialize(void) {
	if(!bool(this->gamePadOperator)) {
		std::cout << "ゲームパッド操作用のクラスが設定されていません" << std::endl;
		return false;
	}
	if(!bool(this->keyboardOperator)) {
		std::cout << "キーボード操作用のクラスが設定されていません" << std::endl;
		return false;
	}
	if(!bool(this->mouseOperator)) {
		std::cout << "マウス操作用のクラスが設定されていません" << std::endl;
		return false;
	}

	// 初期化
	IntAutoOperator* const intAutoOperator[] = {
		this->gamePadOperator.get(),
		this->keyboardOperator.get(),
		this->mouseOperator.get()
	};
	for(auto autoOperator : intAutoOperator) {
		if(!autoOperator->initialize()) {
			return false;
		}
	}

	return true;
}

void AutoOperator::run(void) {
	for(; !this->isFileLoadingCompleted;) {
		// ファイルの読み込みはマルチスレッドで処理
		this->fileLoadThread = std::thread(&AutoOperator::loadFile, this);

		// 自動操作
		this->autoControl(this->bufferList[this->useBufferIndex]);

		// 自動操作完了後にjoinしバッファ入れ替え(indexの移動)
		this->fileLoadThread.join();
		this->swapBuffer();
	}
}

unsigned char AutoOperator::getIndexOfUnusedBuffer(void) const {
	return this->useBufferIndex == 0 ? 1 : 0;
}

void AutoOperator::registerCommandFunc(const CommandSeparator::ControlCommand& command, const Func& func) {
	this->commandFunc[static_cast<Hash::HashType>(command)] = func;
}

void AutoOperator::swapBuffer(void) {
	std::lock_guard lock{this->mutex};

	this->bufferList[this->useBufferIndex].clear();
	this->useBufferIndex = this->getIndexOfUnusedBuffer();
}

void AutoOperator::loadFile(void) {
	// 全て読み込み完了した場合は何もしない
	if(this->isFileLoadingCompleted) {
		return;
	}

	std::lock_guard lock{this->mutex};

	const auto loadStartTime = std::chrono::high_resolution_clock::now();
	const auto startLinePos = this->currentRow;
	const auto endLinePos = this->currentRow + AutoOperator::MaxLoadLines;
	CommandSeparator::Common::CommandLines tempListBuffer;

	// 特定の範囲内の行を読み込み
	readLines(tempListBuffer, AutoOperator::FilePath, startLinePos, endLinePos - 1);
	this->currentRow += tempListBuffer.size();

	// 何も読み込めなければ全て読み込み完了
	if(tempListBuffer.empty()) {
		this->isFileLoadingCompleted = true;
	}

	// コマンドの分離など前処理
	const auto unusedBufferIndex = this->getIndexOfUnusedBuffer();
	auto& subBufferList = this->bufferList[unusedBufferIndex];
	for(const auto& line : tempListBuffer) {
		const auto& commands = CommandSeparator::Commands::getCommand(line);
		const auto& startTime = std::get<0>(commands);
		const auto& command = std::get<1>(commands);
		const auto& args = std::get<2>(commands);

		// コマンドと認識できるもので無ければリストに追加しない
		if(!CommandSeparator::Commands::isCommand(command)) {
			continue;
		}
		//std::cout << "start time: " << static_cast<long long>(startTime) << " command: " << command << " args:" << args << std::endl;

		// コマンドと認識できるもののみ追加
		subBufferList.emplace_back(commands);
	}

	const auto diff = std::chrono::high_resolution_clock::now() - loadStartTime;

	std::cout << "================================" << std::endl;
	std::cout << "ファイル読み込み完了" << std::endl;
	std::cout << "更新バッファインデックス: " << static_cast<int>(unusedBufferIndex) << std::endl;
	std::cout << "現在の読み取り行: " << this->currentRow << std::endl;
	std::cout << "読み取り行数: " << tempListBuffer.size() << std::endl;
	std::cout << "読み取りコマンド数: " << subBufferList.size() << std::endl;
	std::cout << "読み取り時間: " << std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() << "ms" << std::endl;
	std::cout << "================================" << std::endl;
}

void AutoOperator::autoControl(const BufferList& lines) {
	IntAutoOperator* const updater[] = {
		this->gamePadOperator.get(),
		this->keyboardOperator.get(),
		this->mouseOperator.get()
	};

	for(const auto& commands : lines) {
		const auto& startTime = std::get<0>(commands);
		const auto& command = std::get<1>(commands);
		const auto& args = std::get<2>(commands);

		// 条件式等によりスキップするかどうか判定
		if(this->isSkipAutoControl(command, args)) {
			continue;
		}

		// コマンド開始時間になるまでは待機
		auto elapsedTime = this->getElapsedTime();
		do {
			for(auto autoOperator : updater) {
				autoOperator->update(elapsedTime);
			}
			elapsedTime = this->getElapsedTime();
		} while(static_cast<long long>(startTime - 1) > elapsedTime.count());

		// 時間計測
		const auto delayTime = elapsedTime.count() - startTime;

		// コマンドに応じ自動操作実行
		this->executeCommand(command, args);

		constexpr double fps = 1.0f / AutoOperator::MaxFrameRate * 1000;
		if(static_cast<double>(delayTime) > fps) {
			std::cout << "[遅延発生!] 遅延時間:" << delayTime << " ms" << std::endl;
			std::cout << "time: " << startTime << " command: " << command << " args:" << args << std::endl;
		}
	}
}

void AutoOperator::executeCommand(const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) const {
	const auto commandHash = Hash::getHash(command.c_str());
	const auto elapsedTime = this->getElapsedTime();

	// 各コマンドに応じた自動操作を行う
	this->commandFunc.at(commandHash)(elapsedTime, command, args);
}

bool AutoOperator::isSkipAutoControl(const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) {
	const auto commandHash = Hash::getHash(command.c_str());

	// 条件式チェック
	// 条件式が成り立った場合はそのまま次の行へ
	// 成り立たない場合はelseもしくはfiがくるまで何もしない
	if(commandHash == static_cast<Hash::HashType>(CommandSeparator::ControlCommand::IF)) {
		// 条件が成り立たない場合はスキップフラグを立てる
		this->nestHashMap[this->ifNestCounter] = false;	// 条件式チェックを作成していないので仮
		++this->ifNestCounter;
		return true;
	}

	// 前のifの条件が成り立っていない場合はスキップフラグを解除
	if(commandHash == static_cast<Hash::HashType>(CommandSeparator::ControlCommand::ELSE)) {
		if(this->nestHashMap[this->ifNestCounter - 1]) {
			this->nestHashMap[this->ifNestCounter - 1] = false;
		}
		return true;
	}

	// 終了時はスキップフラグの解除とネストカウンタの減算
	if(commandHash == static_cast<Hash::HashType>(CommandSeparator::ControlCommand::FI)) {
		this->nestHashMap[this->ifNestCounter] = false;
		--this->ifNestCounter;
		return true;
	}

	// 条件に応じた範囲でなければfiもしくはelseが来るまでコマンドスキップ
	if(this->ifNestCounter > 0) {
		if(this->nestHashMap[this->ifNestCounter - 1]) {
			return true;
		}
	}

	return false;
}

CommonAutoOperator::ElapsedTime AutoOperator::getElapsedTime(void) const {
	const auto startTime = this->programStartTime;
	const auto currentTime = std::chrono::high_resolution_clock::now();
	const auto diff = currentTime - startTime;
	const auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(diff);
	return elapsedTime;
}