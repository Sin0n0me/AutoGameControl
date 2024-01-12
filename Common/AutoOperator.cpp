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
		std::cout << "�Q�[���p�b�h����p�̃N���X���ݒ肳��Ă��܂���" << std::endl;
		return false;
	}
	if(!bool(this->keyboardOperator)) {
		std::cout << "�L�[�{�[�h����p�̃N���X���ݒ肳��Ă��܂���" << std::endl;
		return false;
	}
	if(!bool(this->mouseOperator)) {
		std::cout << "�}�E�X����p�̃N���X���ݒ肳��Ă��܂���" << std::endl;
		return false;
	}

	// ������
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
		// �t�@�C���̓ǂݍ��݂̓}���`�X���b�h�ŏ���
		this->fileLoadThread = std::thread(&AutoOperator::loadFile, this);

		// ��������
		this->autoControl(this->bufferList[this->useBufferIndex]);

		// �������슮�����join���o�b�t�@����ւ�(index�̈ړ�)
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

	// ����͈͓̔��̍s��ǂݍ���
	readLines(tempListBuffer, AutoOperator::FilePath, startLinePos, endLinePos);

	// �����ǂݍ��߂Ȃ���ΑS�ēǂݍ��݊���
	if(tempListBuffer.empty()) {
		this->isFileLoadingCompleted = true;
	}

	// �R�}���h�̕����ȂǑO����
	auto& subBufferList = this->bufferList[this->getIndexOfUnusedBuffer()];
	for(const auto& line : tempListBuffer) {
		const auto& commands = CommandSeparator::Commands::getCommand(line);
		const auto& startTime = std::get<0>(commands);
		const auto& command = std::get<1>(commands);
		const auto& args = std::get<2>(commands);

		// �R�}���h�ƔF���ł�����̂Ŗ�����΃��X�g�ɒǉ����Ȃ�
		if(!CommandSeparator::Commands::isCommand(command)) {
			continue;
		}

		// �R�}���h�ƔF���ł�����̂̂ݒǉ�
		subBufferList.emplace_back(commands);
	}

	std::cout << subBufferList.size() << std::endl;
	std::cout << "�t�@�C���ǂݍ��݊���" << std::endl;
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

		// ���������ɂ��X�L�b�v���邩�ǂ�������
		if(this->isSkipAutoControl(command, args)) {
			continue;
		}

		// �R�}���h�J�n���ԂɂȂ�܂ł͑ҋ@
		auto elapsedTime = this->getElapsedTime();
		do {
			for(auto autoOperator : updater) {
				autoOperator->update(elapsedTime);
			}
			elapsedTime = this->getElapsedTime();
		} while(startTime > elapsedTime.count());

		// �R�}���h�ɉ�������������s
		this->executeCommand(command, args);
	}
}

void AutoOperator::executeCommand(const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) const {
	const auto commandHash = Hash::getHash(command.c_str());

	// �e�R�}���h�ɉ���������������s��
	switch(commandHash) {
	case static_cast<Hash::HashType>(CommandSeparator::ControlCommand::GAMEPAD_STICK):
		this->gamePadOperator->onCommandGamePadStick(command, args);	// �Q�[���p�b�h�̃W���C�X�e�B�b�N����
		return;
	case static_cast<Hash::HashType>(CommandSeparator::ControlCommand::GAMEPAD_BUTTON):
		this->gamePadOperator->onCommandGamePadButton(command, args);	// �Q�[���p�b�h�̃{�^������
		return;
	case static_cast<Hash::HashType>(CommandSeparator::ControlCommand::KEY_DOWN):
	case static_cast<Hash::HashType>(CommandSeparator::ControlCommand::SPECIAL_KEY_DOWN):
		this->keyboardOperator->onCommandKeyDown(command, args);		// �L�[�{�[�h�̃L�[����
		return;
	case static_cast<Hash::HashType>(CommandSeparator::ControlCommand::MOUSE_CURSOR):
		this->mouseOperator->onCommandMouseCursor(command, args);		// �}�E�X�̃J�[�\������
		return;
	case static_cast<Hash::HashType>(CommandSeparator::ControlCommand::MOUSE_CLICK):
		this->mouseOperator->onCommandMouseClick(command, args);		// �}�E�X�̃N���b�N����
		return;
	case static_cast<Hash::HashType>(CommandSeparator::ControlCommand::MOUSE_SCROLL):
		this->mouseOperator->onCommandMouseScroll(command, args);		// �}�E�X�̃X�N���[������
		return;
	case static_cast<Hash::HashType>(CommandSeparator::ControlCommand::STRING):
		this->keyboardOperator->onCommandString(command, args);			// �L�[�{�[�h�̕������͑���
		return;
	default:
		break;
	}
}

bool AutoOperator::isSkipAutoControl(const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) {
	const auto commandHash = Hash::getHash(command.c_str());

	// �������`�F�b�N
	// �����������藧�����ꍇ�͂��̂܂܎��̍s��
	// ���藧���Ȃ��ꍇ��else��������fi������܂ŉ������Ȃ�
	if(commandHash == static_cast<Hash::HashType>(CommandSeparator::ControlCommand::IF)) {
		// ���������藧���Ȃ��ꍇ�̓X�L�b�v�t���O�𗧂Ă�
		this->nestHashMap[this->ifNestCounter] = false;	// �������`�F�b�N���쐬���Ă��Ȃ��̂ŉ�
		++this->ifNestCounter;
		return true;
	}

	// �O��if�̏��������藧���Ă��Ȃ��ꍇ�̓X�L�b�v�t���O������
	if(commandHash == static_cast<Hash::HashType>(CommandSeparator::ControlCommand::ELSE)) {
		if(this->nestHashMap[this->ifNestCounter - 1]) {
			this->nestHashMap[this->ifNestCounter - 1] = false;
		}
		return true;
	}

	// �I�����̓X�L�b�v�t���O�̉����ƃl�X�g�J�E���^�̌��Z
	if(commandHash == static_cast<Hash::HashType>(CommandSeparator::ControlCommand::FI)) {
		this->nestHashMap[this->ifNestCounter] = false;
		--this->ifNestCounter;
		return true;
	}

	// �����ɉ������͈͂łȂ����fi��������else������܂ŃR�}���h�X�L�b�v
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