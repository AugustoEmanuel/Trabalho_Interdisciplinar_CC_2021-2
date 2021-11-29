#ifndef _AES_H_
#define _AES_H_

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


#define AES_BLOCKLEN 16
#define AES_KEYLEN 16
#define AES_keyExpSize 176

#define ENCRYPT_MODE	1
#define DECRYPT_MODE	0

struct AES_ctx
{
  unsigned char RoundKey[176];
};

void AES_Start(struct AES_ctx* ctx, unsigned char* buffer, int Mode);
void AES_LoadKey(struct AES_ctx* ctx, unsigned char* key);

#endif // _AES_H_
