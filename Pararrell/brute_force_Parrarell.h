#ifndef AO_PROJEKT_BRUTE_FORCE_PARRARELL_H
#define AO_PROJEKT_BRUTE_FORCE_PARRARELL_H

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include "../Avx2/MD5_avx2.h"
#include "../Seq/MD5_seq.h"

static inline void pll_index_to_guess(uint64_t n, size_t L, const char* cs, int cs_len,
                                      int* idx, char* cur) {
    for (int p = static_cast<int>(L) - 1; p >= 0; --p) {
        idx[p] = static_cast<int>(n % static_cast<uint64_t>(cs_len));
        n /= static_cast<uint64_t>(cs_len);
        cur[p] = cs[idx[p]];
    }
}

inline bool brute_force_parallel(const size_t* len, const uint8_t* target_hash,
                                 char* out_guess, unsigned num_threads = 0) {
    extern const std::string full_charset;
    const char* cs = full_charset.data();
    const int cs_len = static_cast<int>(full_charset.size());
    const size_t L = *len;

    if (L == 0) {
        return false;
    }

    uint64_t total = 1;
    for (size_t i = 0; i < L; ++i) {
        total *= static_cast<uint64_t>(cs_len);
    }

    if (num_threads == 0) {
        num_threads = std::thread::hardware_concurrency();
        if (num_threads == 0) {
            num_threads = 4;
        }
    }
    if (static_cast<uint64_t>(num_threads) > total) {
        num_threads = static_cast<unsigned>(total);
    }

    std::atomic<bool> found(false);
    std::mutex result_mutex;
    char result[64] = {};

    auto worker = [&](uint64_t start, uint64_t end) {
        size_t lens[8];
        for (int i = 0; i < 8; ++i) {
            lens[i] = L;
        }

        int idx[64] = {0};
        char cur[64] = {0};
        pll_index_to_guess(start, L, cs, cs_len, idx, cur);

        char guesses[8][64];
        const uint8_t* msgs[8];

        uint64_t pos = start;
        while (pos < end && !found.load(std::memory_order_relaxed)) {
            int count = 0;

            for (int i = 0; i < 8 && pos < end; ++i) {
                std::memcpy(guesses[i], cur, L);
                guesses[i][L] = '\0';
                msgs[i] = reinterpret_cast<const uint8_t*>(guesses[i]);
                ++count;
                ++pos;

                int p = static_cast<int>(L) - 1;
                for (; p >= 0; --p) {
                    if (++idx[p] < cs_len) {
                        cur[p] = cs[idx[p]];
                        break;
                    }
                    idx[p] = 0;
                    cur[p] = cs[0];
                }
            }

            // Dopełnienie nieużytych slotów poprawnym wskaźnikiem dla AVX2
            for (int j = count; j < 8; ++j) {
                msgs[j] = msgs[0];
            }

            uint8_t sigs[8][16];
            md5_8x(msgs, lens, sigs);

            for (int i = 0; i < count; ++i) {
                if (std::memcmp(sigs[i], target_hash, 16) == 0) {
                    std::lock_guard<std::mutex> lock(result_mutex);
                    std::memcpy(result, guesses[i], L);
                    result[L] = '\0';
                    found.store(true, std::memory_order_relaxed);
                    return;
                }
            }
        }
    };

    // Podział zakresu: reszta rozdzielona po jednym indeksie na początkowe wątki
    const uint64_t chunk = total / num_threads;
    const uint64_t rem = total % num_threads;

    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    uint64_t start = 0;
    for (unsigned t = 0; t < num_threads; ++t) {
        uint64_t sz = chunk + (t < rem ? 1 : 0);
        uint64_t end = start + sz;
        threads.emplace_back(worker, start, end);
        start = end;
    }

    for (auto& th : threads) {
        th.join();
    }

    if (found.load()) {
        std::strcpy(out_guess, result);
        return true;
    }
    return false;
}

inline bool brute_force_parallel_seq(const size_t* len, const uint8_t* target_hash,
                                     char* out_guess, unsigned num_threads = 0) {
    extern const std::string full_charset;
    const char* cs = full_charset.data();
    const int cs_len = static_cast<int>(full_charset.size());
    const size_t L = *len;

    if (L == 0) {
        return false;
    }

    // Całkowita liczba kombinacji = cs_len^L
    uint64_t total = 1;
    for (size_t i = 0; i < L; ++i) {
        total *= static_cast<uint64_t>(cs_len);
    }

    if (num_threads == 0) {
        num_threads = std::thread::hardware_concurrency();
        if (num_threads == 0) {
            num_threads = 4;
        }
    }
    if (static_cast<uint64_t>(num_threads) > total) {
        num_threads = static_cast<unsigned>(total);
    }

    std::atomic<bool> found(false);
    std::mutex result_mutex;
    char result[64] = {};

    auto worker = [&](uint64_t start, uint64_t end) {
        int idx[64] = {0};
        char cur[64] = {0};
        pll_index_to_guess(start, L, cs, cs_len, idx, cur);

        uint8_t sig[16];

        for (uint64_t pos = start; pos < end; ++pos) {
            if (found.load(std::memory_order_relaxed)) {
                return;
            }

            md5(reinterpret_cast<const uint8_t*>(cur), L, sig);

            if (std::memcmp(sig, target_hash, 16) == 0) {
                std::lock_guard<std::mutex> lock(result_mutex);
                std::memcpy(result, cur, L);
                result[L] = '\0';
                found.store(true, std::memory_order_relaxed);
                return;
            }

            int p = static_cast<int>(L) - 1;
            for (; p >= 0; --p) {
                if (++idx[p] < cs_len) {
                    cur[p] = cs[idx[p]];
                    break;
                }
                idx[p] = 0;
                cur[p] = cs[0];
            }
        }
    };

    const uint64_t chunk = total / num_threads;
    const uint64_t rem = total % num_threads;

    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    uint64_t start = 0;
    for (unsigned t = 0; t < num_threads; ++t) {
        uint64_t sz = chunk + (t < rem ? 1 : 0);
        uint64_t end = start + sz;
        threads.emplace_back(worker, start, end);
        start = end;
    }

    for (auto& th : threads) {
        th.join();
    }

    if (found.load()) {
        std::strcpy(out_guess, result);
        return true;
    }
    return false;
}

#endif //AO_PROJEKT_BRUTE_FORCE_PARRARELL_H
