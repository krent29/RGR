#include "cipher_interface.h"
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>

using namespace std;

extern "C" int transform(const char* key, unsigned char* data, size_t len, bool encrypt) {
    if (!key || key[0] == '\0' || len == 0) return 0;
    string cipherKey(key);
    size_t numCols = cipherKey.length();
    size_t numRows = (len + numCols - 1) / numCols;

    vector<size_t> colOrder(numCols);
    iota(colOrder.begin(), colOrder.end(), 0);
    stable_sort(colOrder.begin(), colOrder.end(), [&](size_t i, size_t j) {
        return (unsigned char)cipherKey[i] < (unsigned char)cipherKey[j];
    });

    vector<unsigned char> result(len);
    if (encrypt) {
        size_t k = 0;
        for (size_t colIdx : colOrder) {
            for (size_t row = 0; row < numRows; ++row) {
                size_t dataIdx = row * numCols + colIdx;
                if (dataIdx < len) {
                    result[k++] = data[dataIdx];
                }
            }
        }
    } else {
        vector<size_t> colLengths(numCols, len / numCols);
        for (size_t i = 0; i < len % numCols; ++i) {
            colLengths[i]++;
        }
        vector<vector<unsigned char>> columns(numCols);
        size_t k = 0;
        for (size_t colIdx : colOrder) {
            for (size_t row = 0; row < colLengths[colIdx]; ++row) {
                columns[colIdx].push_back(data[k++]);
            }
        }
        size_t r = 0;
        for (size_t row = 0; row < numRows; ++row) {
            for (size_t col = 0; col < numCols; ++col) {
                if (row < columns[col].size()) {
                    result[r++] = columns[col][row];
                }
            }
        }
    }
    copy(result.begin(), result.end(), data);
    return 0;
}
