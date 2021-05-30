#ifndef ENCRYPT_H
#define ENCRYPT_H

#include "stdint.h"
#include "math.h"

#define KEY_SIZE            128
//128, 192 or 256

#define N                   5
#if N == 5
    #define BLOCK_SIZE      64
    #define HALF_SIZE       32
    #define halfblock_t     uint32_t
    #define ROUND_NUM       24
    #define SHIFT_0         11
    #define SHIFT_1         14
    #define SHIFT_2         10
    #define SHIFT_3         19
#elif N == 6
    #define BLOCK_SIZE      128
    #define HALF_SIZE       64
    #define halfblock_t     uint64_t
    #define ROUND_NUM       28

    #define SHIFT_0         37
    #define SHIFT_1         34
    #define SHIFT_2         38
    #define SHIFT_3         29
#endif

#define KEY_BLOCKS          KEY_SIZE/HALF_SIZE

//extern const uint8_t shift_arr[4];  //NEW
uint8_t shift_F(halfblock_t LK, halfblock_t L);

void gen_round_keys(const halfblock_t *key, halfblock_t *r_keys);
void encrypt(const halfblock_t *in, halfblock_t *out, halfblock_t *r_keys);
void decrypt( halfblock_t *in, const halfblock_t *out, halfblock_t *r_keys);

void encrypt_round(const halfblock_t *in, halfblock_t *out, halfblock_t key);
void encrypt_last(const halfblock_t *in, halfblock_t *out, halfblock_t key);
void decrypt_round(halfblock_t *in, const halfblock_t *out, halfblock_t key);
void decrypt_first(halfblock_t *in, const halfblock_t *out, halfblock_t key);

#endif // ENCRYPT_H
