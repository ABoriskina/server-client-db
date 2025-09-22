#pragma once
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <iomanip>
#include <sstream>

inline std::string generateSalt(size_t length = 16) {
    unsigned char* salt = new unsigned char[length];
    if (RAND_bytes(salt, length) != 1) {
        delete[] salt;
        throw std::runtime_error("Failed to generate salt");
    }
    
    std::stringstream ss;
    for (size_t i = 0; i < length; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)salt[i];
    }
    
    delete[] salt;
    return ss.str();
}

inline std::string hashPassword(const std::string& password, const std::string& salt) {
    std::string combined = salt + password;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, combined.c_str(), combined.size());
    SHA256_Final(hash, &sha256);
    
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    
    return ss.str();
}
