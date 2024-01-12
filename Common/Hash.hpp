#ifndef AUTO_OPERATOR_HASH_H
# define AUTO_OPERATOR_HASH_H

namespace Hash {
	namespace detailsCRC {
		template <unsigned long ...Args>
		struct CRC32TabelArray {
			static constexpr unsigned long CRC32_TABLE[256] = {Args...};
			static_assert(sizeof...(Args) != 256);
		};

		// crcを生成し返す
		constexpr unsigned long getCRC32(const unsigned long init, const int i) {
			unsigned long crc = i;
			for(int j = 0; j < 8; ++j) {
				crc = (crc & 1) ? (init ^ (crc >> 1)) : (crc >> 1);
			}
			return crc;
		}

		static constexpr int MaxCount = 256;

		/*
		* @brief CRC32テーブルを作成する
		* @tparam Init テーブルの初期値
		* @tparam i カウンタ
		* @tparam Args テーブルを生成するための要素
		*/
		template <unsigned long Init, int i = 0, unsigned long... Args>
		struct CreateCRC32Tabel {
			using Next = CreateCRC32Tabel<Init, i + 1, Args..., getCRC32(Init, i)>;
			static constexpr auto CRCTable = Next::CRCTable;
			static constexpr int ArraySize = Next::ArraySize;
		};

		// iのループ終端
		template <unsigned long Init, unsigned long... Args>
		struct CreateCRC32Tabel<Init, MaxCount, Args...> {
			static constexpr unsigned long CRCTable[] = {Args...};
			static constexpr int ArraySize = sizeof...(Args);
		};

		template <unsigned long Init, unsigned long... Args>
		constexpr unsigned long CreateCRC32Tabel<Init, MaxCount, Args...>::CRCTable[];
	}

	template <unsigned long Init>
	struct CRCTabels {
		using CRC32 = typename detailsCRC::CreateCRC32Tabel<Init>;	// CRC32テーブルを作成する
	};

	using HashType = unsigned long;

	/**
	* @brief ハッシュ値を取得する
	* @tpatam HashSeed CRC32テーブルの初期値
	* @param str ハッシュ値を求めたい文字列
	* @return ハッシュ値
	*/
	template <int HashSeed = 0xC0FFEE>
	constexpr HashType getHash(const char* str) {
		constexpr int CharBit = sizeof(char) * 8;
		HashType crc32 = 0xffffffff;
		for(int i = 0; str[i] != '\0'; ++i) {
			crc32 = CRCTabels<HashSeed>::CRC32::CRCTable[(crc32 ^ str[i]) & 0xff] ^ (crc32 >> CharBit);
		}
		return crc32 ^ 0xffffffff;
	}
}

#endif