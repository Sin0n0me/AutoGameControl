#include <iostream>
#include "AutoOperator.hpp"

AutoOperator::AutoOperator(void) : programStartTime(std::chrono::high_resolution_clock::now()) {
	this->useBufferIndex = 0;
	this->currentRow = 0;
	this->isFileLoadingCompleted = false;
	this->ifNestCounter = 0;

	this->loadFile();
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

void AutoOperator::swapBuffer(void) {
	std::lock_guard lock{this->mutex};

	this->bufferList[this->useBufferIndex].clear();
	this->useBufferIndex = this->getIndexOfUnusedBuffer();
}

void AutoOperator::loadFile(void) {
	std::lock_guard lock{this->mutex};

	const auto startLinePos = this->currentRow;
	const auto endLinePos = this->currentRow + AutoOperator::MaxLoadLines;
	this->currentRow += AutoOperator::MaxLoadLines;
	CommandSeparator::Common::CommandLines tempListBuffer;

	// 特定の範囲内の行を読み込み
	readLines(tempListBuffer, AutoOperator::FilePath, startLinePos, endLinePos);

	// 何も読み込めなければ全て読み込み完了
	if(tempListBuffer.empty()) {
		this->isFileLoadingCompleted = true;
	}

	// コマンドの分離など前処理
	auto& subBufferList = this->bufferList[this->getIndexOfUnusedBuffer()];
	for(const auto& line : tempListBuffer) {
		const auto& commands = CommandSeparator::Commands::getCommand(line);
		const auto& startTime = std::get<0>(commands);
		const auto& command = std::get<1>(commands);
		const auto& args = std::get<2>(commands);

		// コマンドと認識できるもので無ければリストに追加しない
		if(!CommandSeparator::Commands::isCommand(command)) {
			continue;
		}

		// コマンドと認識できるもののみ追加
		subBufferList.emplace_back(commands);
	}

	std::cout << subBufferList.size() << std::endl;
	std::cout << "ファイル読み込み完了" << std::endl;
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

		std::cout << "time:" << startTime << " command:" << command << " args:" << args << std::endl;

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
		} while(startTime > elapsedTime.count());

		// コマンドに応じ自動操作実行
		this->executeCommand(command, args);
	}
}

void AutoOperator::executeCommand(const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) const {
	const auto commandHash = Hash::getHash(command.c_str());

	// 各コマンドに応じた自動操作を行う
	switch(commandHash) {
	case static_cast<Hash::HashType>(CommandSeparator::ControlCommand::GAMEPAD_STICK):
		this->gamePadOperator->onCommandGamePadStick(command, args);	// ゲームパッドのジョイスティック操作
		return;
	case static_cast<Hash::HashType>(CommandSeparator::ControlCommand::GAMEPAD_BUTTON):
		this->gamePadOperator->onCommandGamePadButton(command, args);	// ゲームパッドのボタン操作
		return;
	case static_cast<Hash::HashType>(CommandSeparator::ControlCommand::KEY_DOWN):
	case static_cast<Hash::HashType>(CommandSeparator::ControlCommand::SPECIAL_KEY_DOWN):
		this->keyboardOperator->onCommandKeyDown(command, args);		// キーボードのキー操作
		return;
	case static_cast<Hash::HashType>(CommandSeparator::ControlCommand::MOUSE_CURSOR):
		this->mouseOperator->onCommandMouseCursor(command, args);		// マウスのカーソル操作
		return;
	case static_cast<Hash::HashType>(CommandSeparator::ControlCommand::MOUSE_CLICK):
		this->mouseOperator->onCommandMouseClick(command, args);		// マウスのクリック操作
		return;
	case static_cast<Hash::HashType>(CommandSeparator::ControlCommand::MOUSE_SCROLL):
		this->mouseOperator->onCommandMouseScroll(command, args);		// マウスのスクロール操作
		return;
	case static_cast<Hash::HashType>(CommandSeparator::ControlCommand::STRING):
		this->keyboardOperator->onCommandString(command, args);			// キーボードの文字入力操作
		return;
	default:
		break;
	}
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