#ifndef AO_PROJEKT_CONSTS_H
#define AO_PROJEKT_CONSTS_H
#include <string>

const std::string all_chars = "abcdefghijklmnopqrstuvwxyz";
constexpr std::string all_nums = "1234567890";
const std::string special_chars = "!@#$%^&*()_+{}[]:;'?.,<>?";

const std::string full_charset = all_chars + all_nums + special_chars;

#endif //AO_PROJEKT_CONSTS_H
