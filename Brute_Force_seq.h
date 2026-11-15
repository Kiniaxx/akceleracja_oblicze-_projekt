#ifndef AO_PROJEKT_BRUTE_FORCE_SEQ_H
#define AO_PROJEKT_BRUTE_FORCE_SEQ_H
#include <cstring>
#include <string>

#include "Consts.h"
#include "MD5_seq.h"

static bool brute_force(char* current_guess, int position, int length, const uint8_t* target_hash) {
    if (position == length) {
        uint8_t current_hash[16];

        md5(reinterpret_cast<const uint8_t*>(current_guess), length, current_hash);

        if (memcmp(current_hash, target_hash, 16) == 0) {
            return true;
        }
        return false;
    }

    for (const char c : full_charset) {
        current_guess[position] = c;
        if (brute_force(current_guess, position + 1, length, target_hash)) {
            return true;
        }
    }
    return false;
}

#endif //AO_PROJEKT_BRUTE_FORCE_SEQ_H
