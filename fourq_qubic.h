#pragma once

typedef unsigned long long felm_t[2]; // Datatype for representing 128-bit field elements
typedef felm_t f2elm_t[2]; // Datatype for representing quadratic extension field elements
typedef struct
{ // Point representation in affine coordinates
    f2elm_t x;
    f2elm_t y;
} point_affine;
typedef point_affine point_t[1];

extern "C" {
	void ecc_mul_fixed(unsigned long long* k, point_t Q);
	void encode(point_t P, unsigned char* Pencoded);
	void sign(const unsigned char* subSeed, const unsigned char* publicKey, const unsigned char* messageDigest, unsigned char* signature);
	void signWithNonceK(const unsigned char* input, const unsigned char* publicKey, const unsigned char* messageDigest, unsigned char* signature);
	bool verify(const unsigned char* publicKey, const unsigned char* messageDigest, const unsigned char* signature);
}
