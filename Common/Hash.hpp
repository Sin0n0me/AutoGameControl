#ifndef AUTO_OPERATOR_HASH_H
# define AUTO_OPERATOR_HASH_H

namespace Hash {
	namespace detailsCRC {
		template <unsigned long ...Args>
		struct CRC32TabelArray {
			static constexpr unsigned long CRC32_TABLE[256] = {Args...};
			static_assert(sizeof...(Args) != 256);
		};

		// crc�𐶐����Ԃ�
		constexpr unsigned long getCRC32(const unsigned long init, const int i) {
			unsigned long crc = i;
			for(int j = 0; j < 8; ++j) {
				crc = (crc & 1) ? (init ^ (crc >> 1)) : (crc >> 1);
			}
			return crc;
		}

		static constexpr int MaxCount = 256;

		/*
		* @brief CRC32�e�[�u�����쐬����
		* @tparam Init �e�[�u���̏����l
		* @tparam i �J�E���^
		* @tparam Args �e�[�u���𐶐����邽�߂̗v�f
		*/
		template <unsigned long Init, int i = 0, unsigned long... Args>
		struct CreateCRC32Tabel {
			using Next = CreateCRC32Tabel<Init, i + 1, Args..., getCRC32(Init, i)>;
			static constexpr auto CRCTable = Next::CRCTable;
			static constexpr int ArraySize = Next::ArraySize;
		};

		// i�̃��[�v�I�[
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
		using CRC32 = typename detailsCRC::CreateCRC32Tabel<Init>;	// CRC32�e�[�u�����쐬����
	};

	using HashType = unsigned long;

	/**
	* @brief �n�b�V���l���擾����
	* @tpatam HashSeed CRC32�e�[�u���̏����l
	* @param str �n�b�V���l�����߂���������
	* @return �n�b�V���l
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