#include <emmintrin.h>
#include <immintrin.h>

#include <cstdint>
#include <vector>

static const char table[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

std::vector<uint8_t> encode(std::vector<uint8_t> bytes) {
  auto encoded_string = std::vector<uint8_t>();

  uint64_t i = 0;
  while (bytes.size() - i >= 3) {
    encoded_string.push_back(table[bytes[i] >> 2]);
    encoded_string.push_back(
        table[((bytes[i] & 0b00000011) << 4) | (bytes[i + 1] >> 4)]);
    encoded_string.push_back(
        table[((bytes[i + 1] & 0b00001111) << 2) | (bytes[i + 2] >> 6)]);
    encoded_string.push_back(table[bytes[i + 2] & 0b00111111]);
    i += 3;
  }
  if (bytes.size() - i == 2) {
    encoded_string.push_back(table[bytes[i] >> 2]);
    encoded_string.push_back(
        table[((bytes[i] & 0b00000011) << 4) | (bytes[i + 1] >> 4)]);
    encoded_string.push_back(table[((bytes[i + 1] & 0b00001111) << 2) | 0]);
    encoded_string.push_back(61);
  } else if (bytes.size() - i == 1) {
    encoded_string.push_back(table[bytes[i] >> 2]);
    encoded_string.push_back(table[((bytes[i] & 0b00000011) << 4) | 0]);
    encoded_string.push_back(61);
    encoded_string.push_back(61);
  }

  return encoded_string;
}

// Minimum version of SSE 4.2
// TODO: Only compile it when we do support it. but come on, even my pentinium
// g2020 from my childhood supports sse4.2
std::vector<uint8_t> encode_but_SSE(std::vector<uint8_t> bytes) {
  // TODO: Later on test getting 15 bytes in and producing 20 bytes out

  auto encoded_string = std::vector<uint8_t>();
  encoded_string.resize(ceil(bytes.size() / 3) * 4);

  uint64_t i = 0;
  uint64_t j = 0;

  alignas(16) uint8_t result[16];

  while (bytes.size() - i >= 12) {
    auto a =
        _mm_set_epi8(bytes[i + 10], bytes[i + 11], bytes[i + 9], bytes[i + 10],
                     bytes[i + 7], bytes[i + 8], bytes[i + 6], bytes[i + 7],
                     bytes[i + 4], bytes[i + 5], bytes[i + 3], bytes[i + 4],
                     bytes[i + 1], bytes[i + 2], bytes[i + 0], bytes[i + 1]);

    _mm_store_si128((__m128i *)result, a);

    _mm_store_si128((__m128i *)result, _mm_srli_epi16(a, 2));

    encoded_string[j + 0] = table[result[1]];
    encoded_string[j + 4] = table[result[5]];
    encoded_string[j + 8] = table[result[9]];
    encoded_string[j + 12] = table[result[13]];

    /* encoded_string.erase() */

    _mm_storeu_si128((__m128i *)result,
                     _mm_and_si128(_mm_slli_epi16(a, 4),
                                   _mm_set1_epi16(0b0011111111111111)));

    encoded_string[j + 1] = table[result[1]];
    encoded_string[j + 5] = table[result[5]];
    encoded_string[j + 9] = table[result[9]];
    encoded_string[j + 13] = table[result[13]];

    _mm_storeu_si128((__m128i *)result,
                     _mm_and_si128(_mm_slli_epi16(a, 2),
                                   _mm_set1_epi16(0b0001111111111111)));

    encoded_string[j + 2] = table[result[3]];
    encoded_string[j + 6] = table[result[7]];
    encoded_string[j + 10] = table[result[11]];
    encoded_string[j + 14] = table[result[15]];

    _mm_storeu_si128((__m128i *)result,
                     _mm_and_si128(a, _mm_set1_epi16(0b0000000000111111)));

    encoded_string[j + 3] = table[result[2]];
    encoded_string[j + 7] = table[result[6]];
    encoded_string[j + 11] = table[result[10]];
    encoded_string[j + 15] = table[result[14]];
    i += 12;
    j += 16;
  }

  if (bytes.size() - i >= 6) {
    auto a = _mm_set_epi64x(
        0, ((uint64_t)bytes[i + 4] << 56) | ((uint64_t)bytes[i + 5] << 48) |
               ((uint64_t)bytes[i + 3] << 40) | ((uint64_t)bytes[i + 4] << 32) |
               ((uint64_t)bytes[i + 1] << 24) | ((uint64_t)bytes[i + 2] << 16) |
               ((uint64_t)bytes[i + 0] << 8) | ((uint64_t)bytes[i + 1]));

    _mm_storel_epi64((__m128i *)result, a);

    _mm_store_si128((__m128i *)result, _mm_srli_epi16(a, 2));
    encoded_string[j + 0] = table[result[1]];
    encoded_string[j + 4] = table[result[5]];
    _mm_storeu_si128((__m128i *)result,
                     _mm_and_si128(_mm_slli_epi16(a, 4),
                                   _mm_set1_epi16(0b0011111111111111)));
    encoded_string[j + 1] = table[result[1]];
    encoded_string[j + 5] = table[result[5]];

    _mm_storeu_si128((__m128i *)result,
                     _mm_and_si128(_mm_slli_epi16(a, 2),
                                   _mm_set1_epi16(0b0001111111111111)));
    encoded_string[j + 2] = table[result[3]];
    encoded_string[j + 6] = table[result[7]];

    _mm_storeu_si128((__m128i *)result,
                     _mm_and_si128(a, _mm_set1_epi16(0b0000000000111111)));

    encoded_string[j + 3] = table[result[2]];
    encoded_string[j + 7] = table[result[6]];
    i += 6;
    j += 8;
  }

  if (bytes.size() - i >= 3) {
    encoded_string.push_back(table[bytes[i] >> 2]);
    encoded_string.push_back(
        table[((bytes[i] & 0b00000011) << 4) | (bytes[i + 1] >> 4)]);
    encoded_string.push_back(
        table[((bytes[i + 1] & 0b00001111) << 2) | (bytes[i + 2] >> 6)]);
    encoded_string.push_back(table[bytes[i + 2] & 0b00111111]);
    i += 3;
  }

  if (bytes.size() - i == 2) {
    encoded_string.push_back(table[bytes[i] >> 2]);
    encoded_string.push_back(
        table[((bytes[i] & 0b00000011) << 4) | (bytes[i + 1] >> 4)]);
    encoded_string.push_back(table[((bytes[i + 1] & 0b00001111) << 2) | 0]);
    encoded_string.push_back(61);
  } else if (bytes.size() - i == 1) {
    encoded_string.push_back(table[bytes[i] >> 2]);
    encoded_string.push_back(table[((bytes[i] & 0b00000011) << 4) | 0]);
    encoded_string.push_back(61);
    encoded_string.push_back(61);
  }

  return encoded_string;
}

// Minimum version AVX2
// TODO: Only compile it when we do support it.
std::vector<uint8_t> encode_but_AVX2(std::vector<uint8_t> bytes) {
  // TODO: Later on test getting 15 bytes in and producing 20 bytes out

  auto encoded_string = std::vector<uint8_t>();
  encoded_string.resize(ceil(bytes.size() / 3) * 4);

  uint64_t i = 0;
  uint64_t j = 0;

  alignas(32) uint8_t result[32];
  while (bytes.size() - i >= 24) {
    auto a = _mm256_set_epi8(
        bytes[i + 22], bytes[i + 23], bytes[i + 21], bytes[i + 22],
        bytes[i + 19], bytes[i + 20], bytes[i + 18], bytes[i + 19],
        bytes[i + 16], bytes[i + 17], bytes[i + 15], bytes[i + 16],
        bytes[i + 13], bytes[i + 14], bytes[i + 12], bytes[i + 13],
        bytes[i + 10], bytes[i + 11], bytes[i + 9], bytes[i + 10], bytes[i + 7],
        bytes[i + 8], bytes[i + 6], bytes[i + 7], bytes[i + 4], bytes[i + 5],
        bytes[i + 3], bytes[i + 4], bytes[i + 1], bytes[i + 2], bytes[i + 0],
        bytes[i + 1]);

    _mm256_store_si256((__m256i *)result, a);
    _mm256_store_si256((__m256i *)result, _mm256_srli_epi16(a, 2));

    encoded_string[j + 0] = table[result[1]];
    encoded_string[j + 4] = table[result[5]];
    encoded_string[j + 8] = table[result[9]];
    encoded_string[j + 12] = table[result[13]];
    encoded_string[j + 16] = table[result[17]];
    encoded_string[j + 20] = table[result[21]];
    encoded_string[j + 24] = table[result[25]];
    encoded_string[j + 28] = table[result[29]];

    _mm256_storeu_si256(
        (__m256i *)result,
        _mm256_and_si256(_mm256_slli_epi16(a, 4),
                         _mm256_set1_epi16(0b0011111111111111)));

    encoded_string[j + 1] = table[result[1]];
    encoded_string[j + 5] = table[result[5]];
    encoded_string[j + 9] = table[result[9]];
    encoded_string[j + 13] = table[result[13]];
    encoded_string[j + 17] = table[result[17]];
    encoded_string[j + 21] = table[result[21]];
    encoded_string[j + 25] = table[result[25]];
    encoded_string[j + 29] = table[result[29]];

    _mm256_storeu_si256(
        (__m256i *)result,
        _mm256_and_si256(_mm256_slli_epi16(a, 2),
                         _mm256_set1_epi16(0b0001111111111111)));

    encoded_string[j + 2] = table[result[3]];
    encoded_string[j + 6] = table[result[7]];
    encoded_string[j + 10] = table[result[11]];
    encoded_string[j + 14] = table[result[15]];
    encoded_string[j + 18] = table[result[19]];
    encoded_string[j + 22] = table[result[23]];
    encoded_string[j + 26] = table[result[27]];
    encoded_string[j + 30] = table[result[31]];

    _mm256_storeu_si256(
        (__m256i *)result,
        _mm256_and_si256(a, _mm256_set1_epi16(0b0000000000111111)));

    encoded_string[j + 3] = table[result[2]];
    encoded_string[j + 7] = table[result[6]];
    encoded_string[j + 11] = table[result[10]];
    encoded_string[j + 15] = table[result[14]];
    encoded_string[j + 19] = table[result[18]];
    encoded_string[j + 23] = table[result[22]];
    encoded_string[j + 27] = table[result[26]];
    encoded_string[j + 31] = table[result[30]];
    i += 24;
    j += 32;
  }

  if (bytes.size() - i >= 12) {
    auto a =
        _mm_set_epi8(bytes[i + 10], bytes[i + 11], bytes[i + 9], bytes[i + 10],
                     bytes[i + 7], bytes[i + 8], bytes[i + 6], bytes[i + 7],
                     bytes[i + 4], bytes[i + 5], bytes[i + 3], bytes[i + 4],
                     bytes[i + 1], bytes[i + 2], bytes[i + 0], bytes[i + 1]);

    _mm_store_si128((__m128i *)result, a);

    _mm_store_si128((__m128i *)result, _mm_srli_epi16(a, 2));

    encoded_string[j + 0] = table[result[1]];
    encoded_string[j + 4] = table[result[5]];
    encoded_string[j + 8] = table[result[9]];
    encoded_string[j + 12] = table[result[13]];

    /* encoded_string.erase() */

    _mm_storeu_si128((__m128i *)result,
                     _mm_and_si128(_mm_slli_epi16(a, 4),
                                   _mm_set1_epi16(0b0011111111111111)));

    encoded_string[j + 1] = table[result[1]];
    encoded_string[j + 5] = table[result[5]];
    encoded_string[j + 9] = table[result[9]];
    encoded_string[j + 13] = table[result[13]];

    _mm_storeu_si128((__m128i *)result,
                     _mm_and_si128(_mm_slli_epi16(a, 2),
                                   _mm_set1_epi16(0b0001111111111111)));

    encoded_string[j + 2] = table[result[3]];
    encoded_string[j + 6] = table[result[7]];
    encoded_string[j + 10] = table[result[11]];
    encoded_string[j + 14] = table[result[15]];

    _mm_storeu_si128((__m128i *)result,
                     _mm_and_si128(a, _mm_set1_epi16(0b0000000000111111)));

    encoded_string[j + 3] = table[result[2]];
    encoded_string[j + 7] = table[result[6]];
    encoded_string[j + 11] = table[result[10]];
    encoded_string[j + 15] = table[result[14]];
    i += 12;
    j += 16;
  }

  if (bytes.size() - i >= 6) {
    auto a = _mm_set_epi64x(
        0, ((uint64_t)bytes[i + 4] << 56) | ((uint64_t)bytes[i + 5] << 48) |
               ((uint64_t)bytes[i + 3] << 40) | ((uint64_t)bytes[i + 4] << 32) |
               ((uint64_t)bytes[i + 1] << 24) | ((uint64_t)bytes[i + 2] << 16) |
               ((uint64_t)bytes[i + 0] << 8) | ((uint64_t)bytes[i + 1]));

    _mm_storel_epi64((__m128i *)result, a);

    _mm_store_si128((__m128i *)result, _mm_srli_epi16(a, 2));
    encoded_string[j + 0] = table[result[1]];
    encoded_string[j + 4] = table[result[5]];
    _mm_storeu_si128((__m128i *)result,
                     _mm_and_si128(_mm_slli_epi16(a, 4),
                                   _mm_set1_epi16(0b0011111111111111)));
    encoded_string[j + 1] = table[result[1]];
    encoded_string[j + 5] = table[result[5]];

    _mm_storeu_si128((__m128i *)result,
                     _mm_and_si128(_mm_slli_epi16(a, 2),
                                   _mm_set1_epi16(0b0001111111111111)));
    encoded_string[j + 2] = table[result[3]];
    encoded_string[j + 6] = table[result[7]];

    _mm_storeu_si128((__m128i *)result,
                     _mm_and_si128(a, _mm_set1_epi16(0b0000000000111111)));

    encoded_string[j + 3] = table[result[2]];
    encoded_string[j + 7] = table[result[6]];
    i += 6;
    j += 8;
  }

  if (bytes.size() - i >= 3) {
    encoded_string.push_back(table[bytes[i] >> 2]);
    encoded_string.push_back(
        table[((bytes[i] & 0b00000011) << 4) | (bytes[i + 1] >> 4)]);
    encoded_string.push_back(
        table[((bytes[i + 1] & 0b00001111) << 2) | (bytes[i + 2] >> 6)]);
    encoded_string.push_back(table[bytes[i + 2] & 0b00111111]);
    i += 3;
  }

  if (bytes.size() - i == 2) {
    encoded_string.push_back(table[bytes[i] >> 2]);
    encoded_string.push_back(
        table[((bytes[i] & 0b00000011) << 4) | (bytes[i + 1] >> 4)]);
    encoded_string.push_back(table[((bytes[i + 1] & 0b00001111) << 2) | 0]);
    encoded_string.push_back(61);
  } else if (bytes.size() - i == 1) {
    encoded_string.push_back(table[bytes[i] >> 2]);
    encoded_string.push_back(table[((bytes[i] & 0b00000011) << 4) | 0]);
    encoded_string.push_back(61);
    encoded_string.push_back(61);
  }

  return encoded_string;
}

int main(int argc, char *argv[]) {
  return 0;
}
