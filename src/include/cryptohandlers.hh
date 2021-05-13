#pragma once
#ifndef _CRYPTOHANDLER_HH_
#define _CRYPTOHANDLER_HH_

#define BLOCK_SIZE 32
#define ENCODE_SIZE 256
#define DEFAULT_SALT_LENGTH 16

#include <string>
#include <vector>
#include <map>
#include <exception>

typedef class CryptoException : public std::runtime_error
{
public:
    explicit CryptoException(const std::string &information)
        : std::runtime_error(information), information(information.c_str())
    {
    }

    const char *what() const noexcept override { return information; }

protected:
    const char *information;

} CryptoException;

template <typename SIZE_T>
SIZE_T
getBlocks(const unsigned int &unit, const SIZE_T &len)
{
    SIZE_T blocks = len / unit;
    if (len > blocks * unit)
    {
        blocks++;
    }
    return blocks;
}

std::vector<unsigned char>
pad(std::vector<unsigned char> data, unsigned int unit);

std::vector<unsigned char>
unpad(std::vector<unsigned char> data);

/**
 * @note These two functions are based on Base64 encoding!
 */
std::string
encrypt_DET(const std::string &plaintext, const std::string &salt, const std::string &rawkey);

std::string
decrypt_DET(const std::string &ciphertext, const std::string &rawkey, const unsigned int &salt_length);

/**
 * @note Encryption for OPE will return a corresponding SQL statement, be careful.
 */
std::string
encrypt_OPE(const std::string &plaintext, const std::string &rawkey, const std::map<double, unsigned int> &local_table);

std::string
decrypt_OPE(const std::string, const std::string &rawkey, const unsigned int &salt_length);

#endif