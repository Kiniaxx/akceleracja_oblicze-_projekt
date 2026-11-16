#ifndef AO_PROJEKT_MD5_HELPER_H
#define AO_PROJEKT_MD5_HELPER_H
#include <immintrin.h>
#include <cstdint>

template<int c>
static inline __m256i mm256_rol_epi32(__m256i x) {
    return _mm256_or_si256(_mm256_slli_epi32(x, c), _mm256_srli_epi32(x, 32 - c));
}

static inline __m256i md5_F(__m256i b, __m256i c, __m256i d) {
    return _mm256_or_si256(_mm256_and_si256(b, c), _mm256_andnot_si256(b, d));
}
static inline __m256i md5_G(__m256i b, __m256i c, __m256i d) {
    return _mm256_or_si256(_mm256_and_si256(b, d), _mm256_andnot_si256(d, c));
}
static inline __m256i md5_H(__m256i b, __m256i c, __m256i d) {
    return _mm256_xor_si256(_mm256_xor_si256(b, c), d);
}
static inline __m256i md5_I(__m256i b, __m256i c, __m256i d) {
    const __m256i ones = _mm256_set1_epi32(-1);
    return _mm256_xor_si256(c, _mm256_or_si256(b, _mm256_xor_si256(d, ones)));
}

// Transpozycja macierzy 8x8 (uint32) w rejestrach AVX2.
// in[j] = wiersz j (8 słów); out[k] = kolumna k (słowo k z każdego wiersza).
// Zastępuje wolny "gather" przez _mm256_set_epi32 (zestaw unpack + permute).
static inline void transpose8x8_epi32(const __m256i in[8], __m256i out[8]) {
    __m256i t0 = _mm256_unpacklo_epi32(in[0], in[1]);
    __m256i t1 = _mm256_unpackhi_epi32(in[0], in[1]);
    __m256i t2 = _mm256_unpacklo_epi32(in[2], in[3]);
    __m256i t3 = _mm256_unpackhi_epi32(in[2], in[3]);
    __m256i t4 = _mm256_unpacklo_epi32(in[4], in[5]);
    __m256i t5 = _mm256_unpackhi_epi32(in[4], in[5]);
    __m256i t6 = _mm256_unpacklo_epi32(in[6], in[7]);
    __m256i t7 = _mm256_unpackhi_epi32(in[6], in[7]);

    __m256i s0 = _mm256_unpacklo_epi64(t0, t2);
    __m256i s1 = _mm256_unpackhi_epi64(t0, t2);
    __m256i s2 = _mm256_unpacklo_epi64(t1, t3);
    __m256i s3 = _mm256_unpackhi_epi64(t1, t3);
    __m256i s4 = _mm256_unpacklo_epi64(t4, t6);
    __m256i s5 = _mm256_unpackhi_epi64(t4, t6);
    __m256i s6 = _mm256_unpacklo_epi64(t5, t7);
    __m256i s7 = _mm256_unpackhi_epi64(t5, t7);

    out[0] = _mm256_permute2x128_si256(s0, s4, 0x20);
    out[1] = _mm256_permute2x128_si256(s1, s5, 0x20);
    out[2] = _mm256_permute2x128_si256(s2, s6, 0x20);
    out[3] = _mm256_permute2x128_si256(s3, s7, 0x20);
    out[4] = _mm256_permute2x128_si256(s0, s4, 0x31);
    out[5] = _mm256_permute2x128_si256(s1, s5, 0x31);
    out[6] = _mm256_permute2x128_si256(s2, s6, 0x31);
    out[7] = _mm256_permute2x128_si256(s3, s7, 0x31);
}

#define MD5_STEP(F, a, b, c, d, x, k, s)                                        \
    a = _mm256_add_epi32(a,                                                     \
            _mm256_add_epi32(_mm256_add_epi32(F(b, c, d),                       \
                             _mm256_set1_epi32((int)(k))), x));                 \
    a = _mm256_add_epi32(b, mm256_rol_epi32<s>(a));

static inline void md5_transform_avx2(__m256i& a0, __m256i& b0, __m256i& c0, __m256i& d0, const __m256i W[16]) {
    __m256i a = a0, b = b0, c = c0, d = d0;

    // Runda 1
    MD5_STEP(md5_F, a, b, c, d, W[0],  0xd76aa478, 7)
    MD5_STEP(md5_F, d, a, b, c, W[1],  0xe8c7b756, 12)
    MD5_STEP(md5_F, c, d, a, b, W[2],  0x242070db, 17)
    MD5_STEP(md5_F, b, c, d, a, W[3],  0xc1bdceee, 22)
    MD5_STEP(md5_F, a, b, c, d, W[4],  0xf57c0faf, 7)
    MD5_STEP(md5_F, d, a, b, c, W[5],  0x4787c62a, 12)
    MD5_STEP(md5_F, c, d, a, b, W[6],  0xa8304613, 17)
    MD5_STEP(md5_F, b, c, d, a, W[7],  0xfd469501, 22)
    MD5_STEP(md5_F, a, b, c, d, W[8],  0x698098d8, 7)
    MD5_STEP(md5_F, d, a, b, c, W[9],  0x8b44f7af, 12)
    MD5_STEP(md5_F, c, d, a, b, W[10], 0xffff5bb1, 17)
    MD5_STEP(md5_F, b, c, d, a, W[11], 0x895cd7be, 22)
    MD5_STEP(md5_F, a, b, c, d, W[12], 0x6b901122, 7)
    MD5_STEP(md5_F, d, a, b, c, W[13], 0xfd987193, 12)
    MD5_STEP(md5_F, c, d, a, b, W[14], 0xa679438e, 17)
    MD5_STEP(md5_F, b, c, d, a, W[15], 0x49b40821, 22)

    // Runda 2
    MD5_STEP(md5_G, a, b, c, d, W[1],  0xf61e2562, 5)
    MD5_STEP(md5_G, d, a, b, c, W[6],  0xc040b340, 9)
    MD5_STEP(md5_G, c, d, a, b, W[11], 0x265e5a51, 14)
    MD5_STEP(md5_G, b, c, d, a, W[0],  0xe9b6c7aa, 20)
    MD5_STEP(md5_G, a, b, c, d, W[5],  0xd62f105d, 5)
    MD5_STEP(md5_G, d, a, b, c, W[10], 0x02441453, 9)
    MD5_STEP(md5_G, c, d, a, b, W[15], 0xd8a1e681, 14)
    MD5_STEP(md5_G, b, c, d, a, W[4],  0xe7d3fbc8, 20)
    MD5_STEP(md5_G, a, b, c, d, W[9],  0x21e1cde6, 5)
    MD5_STEP(md5_G, d, a, b, c, W[14], 0xc33707d6, 9)
    MD5_STEP(md5_G, c, d, a, b, W[3],  0xf4d50d87, 14)
    MD5_STEP(md5_G, b, c, d, a, W[8],  0x455a14ed, 20)
    MD5_STEP(md5_G, a, b, c, d, W[13], 0xa9e3e905, 5)
    MD5_STEP(md5_G, d, a, b, c, W[2],  0xfcefa3f8, 9)
    MD5_STEP(md5_G, c, d, a, b, W[7],  0x676f02d9, 14)
    MD5_STEP(md5_G, b, c, d, a, W[12], 0x8d2a4c8a, 20)

    // Runda 3
    MD5_STEP(md5_H, a, b, c, d, W[5],  0xfffa3942, 4)
    MD5_STEP(md5_H, d, a, b, c, W[8],  0x8771f681, 11)
    MD5_STEP(md5_H, c, d, a, b, W[11], 0x6d9d6122, 16)
    MD5_STEP(md5_H, b, c, d, a, W[14], 0xfde5380c, 23)
    MD5_STEP(md5_H, a, b, c, d, W[1],  0xa4beea44, 4)
    MD5_STEP(md5_H, d, a, b, c, W[4],  0x4bdecfa9, 11)
    MD5_STEP(md5_H, c, d, a, b, W[7],  0xf6bb4b60, 16)
    MD5_STEP(md5_H, b, c, d, a, W[10], 0xbebfbc70, 23)
    MD5_STEP(md5_H, a, b, c, d, W[13], 0x289b7ec6, 4)
    MD5_STEP(md5_H, d, a, b, c, W[0],  0xeaa127fa, 11)
    MD5_STEP(md5_H, c, d, a, b, W[3],  0xd4ef3085, 16)
    MD5_STEP(md5_H, b, c, d, a, W[6],  0x04881d05, 23)
    MD5_STEP(md5_H, a, b, c, d, W[9],  0xd9d4d039, 4)
    MD5_STEP(md5_H, d, a, b, c, W[12], 0xe6db99e5, 11)
    MD5_STEP(md5_H, c, d, a, b, W[15], 0x1fa27cf8, 16)
    MD5_STEP(md5_H, b, c, d, a, W[2],  0xc4ac5665, 23)

    // Runda 4
    MD5_STEP(md5_I, a, b, c, d, W[0],  0xf4292244, 6)
    MD5_STEP(md5_I, d, a, b, c, W[7],  0x432aff97, 10)
    MD5_STEP(md5_I, c, d, a, b, W[14], 0xab9423a7, 15)
    MD5_STEP(md5_I, b, c, d, a, W[5],  0xfc93a039, 21)
    MD5_STEP(md5_I, a, b, c, d, W[12], 0x655b59c3, 6)
    MD5_STEP(md5_I, d, a, b, c, W[3],  0x8f0ccc92, 10)
    MD5_STEP(md5_I, c, d, a, b, W[10], 0xffeff47d, 15)
    MD5_STEP(md5_I, b, c, d, a, W[1],  0x85845dd1, 21)
    MD5_STEP(md5_I, a, b, c, d, W[8],  0x6fa87e4f, 6)
    MD5_STEP(md5_I, d, a, b, c, W[15], 0xfe2ce6e0, 10)
    MD5_STEP(md5_I, c, d, a, b, W[6],  0xa3014314, 15)
    MD5_STEP(md5_I, b, c, d, a, W[13], 0x4e0811a1, 21)
    MD5_STEP(md5_I, a, b, c, d, W[4],  0xf7537e82, 6)
    MD5_STEP(md5_I, d, a, b, c, W[11], 0xbd3af235, 10)
    MD5_STEP(md5_I, c, d, a, b, W[2],  0x2ad7d2bb, 15)
    MD5_STEP(md5_I, b, c, d, a, W[9],  0xeb86d391, 21)

    a0 = _mm256_add_epi32(a0, a);
    b0 = _mm256_add_epi32(b0, b);
    c0 = _mm256_add_epi32(c0, c);
    d0 = _mm256_add_epi32(d0, d);
}

#endif //AO_PROJEKT_MD5_HELPER_H
