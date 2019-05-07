if(WITH_XC_CRYPTO_SSH)
    register_plugin(crypto_ssh)

    set(crypto_ssh_SOURCES
        ssh/bcrypt_pbkdf.cpp
        ssh/blowfish.c
        ssh/ASN1Key.cpp
        ssh/BinaryStream.cpp
        ssh/OpenSSHKey.cpp)

    add_library(crypto_ssh STATIC ${crypto_ssh_SOURCES})
    target_include_directories(crypto_ssh PRIVATE ssh)
    target_link_libraries(crypto_ssh Qt5::Core ${GCRYPT_LIBRARIES})
endif()
