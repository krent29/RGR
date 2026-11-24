#include "cipher_interface.h"
#include <vector>
#include <numeric>
#include <algorithm>
#include <random>
#include <string>

using namespace std;

extern "C" int transform(const char* key, unsigned char* data, size_t len, bool encrypt) {
    if (!key || len == 0) return 0;
    vector<unsigned char> pBox(256);
    iota(pBox.begin(), pBox.end(), 0);

    string cipherKey(key);
    vector<int> seedData;
    if (cipherKey.empty()) {
        seedData.push_back(0);
    } else {
        for (char c : cipherKey) {
            seedData.push_back((unsigned char)c);
        }
    }
    seed_seq seedSequence(seedData.begin(), seedData.end());
    mt19937 generator(seedSequence);
    shuffle(pBox.begin(), pBox.end(), generator);
    if (encrypt) {
        vector<unsigned char> lookupTable(256);
        for (int i = 0; i < 256; ++i) {
            lookupTable[pBox[i]] = (unsigned char)i;
        }
        for (size_t i = 0; i < len; ++i) {
            data[i] = lookupTable[data[i]];
        }
    } else {
        for (size_t i = 0; i < len; ++i) {
            data[i] = pBox[data[i]];
        }
    }

    return 0;
}
