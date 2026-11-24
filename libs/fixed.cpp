#include "cipher_interface.h"
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>

using namespace std;

extern "C" int transform(const char* key, unsigned char* data, size_t len, bool encrypt) {
    if (!key || key[0] == '\0' || len == 0) return 0;
    string cipherKey(key);
    size_t blockSize = cipherKey.length();

    vector<size_t> permVector(blockSize);
    iota(permVector.begin(), permVector.end(), 0);
    stable_sort(permVector.begin(), permVector.end(), [&](size_t i, size_t j) {
        return (unsigned char)cipherKey[i] < (unsigned char)cipherKey[j];
    });
    if (encrypt) {
        for (size_t i = 0; i + blockSize <= len; i += blockSize) {
            vector<unsigned char> tempBlock(blockSize);
            for (size_t j = 0; j < blockSize; ++j) {
                tempBlock[j] = data[i + permVector[j]];
            }
            for (size_t j = 0; j < blockSize; ++j) {
                data[i + j] = tempBlock[j];
            }
        }
    } else {
        vector<size_t> invPermVector(blockSize);
        for (size_t i = 0; i < blockSize; ++i) {
            invPermVector[permVector[i]] = i;
        }
        for (size_t i = 0; i + blockSize <= len; i += blockSize) {
            vector<unsigned char> tempBlock(blockSize);
            for (size_t j = 0; j < blockSize; ++j) {
                tempBlock[j] = data[i + invPermVector[j]];
            }
            for (size_t j = 0; j < blockSize; ++j) {
                data[i + j] = tempBlock[j];
            }
        }
    }

    return 0;
}
