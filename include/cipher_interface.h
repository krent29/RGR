#ifndef CIPHER_INTERFACE_H
#define CIPHER_INTERFACE_H
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif
int transform(const char* key, unsigned char* data, size_t len, bool encrypt);

#ifdef __cplusplus
}
#endif
#endif
