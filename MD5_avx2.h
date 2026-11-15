#ifndef AO_PROJEKT_MD5_AVX2_H
#define AO_PROJEKT_MD5_AVX2_H

#include <avxintrin.h>
#include <cstdint>
#include <emmintrin.h>
#include <vector>
#include <avx2intrin.h>

const uint32_t S[64] = {
    7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
    5, 9,  14, 20, 5, 9,  14, 20, 5, 9,  14, 20, 5, 9,  14, 20,
    4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
    6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
};

inline __m256i mm256_rol_epi32(__m256i x, int c) {
    const __m128i count = _mm_cvtsi32_si128(c);
    const __m128i count_inv = _mm_cvtsi32_si128(32 - c);
    return _mm256_or_si256(
        _mm256_sll_epi32(x, count),
        _mm256_srl_epi32(x, count_inv)
    );
}


static void step_1(__m256i& A, __m256i& B, __m256i& C, __m256i& D, __m256i& W1_8, __m256i& VK, int shift)
{
    // F = (B & C) | ((~B) & D)
    __m256i F = _mm256_or_si256(
        _mm256_and_si256(B, C),
        _mm256_andnot_si256(B, D) // Wykonuje: (~B) & D
    );

    // A = A + F + K[i] + w[g]
    A = _mm256_add_epi32(A, F);
    A = _mm256_add_epi32(A, VK);
    A = _mm256_add_epi32(A, W1_8);

    // A = B + LEFTROTATE(A, shift)
    A = mm256_rol_epi32(A, shift);
    A = _mm256_add_epi32(B, A);
}

constexpr uint32_t K[64] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

#define LEFTROTATE(x, c) (((x) << (c)) | ((x) >> (32 - (c))))

inline void md5(const uint8_t* initial_msg, size_t initial_len, uint8_t* sig) {
    __m256i vA = _mm256_set1_epi32(0x67452301);
    __m256i vB = _mm256_set1_epi32(0xefcdab89);
    __m256i vC = _mm256_set1_epi32(0x98badcfe);
    __m256i vD = _mm256_set1_epi32(0x10325476);

    const size_t new_len = ((((initial_len + 8) / 64) + 1) * 64) - 8;

    std::vector<uint8_t> msg(initial_msg, initial_msg + initial_len);

    msg.push_back(0x80);
    while (msg.size() < new_len) {
        msg.push_back(0);
    }

    uint64_t bits_len = (uint64_t)initial_len * 8;
    for (int i = 0; i < 8; i++) {
        msg.push_back((uint8_t)(bits_len >> (i * 8)));
    }

    for (std::size_t offset = 0; offset < msg.size(); offset += 64) {
        uint32_t w[16];
        for (int i = 0; i < 16; i++) {
            w[i] = static_cast<uint32_t>(msg[offset + i * 4]) |
                  (static_cast<uint32_t>(msg[offset + i * 4 + 1]) << 8) |
                  (static_cast<uint32_t>(msg[offset + i * 4 + 2]) << 16) |
                  (static_cast<uint32_t>(msg[offset + i * 4 + 3]) << 24);
        }

        __m256i a = *vA, b = *vB, c = *vC, d = *vD;

        for (uint8_t i = 0; i < 64; i++) {
            uint32_t F = 0, g = 0;
            if (i < 16) {
                step_1(a, b, c, d, w[0], _mm256_set1_epi32(K[0]), S[0]);
            } else if (i < 32) {
                F = (D & B) | ((~D) & C);
                g = (5 * i + 1) % 16;
            } else if (i < 48) {
                F = B ^ C ^ D;
                g = (3 * i + 5) % 16;
            } else {
                F = C ^ (B | (~D));
                g = (7 * i) % 16;
            }

            uint32_t temp = D;
            D = C;
            C = B;
            B = B + LEFTROTATE((A + F + K[i] + w[g]), S[i]);
            A = temp;
        }

        a0 += A;
        b0 += B;
        c0 += C;
        d0 += D;
    }

    uint32_t result[] = {a0, b0, c0, d0};
    for (int i = 0; i < 4; i++) {
        sig[i * 4]     = static_cast<uint8_t>(result[i] >> 0);
        sig[i * 4 + 1] = static_cast<uint8_t>(result[i] >> 8);
        sig[i * 4 + 2] = static_cast<uint8_t>(result[i] >> 16);
        sig[i * 4 + 3] = static_cast<uint8_t>(result[i] >> 24);
    }
}

#endif //AO_PROJEKT_MD5_AVX2_H
