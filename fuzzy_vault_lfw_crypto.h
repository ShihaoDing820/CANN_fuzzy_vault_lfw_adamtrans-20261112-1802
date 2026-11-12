#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

constexpr std::size_t kFuzzyVaultLfwCoefficientCount = 8;
constexpr std::size_t kFuzzyVaultLfwDigestBytes = 32;
using FuzzyVaultLfwCoefficients =
    std::array<std::int32_t, kFuzzyVaultLfwCoefficientCount>;
using FuzzyVaultLfwDigest =
    std::array<std::uint8_t, kFuzzyVaultLfwDigestBytes>;

FuzzyVaultLfwDigest FuzzyVaultLfwHashCoefficients(
    const FuzzyVaultLfwCoefficients& coefficients);

bool FuzzyVaultLfwDigestEqual(
    const FuzzyVaultLfwDigest& left,
    const FuzzyVaultLfwDigest& right);

