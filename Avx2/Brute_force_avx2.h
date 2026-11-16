#ifndef AO_PROJEKT_BRUTE_FORCE_AVX2_H
#define AO_PROJEKT_BRUTE_FORCE_AVX2_H
#include <cstdint>
#include <string>

#include "MD5_avx2.h"

inline bool brute_force_avx2(const size_t *len, const uint8_t *target_hash, char *out_guess) {

    extern const std::string full_charset;

    const char* cs = full_charset.data();
    const int cs_len = static_cast<int>(full_charset.size());
    const size_t L = *len;

    // md5_8x oczekuje tablicy 8 długości (po jednej na strumień).
    // Wszystkie hasła w batchu mają tę samą długość L.
    size_t lens[8];
    for (int i = 0; i < 8; ++i) {
        lens[i] = L;
    }

    int idx[64] = {0};
    char cur[64];
    for (size_t p = 0; p < L; ++p) {
        cur[p] = cs[0];
    }

    // Trwałe bufory na 8 haseł (bez alokacji/kopii std::string w pętli)
    char guesses[8][64];
    const uint8_t* msgs[8];

    bool done = false;

    while (!done) {
        int count = 0;

        // Składamy 8 kolejnych haseł do batcha
        for (int i = 0; i < 8; ++i) {
            std::memcpy(guesses[i], cur, L);
            guesses[i][L] = '\0';
            msgs[i] = reinterpret_cast<const uint8_t*>(guesses[i]);
            count++;

            // Inkrementacja odometru: O(1) amortyzowane, bez przeszukiwania charsetu
            int p = static_cast<int>(L) - 1;
            for (; p >= 0; --p) {
                if (++idx[p] < cs_len) {
                    cur[p] = cs[idx[p]];
                    break;
                }
                idx[p] = 0;
                cur[p] = cs[0];
            }

            // p < 0 => przepełnienie: wyczerpaliśmy wszystkie hasła tej długości
            if (p < 0) {
                done = true;
                for (int j = i + 1; j < 8; ++j) {
                    msgs[j] = msgs[0];
                }
                break;
            }
        }

        // Hashowanie całej 8-mki naraz
        uint8_t sigs[8][16];
        md5_8x(msgs, lens, sigs);

        // Sprawdzamy, czy któreś z haseł pasuje do target_hash
        for (int i = 0; i < count; ++i) {
            if (memcmp(sigs[i], target_hash, 16) == 0) {
                strcpy(out_guess, guesses[i]);
                return true;
            }
        }
    }
    return false;
}

#endif //AO_PROJEKT_BRUTE_FORCE_AVX2_H
