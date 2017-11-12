#include <string>
#include <cctype>
#include "util.hpp"

std::string capital_case(std::string str){
    str[0] = toupper(str[0]);
    return str;
}
