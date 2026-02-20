#pragma once



#include <iostream>
#include <bitset>
#include <cmath>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

// Function to convert binary to integer
int binaryToInt(const std::string& binary) {
    return std::bitset<16>(binary).to_ulong();
}

// Function to convert integer to binary
std::string intToBinary(int number) {
    std::bitset<16> binary(number);
    return binary.to_string();
}

int test_binary_int_conv() {
    // Binary to Integer conversion
    std::string binaryNumber = "101010";
    int decimalNumber = binaryToInt(binaryNumber);
    std::cout << "Binary: " << binaryNumber << " -> Integer: " << decimalNumber << std::endl;

    // Integer to Binary conversion
    int number = 42;
    std::string binaryRepresentation = intToBinary(number);
    std::cout << "Integer: " << number << " -> Binary: " << binaryRepresentation << std::endl;

    return 0;
}

std::string divisionAlgorithm(int dividend, int divisor, int resultSize) {
    std::string quotient;
    std::string remainder;

    // Perform the division algorithm
    for (int i = 0; i < resultSize; ++i) {
        dividend <<= 1;  // Left shift the dividend by 1 bit

        if (dividend >= divisor) {
            dividend -= divisor;
            quotient += '1';
        } else {
            quotient += '0';
        }
    }

    remainder = std::bitset<32>(dividend).to_string(); // Convert remainder to binary string

    // Pad quotient and remainder with leading zeros if necessary
    while (quotient.length() < resultSize) {
        quotient = '0' + quotient;
    }
    while (remainder.length() < resultSize) {
        remainder = '0' + remainder;
    }

    return quotient + remainder;
}

/* int main() { */
/*     int dividend = 27; */
/*     int divisor = 5; */
/*     int resultSize = 8; */

/*     std::string result = divisionAlgorithm(dividend, divisor, resultSize); */
/*     std::cout << "Quotient: " << result.substr(0, resultSize) << std::endl; */
/*     std::cout << "Remainder: " << result.substr(resultSize) << std::endl; */

/*     return 0; */
/* } */
