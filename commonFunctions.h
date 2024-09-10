#pragma once
#include "stdint.h"
static bool isArrayZero(const uint8_t* ptr, int len){
    for (int i = 0; i < len; i++){
        if (ptr[i] != 0) return false;
    }
    return true;
}
static bool isZeroPubkey(const uint8_t* pubkey){
    return isArrayZero(pubkey, 32);
}