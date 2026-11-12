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
target_compile_definitions(fuzzy_vault_lfw_adamtrans_test PRIVATE
    WIFAKEY_BG2_PATH="${CMAKE_CURRENT_SOURCE_DIR}/wifakey_bg2_set2.tsv"
)
target_link_libraries(fuzzy_vault_lfw_adamtrans_test PRIVATE OpenSSL::Crypto)

add_executable(fuzzy_vault_lfw_full
    fuzzy_vault_lfw_adamtrans.asc
    fuzzy_vault_lfw_crypto.cpp
)

target_compile_features(fuzzy_vault_lfw_full PRIVATE cxx_std_17)
target_compile_options(fuzzy_vault_lfw_full PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${CMAKE_ASC_ARCHITECTURES}>
)
target_compile_definitions(fuzzy_vault_lfw_full PRIVATE
    FUZZY_VAULT_FULL=1
    WIFAKEY_BG2_PATH="${CMAKE_CURRENT_SOURCE_DIR}/wifakey_bg2_set2.tsv"
)
target_link_libraries(fuzzy_vault_lfw_full PRIVATE OpenSSL::Crypto)

