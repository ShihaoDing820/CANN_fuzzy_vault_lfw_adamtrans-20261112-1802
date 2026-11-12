#include "fuzzy_vault_lfw_crypto.h"

#include <openssl/crypto.h>
#include <openssl/sha.h>

#include <array>
#include <stdexcept>

FuzzyVaultLfwDigest FuzzyVaultLfwHashCoefficients(
    const FuzzyVaultLfwCoefficients& coefficients)
{
    std::array<std::uint8_t, kFuzzyVaultLfwCoefficientCount * 4> serialized{};
    for (std::size_t index = 0; index < coefficients.size(); ++index) {
        const std::uint32_t value = static_cast<std::uint32_t>(coefficients[index]);
        serialized[index * 4 + 0] = static_cast<std::uint8_t>(value & 0xffU);
        serialized[index * 4 + 1] = static_cast<std::uint8_t>((value >> 8U) & 0xffU);
        serialized[index * 4 + 2] = static_cast<std::uint8_t>((value >> 16U) & 0xffU);
        serialized[index * 4 + 3] = static_cast<std::uint8_t>((value >> 24U) & 0xffU);
    }
    FuzzyVaultLfwDigest digest{};
    if (SHA256(serialized.data(), serialized.size(), digest.data()) == nullptr) {
        throw std::runtime_error("SHA-256 failed");
    }
    return digest;
}

bool FuzzyVaultLfwDigestEqual(
    const FuzzyVaultLfwDigest& left,
    const FuzzyVaultLfwDigest& right)
{
    return CRYPTO_memcmp(left.data(), right.data(), left.size()) == 0;
}

