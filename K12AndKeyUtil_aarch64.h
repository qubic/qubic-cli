/***********************************************************************************
* FourQlib: a high-performance crypto library based on the elliptic curve FourQ
*
*    Copyright (c) Microsoft Corporation. All rights reserved.
*
* Abstract: modular arithmetic and other low-level operations for x64 platforms
************************************************************************************/

#ifndef __FP_ARM64_H__
#define __FP_ARM64_H__

static const uint64_t PARAMETER_d[4]       = { 0x0000000000000142, 0x00000000000000E4, 0xB3821488F1FC0C8D, 0x5E472F846657E0FC };
static const uint64_t GENERATOR_x[4]       = { 0x286592AD7B3833AA, 0x1A3472237C2FB305, 0x96869FB360AC77F6, 0x1E1F553F2878AA9C };
static const uint64_t GENERATOR_y[4]       = { 0xB924A2462BCBB287, 0x0E3FEE9BA120785A, 0x49A7C344844C8B5C, 0x6E1C4AF8630E0242 };
static const uint64_t curve_order[4]       = { 0x2FB2540EC7768CE7, 0xDFBD004DFE0F7999, 0xF05397829CBC14E5, 0x0029CBC14E5E0A72 };
static const uint64_t Montgomery_Rprime[4] = { 0xC81DB8795FF3D621, 0x173EA5AAEA6B387D, 0x3D01B7C72136F61C, 0x0006A5F16AC8F9D3 };
static const uint64_t Montgomery_rprime[4] = { 0xE12FE5F079BC3929, 0xD75E78B8D1FCDCF3, 0xBCE409ED76B5DB21, 0xF32702FDAFC1C074 };

typedef unsigned uint128_t __attribute__((mode(TI)));
#define TARGET TARGET_ARM64
#define RADIX           64
typedef uint64_t        digit_t;      // Unsigned 64-bit digit
typedef int64_t         sdigit_t;     // Signed 64-bit digit
#define NWORDS_FIELD    2
#define NWORDS_ORDER    4
typedef unsigned long long felm_t[2];
const uint128_t prime1271 = ((uint128_t)1 << 127) - 1;
#define mask63 0x7FFFFFFFFFFFFFFF

const digit_t mask_7fff = (digit_t)(-1) >> 1;
const digit_t prime1271_0 = (digit_t)(-1);
#define prime1271_1 mask_7fff

typedef digit_t felm_t[NWORDS_FIELD];                  // Datatype for representing 128-bit field elements
typedef felm_t f2elm_t[2];                             // Datatype for representing quadratic extension field elements

typedef struct { f2elm_t x; f2elm_t y; } point_affine; // Point representation in affine coordinates.
typedef point_affine point_t[1];

void digit_x_digit(digit_t a, digit_t b, digit_t* c)
{ // Digit multiplication, digit * digit -> 2-digit result
    register digit_t al, ah, bl, bh, temp;
    digit_t albl, albh, ahbl, ahbh, res1, res2, res3, carry;
    digit_t mask_low = (digit_t)(-1) >> (sizeof(digit_t)*4), mask_high = (digit_t)(-1) << (sizeof(digit_t)*4);

    al = a & mask_low;                        // Low part
    ah = a >> (sizeof(digit_t) * 4);          // High part
    bl = b & mask_low;
    bh = b >> (sizeof(digit_t) * 4);

    albl = al*bl;
    albh = al*bh;
    ahbl = ah*bl;
    ahbh = ah*bh;
    c[0] = albl & mask_low;                   // C00

    res1 = albl >> (sizeof(digit_t) * 4);
    res2 = ahbl & mask_low;
    res3 = albh & mask_low;
    temp = res1 + res2 + res3;
    carry = temp >> (sizeof(digit_t) * 4);
    c[0] ^= temp << (sizeof(digit_t) * 4);    // C01

    res1 = ahbl >> (sizeof(digit_t) * 4);
    res2 = albh >> (sizeof(digit_t) * 4);
    res3 = ahbh & mask_low;
    temp = res1 + res2 + res3 + carry;
    c[1] = temp & mask_low;                   // C10
    carry = temp & mask_high;
    c[1] ^= (ahbh & mask_high) + carry;       // C11
}

static __inline unsigned int is_digit_nonzero_ct(digit_t x)
{ // Is x != 0?
    return (unsigned int)((x | (0-x)) >> (RADIX-1));
}

static __inline unsigned int is_digit_zero_ct(digit_t x)
{ // Is x = 0?
    return (unsigned int)(1 ^ is_digit_nonzero_ct(x));
}

static __inline unsigned int is_digit_lessthan_ct(digit_t x, digit_t y)
{ // Is x < y?
    return (unsigned int)((x ^ ((x ^ y) | ((x - y) ^ y))) >> (RADIX-1));
}

// Digit multiplication
#define MUL(multiplier, multiplicand, hi, lo)                                                     \
    digit_x_digit((multiplier), (multiplicand), &(lo));

// Digit addition with carry
#define ADDC(carryIn, addend1, addend2, carryOut, sumOut)                                         \
    { digit_t tempReg = (addend1) + (digit_t)(carryIn);                                           \
    (sumOut) = (addend2) + tempReg;                                                               \
    (carryOut) = (is_digit_lessthan_ct(tempReg, (digit_t)(carryIn)) | is_digit_lessthan_ct((sumOut), tempReg)); }

// Digit subtraction with borrow
#define SUBC(borrowIn, minuend, subtrahend, borrowOut, differenceOut)                             \
    { digit_t tempReg = (minuend) - (subtrahend);                                                 \
    unsigned int borrowReg = (is_digit_lessthan_ct((minuend), (subtrahend)) | ((borrowIn) & is_digit_zero_ct(tempReg)));  \
    (differenceOut) = tempReg - (digit_t)(borrowIn);                                              \
    (borrowOut) = borrowReg; }

// Shift right with flexible datatype
#define SHIFTR(highIn, lowIn, shift, shiftOut, DigitSize)                                         \
    (shiftOut) = ((lowIn) >> (shift)) ^ ((highIn) << (DigitSize - (shift)));

// shift left with flexible datatype
#define SHIFTL(highIn, lowIn, shift, shiftOut, DigitSize)                                         \
    (shiftOut) = ((highIn) << (shift)) ^ ((lowIn) >> (DigitSize - (shift)));

// 64x64-bit multiplication
#define MUL128(multiplier, multiplicand, product)                                                 \
    mp_mul((digit_t*)&(multiplier), (digit_t*)&(multiplicand), (digit_t*)&(product), NWORDS_FIELD/2);

// 128-bit addition, inputs < 2^127
#define ADD128(addend1, addend2, addition)                                                        \
    mp_add((digit_t*)(addend1), (digit_t*)(addend2), (digit_t*)(addition), NWORDS_FIELD);

// 128-bit addition with output carry
#define ADC128(addend1, addend2, carry, addition)                                                 \
    (carry) = mp_add((digit_t*)(addend1), (digit_t*)(addend2), (digit_t*)(addition), NWORDS_FIELD);


// For C++
#ifdef __cplusplus
extern "C" {
#endif

void mod1271(felm_t a)
{ // Modular correction, a = a mod (2^127-1)
    uint128_t* r = (uint128_t*)&a[0];

    *r = *r - prime1271;
    *r = *r + (((uint128_t)0 - (*r >> 127)) & prime1271);
}


__inline void fpcopy1271(felm_t a, felm_t c)
{ // Copy of a field element, c = a
    c[0] = a[0];
    c[1] = a[1];
}


static __inline void fpzero1271(felm_t a)
{ // Zeroing a field element, a = 0
    a[0] = 0;
    a[1] = 0;
}


__inline void fpadd1271(felm_t a, felm_t b, felm_t c)
{ // Field addition, c = a+b mod (2^127-1)
    uint128_t* r = (uint128_t*)&a[0];
    uint128_t* s = (uint128_t*)&b[0];
    uint128_t* t = (uint128_t*)&c[0];

    *t = *r + *s;
    *t += (*t >> 127);
    *t &= prime1271;
}


__inline void fpsub1271(felm_t a, felm_t b, felm_t c)
{ // Field subtraction, c = a-b mod (2^127-1)
    uint128_t* r = (uint128_t*)&a[0];
    uint128_t* s = (uint128_t*)&b[0];
    uint128_t* t = (uint128_t*)&c[0];

    *t = *r - *s;
    *t -= (*t >> 127);
    *t &= prime1271;
}


void fpneg1271(felm_t a)
{ // Field negation, a = -a mod (2^127-1)
    uint128_t* r = (uint128_t*)&a[0];

    *r = prime1271 - *r;
}


__inline void fpmul1271(felm_t a, felm_t b, felm_t c)
{ // Field multiplication, c = a*b mod (2^127-1)
    uint128_t tt1, tt2, tt3 = {0};

    tt1 = (uint128_t)a[0]*b[0];
    tt2 = (uint128_t)a[0]*b[1] + (uint128_t)a[1]*b[0] + (uint64_t)(tt1 >> 64);
    tt3 = (uint128_t)a[1]*(b[1]*2) + ((uint128_t)tt2 >> 63);
    tt1 = (uint64_t)tt1 | ((uint128_t)((uint64_t)tt2 & mask63) << 64);
    tt1 += tt3;
    tt1 = (tt1 >> 127) + (tt1 & prime1271);
    c[0] = (uint64_t)tt1;
    c[1] = (uint64_t)(tt1 >> 64);
}


void fpsqr1271(felm_t a, felm_t c)
{ // Field squaring, c = a^2 mod (2^127-1)
    uint128_t tt1, tt2, tt3 = {0};

    tt1 = (uint128_t)a[0]*a[0];
    tt2 = (uint128_t)a[0]*(a[1]*2) + (uint64_t)(tt1 >> 64);
    tt3 = (uint128_t)a[1]*(a[1]*2) + ((uint128_t)tt2 >> 63);
    tt1 = (uint64_t)tt1 | ((uint128_t)((uint64_t)tt2 & mask63) << 64);
    tt1 += tt3;
    tt1 = (tt1 >> 127) + (tt1 & prime1271);
    c[0] = (uint64_t)tt1;
    c[1] = (uint64_t)(tt1 >> 64);
}


__inline void fpexp1251(felm_t a, felm_t af)
{ // Exponentiation over GF(p), af = a^(125-1)
    int i;
    felm_t t1, t2, t3, t4, t5;

    fpsqr1271(a, t2);
    fpmul1271(a, t2, t2);
    fpsqr1271(t2, t3);
    fpsqr1271(t3, t3);
    fpmul1271(t2, t3, t3);
    fpsqr1271(t3, t4);
    fpsqr1271(t4, t4);
    fpsqr1271(t4, t4);
    fpsqr1271(t4, t4);
    fpmul1271(t3, t4, t4);
    fpsqr1271(t4, t5);
    for (i=0; i<7; i++) fpsqr1271(t5, t5);
    fpmul1271(t4, t5, t5);
    fpsqr1271(t5, t2);
    for (i=0; i<15; i++) fpsqr1271(t2, t2);
    fpmul1271(t5, t2, t2);
    fpsqr1271(t2, t1);
    for (i=0; i<31; i++) fpsqr1271(t1, t1);
    fpmul1271(t2, t1, t1);
    for (i=0; i<32; i++) fpsqr1271(t1, t1);
    fpmul1271(t1, t2, t1);
    for (i=0; i<16; i++) fpsqr1271(t1, t1);
    fpmul1271(t5, t1, t1);
    for (i=0; i<8; i++) fpsqr1271(t1, t1);
    fpmul1271(t4, t1, t1);
    for (i=0; i<4; i++) fpsqr1271(t1, t1);
    fpmul1271(t3, t1, t1);
    fpsqr1271(t1, t1);
    fpmul1271(a, t1, af);
}


void fpinv1271(felm_t a)
{ // Field inversion, af = a^-1 = a^(p-2) mod p
    // Hardcoded for p = 2^127-1
    felm_t t;

    fpexp1251(a, t);
    fpsqr1271(t, t);
    fpsqr1271(t, t);
    fpmul1271(a, t, a);
}


static __inline void multiply(const digit_t* a, const digit_t* b, digit_t* c)
{ // Schoolbook multiprecision multiply, c = a*b
    unsigned int i, j;
    digit_t u, v, UV[2];
    unsigned char carry = 0;

    for (i = 0; i < (2*NWORDS_ORDER); i++) c[i] = 0;

    for (i = 0; i < NWORDS_ORDER; i++) {
        u = 0;
        for (j = 0; j < NWORDS_ORDER; j++) {
            MUL(a[i], b[j], UV+1, UV[0]);
            ADDC(0, UV[0], u, carry, v);
            u = UV[1] + carry;
            ADDC(0, c[i+j], v, carry, v);
            u = u + carry;
            c[i+j] = v;
        }
        c[NWORDS_ORDER+i] = u;
    }
}


static __inline unsigned char add(const digit_t* a, const digit_t* b, digit_t* c, const unsigned int nwords)
{ // Multiprecision addition, c = a+b. Returns the carry bit
    unsigned int i;
    unsigned char carry = 0;

    for (i = 0; i < nwords; i++) {
        ADDC(carry, a[i], b[i], carry, c[i]);
    }

    return carry;
}


unsigned char subtract(const digit_t* a, const digit_t* b, digit_t* c, const unsigned int nwords)
{ // Multiprecision subtraction, c = a-b. Returns the borrow bit
    unsigned int i;
    unsigned char borrow = 0;

    for (i = 0; i < nwords; i++) {
        SUBC(borrow, a[i], b[i], borrow, c[i]);
    }

    return borrow;
}


void subtract_mod_order(const digit_t* a, const digit_t* b, digit_t* c)
{ // Subtraction modulo the curve order, c = a-b mod order
    digit_t mask, carry = 0;
    digit_t* order = (digit_t*)curve_order;
    unsigned int i, bout;

    bout = subtract(a, b, c, NWORDS_ORDER);            // (bout, c) = a - b
    mask = 0 - (digit_t)bout;                          // if bout = 0 then mask = 0x00..0, else if bout = 1 then mask = 0xFF..F

    for (i = 0; i < NWORDS_ORDER; i++) {               // c = c + (mask & order)
        ADDC(carry, c[i], mask & order[i], carry, c[i]);
    }
}


void add_mod_order(const digit_t* a, const digit_t* b, digit_t* c)
{ // Addition modulo the curve order, c = a+b mod order

    add(a, b, c, NWORDS_ORDER);                        // c = a + b
    subtract_mod_order(c, (digit_t*)&curve_order, c);  // if c >= order then c = c - order
}


void Montgomery_multiply_mod_order(const digit_t* ma, const digit_t* mb, digit_t* mc)
{ // 256-bit Montgomery multiplication modulo the curve order, mc = ma*mb*r' mod order, where ma,mb,mc in [0, order-1]
    // ma, mb and mc are assumed to be in Montgomery representation
    // The Montgomery constant r' = -r^(-1) mod 2^(log_2(r)) is the global value "Montgomery_rprime", where r is the order
    unsigned int i;
    digit_t mask, P[2*NWORDS_ORDER], Q[2*NWORDS_ORDER], temp[2*NWORDS_ORDER];
    digit_t* order = (digit_t*)curve_order;
    unsigned char cout = 0, bout = 0;

    multiply(ma, mb, P);                               // P = ma * mb
    multiply(P, (digit_t*)&Montgomery_rprime, Q);      // Q = P * r' mod 2^(log_2(r))
    multiply(Q, (digit_t*)&curve_order, temp);         // temp = Q * r
    cout = add(P, temp, temp, 2*NWORDS_ORDER);         // (cout, temp) = P + Q * r

    for (i = 0; i < NWORDS_ORDER; i++) {               // (cout, mc) = (P + Q * r)/2^(log_2(r))
        mc[i] = temp[NWORDS_ORDER + i];
    }

    // Final, constant-time subtraction
    bout = subtract(mc, (digit_t*)&curve_order, mc, NWORDS_ORDER);    // (cout, mc) = (cout, mc) - r
    mask = (digit_t)(cout - bout);                     // if (cout, mc) >= 0 then mask = 0x00..0, else if (cout, mc) < 0 then mask = 0xFF..F

    for (i = 0; i < NWORDS_ORDER; i++) {               // temp = mask & r
        temp[i] = (order[i] & mask);
    }
    add(mc, temp, mc, NWORDS_ORDER);                   //  mc = mc + (mask & r)

    return;
}


void modulo_order(digit_t* a, digit_t* c)
{ // Reduction modulo the order using Montgomery arithmetic
    // ma = a*Montgomery_Rprime mod r, where a,ma in [0, r-1], a,ma,r < 2^256
    // c = ma*1*Montgomery_Rprime^(-1) mod r, where ma,c in [0, r-1], ma,c,r < 2^256
    digit_t ma[NWORDS_ORDER], one[NWORDS_ORDER] = {0};

    one[0] = 1;
    Montgomery_multiply_mod_order(a, (digit_t*)&Montgomery_Rprime, ma);
    Montgomery_multiply_mod_order(ma, one, c);
}


void conversion_to_odd(digit_t* k, digit_t* k_odd)
{// Convert scalar to odd if even using the prime subgroup order r
    digit_t i, mask;
    digit_t* order = (digit_t*)curve_order;
    unsigned char carry = 0;

    mask = ~(0 - (k[0] & 1));

    for (i = 0; i < NWORDS_ORDER; i++) {  // If (k is odd) then k_odd = k else k_odd = k + r
        ADDC(carry, order[i] & mask, k[i], carry, k_odd[i]);
    }
}


void fpdiv1271(felm_t a)
{ // Field division by two, c = a/2 mod p
    digit_t mask, temp[2];
    unsigned char carry;

    mask = (0 - (1 & a[0]));
    ADDC(0,     a[0], mask, carry, temp[0]);
    ADDC(carry, a[1], (mask >> 1), carry, temp[1]);
    SHIFTR(temp[1], temp[0], 1, a[0], RADIX);
    a[1] = (temp[1] >> 1);
}


void fp2div1271(f2elm_t a)
{ // GF(p^2) division by two c = a/2 mod p
    digit_t mask, temp[2];
    unsigned char carry;

    mask = (0 - (1 & a[0][0]));
    ADDC(0,     a[0][0], mask, carry, temp[0]);
    ADDC(carry, a[0][1], (mask >> 1), carry, temp[1]);
    SHIFTR(temp[1], temp[0], 1, a[0][0], RADIX);
    a[0][1] = (temp[1] >> 1);

    mask = (0 - (1 & a[1][0]));
    ADDC(0,     a[1][0], mask, carry, temp[0]);
    ADDC(carry, a[1][1], (mask >> 1), carry, temp[1]);
    SHIFTR(temp[1], temp[0], 1, a[1][0], RADIX);
    a[1][1] = (temp[1] >> 1);
}


VOID_FUNC_DECL sign(const unsigned char* subseed, const unsigned char* publicKey, const unsigned char* messageDigest, unsigned char* signature)
{ // SchnorrQ signature generation
    // It produces the signature signature of a message messageDigest of size 32 in bytes
    // Inputs: 32-byte subseed, 32-byte publicKey, and messageDigest of size 32 in bytes
    // Output: 64-byte signature
    point_t R;
    unsigned char k[64] , h[64]  , temp[32 + 64] ;
    unsigned long long r[8] ;

    KangarooTwelve((unsigned char*)subseed, 32, k, 64);

    *((__m256i*)(temp + 32)) = *((__m256i*)(k + 32));
    *((__m256i*)(temp + 64)) = *((__m256i*)messageDigest);

    KangarooTwelve(temp + 32, 32 + 32, (unsigned char*)r, 64);

    ecc_mul_fixed(r, R);
    encode(R, signature); // Encode lowest 32 bytes of signature
    *((__m256i*)temp) = *((__m256i*)signature);
    *((__m256i*)(temp + 32)) = *((__m256i*)publicKey);

    KangarooTwelve(temp, 32 + 64, h, 64);
    Montgomery_multiply_mod_order(r, Montgomery_Rprime, r);
    Montgomery_multiply_mod_order(r, ONE, r);
    Montgomery_multiply_mod_order((unsigned long long*)h, Montgomery_Rprime, (unsigned long long*)h);
    Montgomery_multiply_mod_order((unsigned long long*)h, ONE, (unsigned long long*)h);
    Montgomery_multiply_mod_order((unsigned long long*)k, Montgomery_Rprime, (unsigned long long*)(signature + 32));
    Montgomery_multiply_mod_order((unsigned long long*)h, Montgomery_Rprime, (unsigned long long*)h);
    Montgomery_multiply_mod_order((unsigned long long*)(signature + 32), (unsigned long long*)h, (unsigned long long*)(signature + 32));
    Montgomery_multiply_mod_order((unsigned long long*)(signature + 32), ONE, (unsigned long long*)(signature + 32));
    if (_subborrow_u64(_subborrow_u64(_subborrow_u64(_subborrow_u64(0, r[0], ((unsigned long long*)signature)[4], &((unsigned long long*)signature)[4]), r[1], ((unsigned long long*)signature)[5], &((unsigned long long*)signature)[5]), r[2], ((unsigned long long*)signature)[6], &((unsigned long long*)signature)[6]), r[3], ((unsigned long long*)signature)[7], &((unsigned long long*)signature)[7]))
    {
        _addcarry_u64(_addcarry_u64(_addcarry_u64(_addcarry_u64(0, ((unsigned long long*)signature)[4], CURVE_ORDER_0, &((unsigned long long*)signature)[4]), ((unsigned long long*)signature)[5], CURVE_ORDER_1, &((unsigned long long*)signature)[5]), ((unsigned long long*)signature)[6], CURVE_ORDER_2, &((unsigned long long*)signature)[6]), ((unsigned long long*)signature)[7], CURVE_ORDER_3, &((unsigned long long*)signature)[7]);
    }
}

BOOL_FUNC_DECL verify(const unsigned char* publicKey, const unsigned char* messageDigest, const unsigned char* signature)
{
    point_t A;
    unsigned char temp[32 + 64];
    unsigned char h[64];

    if ((publicKey[15] & 0x80) || (signature[15] & 0x80) || (signature[62] & 0xC0) || signature[63])
    {
        return false;
    }

    if (!decode(publicKey, A)) // Also verifies that A is on the curve, if it is not it fails
    {
        return false;
    }
    memcpy(temp, signature, 32);
    memcpy(temp + 32, publicKey, 32);
    memcpy(temp + 64, messageDigest, 32);

    KangarooTwelve(temp, 32 + 64, h, 64);

    if (!ecc_mul_double((unsigned long long*)(signature + 32), (unsigned long long*)h, A))
    {
        return false;
    }

    encode(A, (unsigned char*)A);

    return (memcmp(A, signature, 32) == 0);
}

#ifdef __cplusplus
}
#endif


#endif