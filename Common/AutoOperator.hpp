#ifndef AUTO_OPERATOR_H
# define AUTO_OPERATOR_H

#include <vector>
#include <mutex>
#include <thread>
#include <string>
#include <chrono>
#include <memory>
#include <unordered_map>
#include "Common.hpp"
#include "CommandSeparater.hpp"
#include "IntAutoGamePadOperator.hpp"
#include "IntAutoKeyboardOperator.hpp"
#include "IntAutoMouseOperator.hpp"

class AutoOperator {
private:
	using BufferList = std::vector<CommandSeparator::Commands::SplitCommand>;
	using Func = std::function<void(const CommonAutoOperator::ElapsedTime&, const CommandSeparator::Commands::Command&, const CommandSeparator::Commands::Args&)>;
	std::unordered_map<Hash::HashType, Func> commandFunc;

	static constexpr unsigned int MaxFrameRate = 120;
	static constexpr unsigned int MaxBufferList = 2;
	static constexpr unsigned int MaxLoadLines = 500;
	static constexpr char FilePath[] = "captured_inputs.txt";

	std::unique_ptr<IntAutoGamePadOperator> gamePadOperator;
	std::unique_ptr<IntAutoKeyboardOperator> keyboardOperator;
	std::unique_ptr<IntAutoMouseOperator> mouseOperator;

	const std::chrono::steady_clock::time_point programStartTime;
	std::thread fileLoadThread;
	std::mutex mutex;
	BufferList bufferList[MaxBufferList];
	unsigned char useBufferIndex;
	size_t currentRow;

	// ネスト関連
	std::unordered_map<int, bool> nestHashMap;
	unsigned int ifNestCounter;
	bool isFileLoadingCompleted;

	void swapBuffer(void);
	void loadFile(void);

	bool isSkipAutoControl(const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args);
	void autoControl(const BufferList& bufferList);
	void executeCommand(const CommandSeparator::Commands::Command& command, const CommandSeparator::Commands::Args& args) const;

	CommonAutoOperator::ElapsedTime getElapsedTime(void) const;
	unsigned char getIndexOfUnusedBuffer(void) const;

	void registerCommandFunc(const CommandSeparator::ControlCommand& command, const Func& func);

public:

	explicit AutoOperator(void);
	virtual ~AutoOperator(void) noexcept;

	template <class GamePadOperator>
	void registerGamePadOperator(void) {
		this->gamePadOperator = std::make_unique<GamePadOperator>();
	}
	template <class KeyboardOperator>
	void registerKeyboardOperator(void) {
		this->keyboardOperator = std::make_unique<KeyboardOperator>();
	}
	template <class MouseOperator>
	void registerMouseOperator(void) {
		this->mouseOperator = std::make_unique<MouseOperator>();
	}

	bool initialize(void);

	void run(void);
};
#endif