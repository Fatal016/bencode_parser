#ifndef H_BENCODE_UTILS
#define H_BENCODE_UTILS

#include "bencode.h"

/* Free Bencode Module */
uint8_t fbm(struct bm*);

/* Bencode Module Init */
uint8_t bm_init(struct bm*);

/* Verify Integer */
int vi(char*, uint32_t*);

/* Return Value */
uint8_t rv(struct bm*, FILE*);

uint8_t gc(int32_t*, FILE*);

//int printBencode(struct bencode_module *bencode);

#endif
