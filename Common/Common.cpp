#include <fstream>
#include <iostream>
#include <sstream>
#include "Common.hpp"

// Pythonのisliceのような動きをする関数
void readLines(std::vector<std::string>& lines, const std::string& filePath, const int startLine, const int endLine) {
	std::ifstream file(filePath);
	if(!file.is_open()) {
		std::cerr << "ファイルを開けませんでした: " << filePath << std::endl;
		throw;
	}

	std::string line;
	for(int current_line = 0; std::getline(file, line); ++current_line) {
		// 指定範囲を下回っている間はcontinue
		if(current_line < startLine) {
			continue;
		}
		// 指定範囲を上回ったらそれ以上ループするのは無駄なのでbreakで抜ける
		if(current_line > endLine) {
			break;
		}

		lines.emplace_back(line);
	}
}

std::vector<std::string> split(const std::string& str, const char delimiter) {
	std::vector<std::string> result;
	std::istringstream iss(str);

	for(std::string clippedString; getline(iss, clippedString, delimiter);) {
		if(clippedString.empty()) {
			continue;
		}

		result.emplace_back(clippedString);
	}

	return result;
}

// 文字列の先頭と末尾の空白を削除する
std::string strip(const std::string& str) {
	const auto first = str.find_first_not_of(" \t\n\r\f\v");
	if(first == std::string::npos) {
		return "";
	}
	const auto last = str.find_last_not_of(" \t\n\r\f\v");
	return str.substr(first, (last - first + 1));
}

std::vector<std::string> slice(const std::vector<std::string>& vec, const int start, const int end) {
	const int size = vec.size();
	const int endPos = end == 0x7fffffff ? size - 1 : end;
	if(endPos > size) {
		throw std::runtime_error("end index out of range");
	}

	const std::pair<int, int> index = [&]() -> std::pair<int, int> {
		int startIndex = start;
		int endIndex = end;

		// 負数を指定された場合は正数に修正(Pythonのlist型のhoge[-1:]のような振る舞いにする)
		if(start < 0) {
			startIndex = size + start;
		}
		if(end < 0) {
			startIndex = size + end;
		}

		// インデックスの範囲を調整
		if(startIndex < 0) {
			startIndex = 0;
		}
		if(endIndex > size) {
			endIndex = size;
		}
		if(startIndex > endIndex) {
			startIndex = endIndex;
		}

		return {startIndex, endIndex};
		}
	();

	// 指定された範囲の要素を切り抜く
	std::vector<std::string> result;
	for(int i = index.first; i < index.second; ++i) {
		result.emplace_back(vec[i]);
	}

	return result;
}