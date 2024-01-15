#include <iostream>
#include "AutoOperator.hpp"

AutoOperator::AutoOperator(void) : programStartTime(std::chrono::high_resolution_clock::now()) {
	this->useBufferIndex = 0;
	this->currentRow = 0;
	this->isFileLoadingCompleted = false;
	this->ifNestCounter = 0;

	// �����ǂݍ���&�o�b�t�@�X���b�v
	this->loadFile();
	this->swapBuffer();

	// �R�}���h�ɉ������֐��̓o�^

	// �Q�[���p�b�h�̃W���C�X�e�B�b�N����
	using namespace CommandSeparator;
	this->registerCommandFunc(ControlCommand::GAMEPAD_STICK,
		[this](const CommonAutoOperator::ElapsedTime& elapsedTime, const Commands::Command& command, const Commands::Args& args) {
			this->gamePadOperator->onCommandGamePadStick(elapsedTime, command, args);
		}
	);

	// �Q�[���p�b�h�̃{�^������
	this->registerCommandFunc(ControlCommand::GAMEPAD_BUTTON,
		[this](const CommonAutoOperator::ElapsedTime& elapsedTime, const Commands::Command& command, const Commands::Args& args) {
			this->gamePadOperator->onCommandGamePadButton(elapsedTime, command, args);
		}
	);

	// �L�[�{�[�h�̃L�[����
	this->registerCommandFunc(ControlCommand::KEY_DOWN,
		[this](const CommonAutoOperator::ElapsedTime& elapsedTime, const Commands::Command& command, const Commands::Args& args) {
			this->keyboardOperator->onCommandKeyDown(elapsedTime, command, args);
		}
	);

	// �L�[�{�[�h�̃L�[����(�����L�[)
	this->registerCommandFunc(ControlCommand::SPECIAL_KEY_DOWN,
		[this](const CommonAutoOperator::ElapsedTime& elapsedTime, const Commands::Command& command, const Commands::Args& args) {
			this->keyboardOperator->onCommandKeyDown(elapsedTime, command, args);
		}
	);

	// �L�[�{�[�h�̕�������͑���
	this->registerCommandFunc(ControlCommand::STRING,
		[this](const CommonAutoOperator::ElapsedTime& elapsedTime, const Commands::Command& command, const Commands::Args& args) {
			this->keyboardOperator->onCommandString(elapsedTime, command, args);
		}
	);

	// �}�E�X�̃J�[�\������
	this->registerCommandFunc(ControlCommand::MOUSE_CURSOR,
		[this](const CommonAutoOperator::ElapsedTime& elapsedTime, const Commands::Command& command, const Commands::Args& args) {
			this->mouseOperator->onCommandMouseCursor(elapsedTime, command, args);
		}
	);

	// �}�E�X�̃N���b�N����
	this->registerCommandFunc(ControlCommand::MOUSE_CLICK,
		[this](const CommonAutoOperator::ElapsedTime& elapsedTime, const Commands::Command& command, const Commands::Args& args) {
			this->mouseOperator->onCommandMouseClick(elapsedTime, command, args);
		}
	);

	// �}�E�X�̃X�N���[������
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

void AutoOperator::registerCommandFunc(const CommandSeparator::ControlCommand& command, const Func& func) {
	this->commandFunc[static_cast<Hash::HashType>(command)] = func;
}

void AutoOperator::swapBuffer(void) {
	std::lock_guard lock{this->mutex};

	this->bufferList[this->useBufferIndex].clear();
	this->useBufferIndex = this->getIndexOfUnusedBuffer();
}

void AutoOperator::loadFile(void) {
	// �S�ēǂݍ��݊��������ꍇ�͉������Ȃ�
	if(this->isFileLoadingCompleted) {
		return;
	}

	std::lock_guard lock{this->mutex};

	const auto loadStartTime = std::chrono::high_resolution_clock::now();
	const auto startLinePos = this->currentRow;
	const auto endLinePos = this->currentRow + AutoOperator::MaxLoadLines;
	CommandSeparator::Common::CommandLines tempListBuffer;

	// ����͈͓̔��̍s��ǂݍ���
	readLines(tempListBuffer, AutoOperator::FilePath, startLinePos, endLinePos - 1);
	this->currentRow += tempListBuffer.size();

	// �����ǂݍ��߂Ȃ���ΑS�ēǂݍ��݊���
	if(tempListBuffer.empty()) {
		this->isFileLoadingCompleted = true;
	}

	// �R�}���h�̕����ȂǑO����
	const auto unusedBufferIndex = this->getIndexOfUnusedBuffer();
	auto& subBufferList = this->bufferList[unusedBufferIndex];
	for(const auto& line : tempListBuffer) {
		const auto& commands = CommandSeparator::Commands::getCommand(line);
		const auto& startTime = std::get<0>(commands);
		const auto& command = std::get<1>(commands);
		const auto& args = std::get<2>(commands);

		// �R�}���h�ƔF���ł�����̂Ŗ�����΃��X�g�ɒǉ����Ȃ�
		if(!CommandSeparator::Commands::isCommand(command)) {
			continue;
		}
		//std::cout << "start time: " << static_cast<long long>(startTime) << " command: " << command << " args:" << args << std::endl;

		// �R�}���h�ƔF���ł�����̂̂ݒǉ�
		subBufferList.emplace_back(commands);
	}

	const auto diff = std::chrono::high_resolution_clock::now() - loadStartTime;

	std::cout << "================================" << std::endl;
	std::cout << "�t�@�C���ǂݍ��݊���" << std::endl;
	std::cout << "�X�V�o�b�t�@�C���f�b�N�X: " << static_cast<int>(unusedBufferIndex) << std::endl;
	std::cout << "���݂̓ǂݎ��s: " << this->currentRow << std::endl;
	std::cout << "�ǂݎ��s��: " << tempListBuffer.size() << std::endl;
	std::cout << "�ǂݎ��R�}���h��: " << subBufferList.size() << std::endl;
	std::cout << "�ǂݎ�莞��: " << std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() << "ms" << std::endl;
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
		} while(static_cast<long long>(startTime - 1) > elapsedTime.count());

		// ���Ԍv��
		const auto delayTime = elapsedTime.count() - startTime;

		// �R�}���h�ɉ�������������s
		this->executeCommand(command, args);

		constexpr double fps = 1.0f / AutoOperator::MaxFrameRate * 1000;
		if(static_cast<double>(delayTime) > fps) {
			std::cout << "[�x������!] �x������:" << delayTime << " ms" << std::endl;
			std::cout << "time: " << startTime << " command: " << command << " args:" << args << std::endl;
		}
	}
}

void AutoOperator::executeCommand(const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) const {
	const auto commandHash = Hash::getHash(command.c_str());
	const auto elapsedTime = this->getElapsedTime();

	// �e�R�}���h�ɉ���������������s��
	this->commandFunc.at(commandHash)(elapsedTime, command, args);
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