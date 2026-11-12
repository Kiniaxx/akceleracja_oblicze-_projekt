#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <random>
#include <cstring>

using namespace std;
using namespace std::chrono;

const string all_chars = "abcdefghijklmnopqrstuvwxyz";
const string all_nums = "1234567890";
const string special_chars = "!@#$%^&*()_+{}[]:;'?.,<>?";

const string full_charset = all_chars + all_nums + special_chars;

const int max_len = 5;

const uint32_t S[64] = {
    7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
    5, 9,  14, 20, 5, 9,  14, 20, 5, 9,  14, 20, 5, 9,  14, 20,
    4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
    6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
};

const uint32_t K[64] = {
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

void md5(const uint8_t* initial_msg, size_t initial_len, uint8_t* sig) {
    uint32_t a0 = 0x67452301;
    uint32_t b0 = 0xefcdab89;
    uint32_t c0 = 0x98badcfe;
    uint32_t d0 = 0x10325476;

    size_t new_len = ((((initial_len + 8) / 64) + 1) * 64) - 8;

    vector<uint8_t> msg(initial_msg, initial_msg + initial_len);

    msg.push_back(0x80);
    while (msg.size() < new_len) {
        msg.push_back(0);
    }

    uint64_t bits_len = (uint64_t)initial_len * 8;
    for (int i = 0; i < 8; i++) {
        msg.push_back((uint8_t)(bits_len >> (i * 8)));
    }

    for (size_t offset = 0; offset < msg.size(); offset += 64) {
        uint32_t w[16];
        for (int i = 0; i < 16; i++) {
            w[i] = (uint32_t)msg[offset + i * 4] |
                  ((uint32_t)msg[offset + i * 4 + 1] << 8) |
                  ((uint32_t)msg[offset + i * 4 + 2] << 16) |
                  ((uint32_t)msg[offset + i * 4 + 3] << 24);
        }

        uint32_t A = a0;
        uint32_t B = b0;
        uint32_t C = c0;
        uint32_t D = d0;

        for (uint8_t i = 0; i < 64; i++) {
            uint32_t F = 0, g = 0;
            if (i < 16) {
                F = (B & C) | ((~B) & D);
                g = i;
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
        sig[i * 4]     = (uint8_t)(result[i] >> 0);
        sig[i * 4 + 1] = (uint8_t)(result[i] >> 8);
        sig[i * 4 + 2] = (uint8_t)(result[i] >> 16);
        sig[i * 4 + 3] = (uint8_t)(result[i] >> 24);
    }
}

bool brute_force(char* current_guess, int position, int length, const uint8_t* target_hash) {
    if (position == length) {
        uint8_t current_hash[16];

        md5((const uint8_t*)current_guess, length, current_hash);

        if (memcmp(current_hash, target_hash, 16) == 0) {
            return true;
        }
        return false;
    }

    for (char c : full_charset) {
        current_guess[position] = c;
        if (brute_force(current_guess, position + 1, length, target_hash)) {
            return true;
        }
    }
    return false;
}

int main() {
    int pwd_length = max_len;
    string secret_password = "";

    // cout << "Podaj haslo (max dlugosc: " << pwd_length << "): ";
    // cin >> secret_password;

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(0, full_charset.size() - 1);

      for (int i = 0; i < pwd_length; ++i) {
      secret_password += full_charset[dist(gen)];
      }

      cout << "Wgenerowane haslo: " << secret_password << endl;


    uint8_t target_hash[16];
    md5((const uint8_t*)secret_password.c_str(), secret_password.length(), target_hash);

    cout << "Rozpoczynam lamanie hasla..." << endl;

    auto start_time = high_resolution_clock::now();

    char guess[32];
    memset(guess, 0, sizeof(guess));
    bool found = false;

    for (int len = 1; len <= max_len; ++len) {
        if (brute_force(guess, 0, len, target_hash)) {
            guess[len] = '\0';
            found = true;
            break;
        }
    }

    auto end_time = high_resolution_clock::now();
    duration<double> total_time = end_time - start_time;

    if (found) {
        cout << "Zlamano haslo: " << guess << endl;
    } else {
        cout << "Nie znaleziono hasla." << endl;
    }

    cout << "Czas dzialania: " << total_time.count() << " sekund." << endl;

    return 0;
}