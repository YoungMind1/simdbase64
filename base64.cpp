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

int main(int argc, char *argv[]) {
  return 0;
}
