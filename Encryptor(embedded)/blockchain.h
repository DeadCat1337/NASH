#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include "encrypt.h"

void encryptblock(halfblock_t * message,halfblock_t * encrypted, int size, halfblock_t * vector, halfblock_t * r_keys);
void decryptblock(halfblock_t * decrypted,halfblock_t * encrypted, int size, halfblock_t * vector, halfblock_t * r_keys);

#endif // BLOCKCHAIN_H
