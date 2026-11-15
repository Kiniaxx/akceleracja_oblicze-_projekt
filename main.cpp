#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <random>
#include "Brute_Force_seq.h"
#include "MD5_seq.h"

using namespace std;
using namespace std::chrono;

constexpr int max_len = 5;

int main() {
    constexpr int pwd_length = max_len;
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
    md5(reinterpret_cast<const uint8_t*>(secret_password.c_str()), secret_password.length(), target_hash);

    cout << "Rozpoczynam lamanie hasla..." << endl;

    auto start_time = high_resolution_clock::now();

    char guess[32] = {};
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