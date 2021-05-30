#include "encrypt.h"

#if N == 5
const halfblock_t arr[8] = {707106788, 866025405, 559116991, 661437827, 357247820, 120784366, 587496123, 154786201};
#elif N == 6
const halfblock_t arr[8] = {70710678854716856240, 86602540520124747118, 55911699123217836004, 66143782701487634691, 357247820694701693, 1207843622113086, 58749612344741163, 154786278510901};
#endif

const uint8_t shift_arr[4] = {SHIFT_0, SHIFT_1, SHIFT_2, SHIFT_3}; //NEW

void gen_round_keys(const halfblock_t *key, halfblock_t *r_keys) {
    halfblock_t c[8];

    for(uint8_t i = 0; i < KEY_BLOCKS; i++)
        c[i] = *(key + i);

    for(uint8_t i = 0; i < 8 - KEY_BLOCKS; i++) {
        c[i + KEY_BLOCKS] = arr[i];
    }
    r_keys[0] = c[0];
    r_keys[1] = c[1];

    for(uint8_t i = 1; i <= ROUND_NUM; i++) {
        encrypt_round(r_keys + 2*i - 2, r_keys + 2*i, i ^ c[i % 6 + 2]);
    }

}

void encrypt(const halfblock_t *in, halfblock_t *out, halfblock_t *r_keys) {
    int i;
    halfblock_t tmp[2] = {in[0], in[1]};
    for(i = 0; i < ROUND_NUM - 1; i++) {
        encrypt_round(tmp, out, r_keys[2*i + 2]);
        tmp[0] = out[0];
        tmp[1] = out[1];
    }
    encrypt_last(tmp, out, r_keys[2*i + 2]);
}

void encrypt_round(const halfblock_t *in, halfblock_t *out, halfblock_t key) {
    out[1] = in[0];
    halfblock_t tmp = (in[0] + key);
    uint8_t shift = shift_F(tmp, in[0]);
    out[0] = (tmp >> shift) | (tmp << (HALF_SIZE - shift));
    out[0] ^= in[1];
}

void encrypt_last(const halfblock_t *in, halfblock_t *out, halfblock_t key) {
    out[0] = in[0];
    halfblock_t tmp = (in[0] + key);
    uint8_t shift = shift_F(tmp, in[0]);
    out[1] = (tmp >> shift) | (tmp << (HALF_SIZE - shift));
    out[1] ^= in[1];
}

uint8_t shift_F(halfblock_t LK, halfblock_t L) {
    uint8_t value = 0;
    uint8_t shift = 0;
    for(int i = 1; i <= N; i++) {
        value += ((LK >> ((1 << i) - 1)) & 0x01) << (N - i);
    }
    shift = (L >> (value - 1)) & 0x02;

    value = 0;
    for(int i = 1; i <= N; i++) {
        value += ((L >> ((1 << i) - 1)) & 0x01) << (N - i);
    }
    shift += (LK >> value) & 0x01;

    return shift_arr[shift]; //NN

    /*if(shift == 0) {
        return SHIFT_0;
    } else if(shift == 1) {
        return SHIFT_1;
    } else if(shift == 2) {
        return SHIFT_2;
    } else if(shift == 3) {
        return SHIFT_3;
    } else {
        //qDebug() << "Something went wrong!";
        return 0;
    }*/
}



void decrypt( halfblock_t *in, const halfblock_t *out, halfblock_t *r_keys)
{
    int i;
    halfblock_t tmp[2] = {out[0], out[1]};
    decrypt_first(in, tmp, r_keys[2*ROUND_NUM]);
    for(i = ROUND_NUM-2; i >=0 ; i--) {
        tmp[0] = in[0];
        tmp[1] = in[1];
        decrypt_round(in, tmp, r_keys[2*i + 2]);
    }
}


void decrypt_round(halfblock_t *in, const halfblock_t *out, halfblock_t key)
{
    in[0] = out[1];
    halfblock_t tmp = (in[0] + key);
    uint8_t shift = shift_F(tmp, in[0]);
    in[1] = (tmp >> shift) | (tmp << (HALF_SIZE - shift));
    in[1] ^= out[0];
}


void decrypt_first(halfblock_t *in, const halfblock_t *out, halfblock_t key)
{
    in[0] = out[0];
    halfblock_t tmp = (in[0] + key);
    uint8_t shift = shift_F(tmp, in[0]);
    in[1] = (tmp >> shift) | (tmp << (HALF_SIZE - shift));
    in[1] ^= out[1];
}
