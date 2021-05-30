#include "encrypt.h"
#include "blockchain.h"



#include <stdio.h>

void encryptblock(halfblock_t * message,halfblock_t * encrypted, int size, halfblock_t * vector, halfblock_t * r_keys)
 {
			  printf("%u\r\n", vector[0]);
     encrypted[0]=message[0]^vector[0];
     encrypted[1]=message[1]^vector[1];
     encrypt(encrypted, encrypted, r_keys);
     for(int i = 1; i < size/2; i++) {
			  printf("%u\r\n", encrypted[2*i-2]);
        encrypted[2*i]=message[2*i]^encrypted[2*i-2];
        encrypted[2*i+1]=message[2*i+1]^encrypted[2*i-1];
        encrypt(encrypted + 2*i, encrypted + 2*i, r_keys);
     }
     vector[0]=encrypted[size-2]; vector[1]=encrypted[size-1];
 }

void decryptblock(halfblock_t * decrypted,halfblock_t * encrypted, int size, halfblock_t * vector, halfblock_t * r_keys)
{
     decrypt(decrypted, encrypted, r_keys);
     decrypted[0]^=vector[0];
     decrypted[1]^=vector[1];
     for(int i = 1; i < size/2; i++) {
         decrypt(decrypted+2*i, encrypted + 2*i, r_keys);
         decrypted[2*i]^=encrypted[2*i-2];
         decrypted[2*i+1]^=encrypted[2*i-1];
     }
     vector[0]=encrypted[size-2]; vector[1]=encrypted[size-1];
}
