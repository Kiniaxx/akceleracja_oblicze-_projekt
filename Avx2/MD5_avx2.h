#ifndef AO_PROJEKT_MD5_AVX2_H
#define AO_PROJEKT_MD5_AVX2_H

#include <immintrin.h>
#include <cstdint>
#include <cstring>
#include "MD5_Helper.h"

inline void md5_8x(const uint8_t* msgs[8], const size_t lens[8], uint8_t sigs[8][16]) {
    size_t blocks[8];
    size_t max_blocks = 0;

    for (int i = 0; i < 8; ++i) {
        blocks[i] = (lens[i] + 8) / 64 + 1;
        if (blocks[i] > max_blocks) {
            max_blocks = blocks[i];
        }
    }

    __m256i vA = _mm256_set1_epi32((int)0x67452301);
    __m256i vB = _mm256_set1_epi32((int)0xefcdab89);
    __m256i vC = _mm256_set1_epi32((int)0x98badcfe);
    __m256i vD = _mm256_set1_epi32((int)0x10325476);

    for (size_t block = 0; block < max_blocks; ++block) {
        alignas(32) uint8_t buffers[8][64];

        alignas(32) uint32_t act[8];

        for (int lane = 0; lane < 8; ++lane) {
            uint8_t* buf = buffers[lane];
            std::memset(buf, 0, 64);

            if (block >= blocks[lane]) {
                act[lane] = 0; // strumień zakończony - nie aktualizujemy jego stanu
                continue;
            }
            act[lane] = 0xFFFFFFFFu;

            size_t offset = block * 64;
            size_t remaining = (lens[lane] > offset) ? (lens[lane] - offset) : 0;

            if (remaining >= 64) {
                std::memcpy(buf, msgs[lane] + offset, 64);
            } else {
                if (remaining > 0) {
                    std::memcpy(buf, msgs[lane] + offset, remaining);
                }
                // Bit '1' (0x80) zaraz za danymi
                if (offset <= lens[lane] && lens[lane] < offset + 64) {
                    buf[remaining] = 0x80;
                }
                // Oryginalna długość w bitach w ostatnim bloku (x86 = little-endian)
                if (block == blocks[lane] - 1) {
                    uint64_t bits_len = static_cast<uint64_t>(lens[lane]) * 8;
                    std::memcpy(&buf[56], &bits_len, 8);
                }
            }
        }

        // Transpozycja AoS -> SoA: z 8 wierszy (po 16 słów) budujemy 16 wektorów W.
        // Słowa 0-7 i 8-15 to dwie niezależne transpozycje 8x8.
        __m256i rows_lo[8], rows_hi[8];
        for (int lane = 0; lane < 8; ++lane) {
            rows_lo[lane] = _mm256_load_si256(reinterpret_cast<const __m256i*>(buffers[lane]));
            rows_hi[lane] = _mm256_load_si256(reinterpret_cast<const __m256i*>(buffers[lane] + 32));
        }

        __m256i W[16];
        transpose8x8_epi32(rows_lo, &W[0]);
        transpose8x8_epi32(rows_hi, &W[8]);

        // Rdzeń MD5 na kopii stanu
        __m256i a = vA, b = vB, c = vC, d = vD;
        md5_transform_avx2(a, b, c, d, W);

        // Aktualizujemy stan tylko dla aktywnych strumieni (maska per-lane)
        __m256i mask = _mm256_load_si256(reinterpret_cast<const __m256i*>(act));
        vA = _mm256_blendv_epi8(vA, a, mask);
        vB = _mm256_blendv_epi8(vB, b, mask);
        vC = _mm256_blendv_epi8(vC, c, mask);
        vD = _mm256_blendv_epi8(vD, d, mask);
    }

    // Rozpakowanie stanu do tablic i zapis sygnatur (16 bajtów, little-endian)
    alignas(32) uint32_t ra[8], rb[8], rc[8], rd[8];
    _mm256_store_si256(reinterpret_cast<__m256i*>(ra), vA);
    _mm256_store_si256(reinterpret_cast<__m256i*>(rb), vB);
    _mm256_store_si256(reinterpret_cast<__m256i*>(rc), vC);
    _mm256_store_si256(reinterpret_cast<__m256i*>(rd), vD);

    for (int lane = 0; lane < 8; ++lane) {
        uint32_t final_res[4] = { ra[lane], rb[lane], rc[lane], rd[lane] };
        std::memcpy(sigs[lane], final_res, 16);
    }
}

#endif //AO_PROJEKT_MD5_AVX2_H
