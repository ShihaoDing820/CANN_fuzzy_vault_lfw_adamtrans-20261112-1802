cmake_minimum_required(VERSION 3.16)

find_package(ASC REQUIRED)
project(fuzzy_vault_lfw_adamtrans_cann LANGUAGES ASC CXX)

find_package(OpenSSL REQUIRED)

add_executable(fuzzy_vault_lfw_adamtrans_test
    fuzzy_vault_lfw_adamtrans.asc
    fuzzy_vault_lfw_crypto.cpp
)

target_compile_features(fuzzy_vault_lfw_adamtrans_test PRIVATE cxx_std_17)
target_compile_options(fuzzy_vault_lfw_adamtrans_test PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${CMAKE_ASC_ARCHITECTURES}>
)
target_link_libraries(fuzzy_vault_lfw_adamtrans_test PRIVATE OpenSSL::Crypto)

