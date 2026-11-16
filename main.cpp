#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <random>

#include "Avx2/Brute_force_avx2.h"
#include "Seq/Brute_Force_seq.h"
#include "Seq/MD5_seq.h"
#include "Avx2/MD5_avx2.h"
#include "Pararrell/brute_force_Parrarell.h"

using namespace std;
using namespace std::chrono;

constexpr int max_len = 4;

int main() {
    constexpr int pwd_length = max_len;
    string secret_password = "";

    // cout << "Podaj haslo (max dlugosc: " << pwd_length << "): ";
    // cin >> secret_password;

    random_device rd;
    mt19937 gen(rd());
    // Zakładamy, że gdzies w projekcie masz: const string full_charset = "...";
    extern const string full_charset;
    uniform_int_distribution<> dist(0, full_charset.size() - 1);

    for (int i = 0; i < pwd_length; ++i) {
        secret_password += full_charset[dist(gen)];
    }

    cout << "Wygenerowane haslo: " << secret_password << "\n\n";

    uint8_t target_hash[16];
    md5(reinterpret_cast<const uint8_t*>(secret_password.c_str()), secret_password.length(), target_hash);

    cout << "--- Rozpoczynam lamanie hasla (Sekwencyjne) ---" << endl;
    auto start_seq = high_resolution_clock::now();

    char guess_seq[32] = {};
    bool found_seq = false;

    for (int len = 1; len <= max_len; ++len) {
        if (brute_force_seq(guess_seq, 0, len, target_hash)) {
            guess_seq[len] = '\0';
            found_seq = true;
            break;
        }
    }

    auto end_seq = high_resolution_clock::now();
    duration<double> time_seq = end_seq - start_seq;

    if (found_seq) {
        cout << "Zlamano haslo: " << guess_seq << endl;
    } else {
        cout << "Nie znaleziono hasla." << endl;
    }
    cout << "Czas dzialania (Seq): " << time_seq.count() << " sekund.\n\n";

    cout << "Rozpoczynam lamanie hasla (AVX2)" << endl;
    auto start_avx2 = high_resolution_clock::now();

    char guess_avx2[32] = {};
    bool found_avx2 = false;

    for (size_t len = 1; len <= max_len; ++len) {
        if (brute_force_avx2(&len, target_hash, guess_avx2)) {
            found_avx2 = true;
            break;
        }
    }

    auto end_avx2 = high_resolution_clock::now();
    duration<double> time_avx2 = end_avx2 - start_avx2;

    if (found_avx2) {
        cout << "Zlamano haslo: " << guess_avx2 << endl;
    } else {
        cout << "Nie znaleziono hasla." << endl;
    }
    cout << "Czas dzialania (AVX2): " << time_avx2.count() << " sekund.\n\n";

    cout << "Rozpoczynam lamanie hasla (Wspolbieznie: watki + AVX2)" << endl;
    const unsigned n_threads = thread::hardware_concurrency();
    cout << "Liczba watkow: " << (n_threads ? n_threads : 4) << endl;
    auto start_par = high_resolution_clock::now();

    char guess_par[32] = {};
    bool found_par = false;

    for (size_t len = 1; len <= max_len; ++len) {
        if (brute_force_parallel(&len, target_hash, guess_par)) {
            found_par = true;
            break;
        }
    }

    auto end_par = high_resolution_clock::now();
    duration<double> time_par = end_par - start_par;

    if (found_par) {
        cout << "Zlamano haslo: " << guess_par << endl;
    } else {
        cout << "Nie znaleziono hasla." << endl;
    }
    cout << "Czas dzialania (Parallel + AVX2): " << time_par.count() << " sekund.\n\n";

    cout << "Rozpoczynam lamanie hasla (Wspolbieznie bez AVX: tylko watki)" << endl;
    auto start_par_seq = high_resolution_clock::now();

    char guess_par_seq[32] = {};
    bool found_par_seq = false;

    for (size_t len = 1; len <= max_len; ++len) {
        if (brute_force_parallel_seq(&len, target_hash, guess_par_seq)) {
            found_par_seq = true;
            break;
        }
    }

    auto end_par_seq = high_resolution_clock::now();
    duration<double> time_par_seq = end_par_seq - start_par_seq;

    if (found_par_seq) {
        cout << "Zlamano haslo: " << guess_par_seq << endl;
    } else {
        cout << "Nie znaleziono hasla." << endl;
    }
    cout << "Czas dzialania (Parallel bez AVX): " << time_par_seq.count() << " sekund.\n\n";

    cout << "Czas Sekwencyjny:       " << time_seq.count() << " s" << endl;
    cout << "Czas AVX2:              " << time_avx2.count() << " s" << endl;
    cout << "Czas Parallel (watki):  " << time_par_seq.count() << " s" << endl;
    cout << "Czas Parallel + AVX2:   " << time_par.count() << " s" << endl;

    if (time_avx2.count() > 0) {
        cout << "Przyspieszenie AVX2:            " << time_seq.count() / time_avx2.count() << "x" << endl;
    }
    if (time_par_seq.count() > 0) {
        cout << "Przyspieszenie Parallel:        " << time_seq.count() / time_par_seq.count() << "x" << endl;
    }
    if (time_par.count() > 0) {
        cout << "Przyspieszenie Parallel + AVX2: " << time_seq.count() / time_par.count() << "x" << endl;
    }

    return 0;
}