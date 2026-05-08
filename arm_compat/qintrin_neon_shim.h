#include <arm_neon.h>
#include <cstring>
#include <cstdint>

// 256-bit integer type emulated as two 128-bit NEON vectors.
// Layout is byte-identical to Intel's __m256i (low 16 bytes first).
struct __m256i
{
    uint8x16_t lo;
    uint8x16_t hi;
};

struct __m128i_compat
{
    uint8x16_t v;
};

// ---- load / store ---------------------------------------------------------

static inline __m256i _mm256_loadu_si256(const __m256i* p) noexcept
{
    __m256i r;
    const uint8_t* s = reinterpret_cast<const uint8_t*>(p);
    r.lo = vld1q_u8(s);
    r.hi = vld1q_u8(s + 16);
    return r;
}

static inline __m256i _mm256_lddqu_si256(const __m256i* p) noexcept
{
    return _mm256_loadu_si256(p);
}

static inline void _mm256_storeu_si256(__m256i* p, __m256i v) noexcept
{
    uint8_t* d = reinterpret_cast<uint8_t*>(p);
    vst1q_u8(d,      v.lo);
    vst1q_u8(d + 16, v.hi);
}

// ---- constructors ---------------------------------------------------------

static inline __m256i _mm256_setzero_si256() noexcept
{
    __m256i r;
    r.lo = vdupq_n_u8(0);
    r.hi = vdupq_n_u8(0);
    return r;
}

static inline __m256i _mm256_set_epi64x(int64_t e3, int64_t e2,
                                        int64_t e1, int64_t e0) noexcept
{
    __m256i r;
    int64_t lo[2] = { e0, e1 };
    int64_t hi[2] = { e2, e3 };
    r.lo = vreinterpretq_u8_s64(vld1q_s64(lo));
    r.hi = vreinterpretq_u8_s64(vld1q_s64(hi));
    return r;
}

static inline __m256i _mm256_set_epi8(
    int8_t e31, int8_t e30, int8_t e29, int8_t e28,
    int8_t e27, int8_t e26, int8_t e25, int8_t e24,
    int8_t e23, int8_t e22, int8_t e21, int8_t e20,
    int8_t e19, int8_t e18, int8_t e17, int8_t e16,
    int8_t e15, int8_t e14, int8_t e13, int8_t e12,
    int8_t e11, int8_t e10, int8_t e9,  int8_t e8,
    int8_t e7,  int8_t e6,  int8_t e5,  int8_t e4,
    int8_t e3,  int8_t e2,  int8_t e1,  int8_t e0) noexcept
{
    // Note: Intel's _mm256_set_epi8 lists arguments from highest to lowest,
    // but in memory the lowest-indexed lane is at the lowest address.
    int8_t bytes[32] = {
        e0,  e1,  e2,  e3,  e4,  e5,  e6,  e7,
        e8,  e9,  e10, e11, e12, e13, e14, e15,
        e16, e17, e18, e19, e20, e21, e22, e23,
        e24, e25, e26, e27, e28, e29, e30, e31
    };
    __m256i r;
    r.lo = vld1q_u8(reinterpret_cast<const uint8_t*>(bytes));
    r.hi = vld1q_u8(reinterpret_cast<const uint8_t*>(bytes + 16));
    return r;
}

// ---- compare / mask -------------------------------------------------------

static inline __m256i _mm256_cmpeq_epi64(__m256i a, __m256i b) noexcept
{
    __m256i r;
    uint64x2_t lo = vceqq_u64(vreinterpretq_u64_u8(a.lo), vreinterpretq_u64_u8(b.lo));
    uint64x2_t hi = vceqq_u64(vreinterpretq_u64_u8(a.hi), vreinterpretq_u64_u8(b.hi));
    r.lo = vreinterpretq_u8_u64(lo);
    r.hi = vreinterpretq_u8_u64(hi);
    return r;
}

// Emulates _mm256_movemask_epi8: for each of 32 bytes, take its top bit and
// pack into a 32-bit integer (lane 0 -> bit 0).
static inline int _mm256_movemask_epi8(__m256i a) noexcept
{
    // Standard NEON trick: AND each byte with a per-lane bit, then horizontally add.
    static const uint8_t mask_lo_arr[16] = {
        1, 2, 4, 8, 16, 32, 64, 128, 1, 2, 4, 8, 16, 32, 64, 128
    };
    const uint8x16_t bit_mask = vld1q_u8(mask_lo_arr);

    // Keep only sign bits, then map to per-lane bit positions.
    uint8x16_t sign_lo = vshrq_n_u8(a.lo, 7);
    uint8x16_t sign_hi = vshrq_n_u8(a.hi, 7);

    uint8x16_t weighted_lo = vandq_u8(vtstq_u8(sign_lo, vdupq_n_u8(1)), bit_mask);
    uint8x16_t weighted_hi = vandq_u8(vtstq_u8(sign_hi, vdupq_n_u8(1)), bit_mask);

    // Sum the two 8-byte halves of each 128-bit vector to get one byte per half.
    uint8_t lo0 = vaddv_u8(vget_low_u8(weighted_lo));
    uint8_t lo1 = vaddv_u8(vget_high_u8(weighted_lo));
    uint8_t hi0 = vaddv_u8(vget_low_u8(weighted_hi));
    uint8_t hi1 = vaddv_u8(vget_high_u8(weighted_hi));

    return  (int)lo0
         | ((int)lo1 << 8)
         | ((int)hi0 << 16)
         | ((int)hi1 << 24);
}

// ---- testz ---------------------------------------------------------------

// Returns 1 if (a & b) is all zero, else 0 (matches Intel semantics for our use).
static inline int _mm256_testz_si256(__m256i a, __m256i b) noexcept
{
    uint8x16_t and_lo = vandq_u8(a.lo, b.lo);
    uint8x16_t and_hi = vandq_u8(a.hi, b.hi);
    uint8x16_t or_all = vorrq_u8(and_lo, and_hi);
    return (vmaxvq_u8(or_all) == 0) ? 1 : 0;
}

// ---- RNG ------------------------------------------------------------------

#include <stdlib.h>   // arc4random_buf on Apple platforms

static inline int _rdrand64_step(unsigned long long* out) noexcept
{
#if defined(__ARM_FEATURE_RNG)
    // Hardware RNG via the FEAT_RNG extension (not all Apple cores expose this
    // to userspace, so we keep it guarded).
    unsigned long long v;
    unsigned long long ok;
    __asm__ volatile("mrs %0, S3_3_C2_C4_0\n" // RNDR
                     "cset %w1, ne\n"
                     : "=r"(v), "=r"(ok));
    if (ok) { *out = v; return 1; }
#endif
    arc4random_buf(out, sizeof(*out));
    return 1;
}