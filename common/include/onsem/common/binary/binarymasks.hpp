#ifndef ONSEM_COMMON_SAVER_BINARYMASKS_HPP
#define ONSEM_COMMON_SAVER_BINARYMASKS_HPP

#include <fstream>

namespace onsem {
namespace binarymasks {
static const unsigned char mask0{0x1};                       // hex for 0000 0001
static const unsigned char mask1{0x2};                       // hex for 0000 0010
static const unsigned char mask2{0x4};                       // hex for 0000 0100
static const unsigned char mask3{0x8};                       // hex for 0000 1000
static const unsigned char mask4{0x10};                      // hex for 0001 0000
static const unsigned char mask5{0x20};                      // hex for 0010 0000
static const unsigned char mask6{0x40};                      // hex for 0100 0000
static const unsigned char mask7{0x80};                      // hex for 1000 0000
static const unsigned char mask0To1{mask0 | mask1};          // hex for 0000 0011
static const unsigned char mask0To2{mask0To1 | mask2};       // hex for 0000 0111
static const unsigned char mask0To3{mask0To2 | mask3};       // hex for 0000 1111
static const unsigned char mask0To4{mask0To3 | mask4};       // hex for 0001 1111
static const unsigned char mask0To5{mask0To4 | mask5};       // hex for 0011 1111
static const unsigned char mask0To6{mask0To5 | mask6};       // hex for 0111 1111
static const unsigned char mask0To7{mask0To6 | mask7};       // hex for 1111 1111
static const unsigned char mask1To2{mask1 | mask2};          // hex for 0000 0110
static const unsigned char mask2To3{mask2 | mask3};          // hex for 0000 1100
static const unsigned char mask4To5{mask4 | mask5};          // hex for 0011 0000
static const unsigned char mask6To7{mask6 | mask7};          // hex for 1100 0000
static const unsigned char mask5To7{mask5 | mask6To7};       // hex for 1110 0000
static const unsigned char mask4To7{mask4 | mask5To7};       // hex for 1111 0000
static const unsigned char mask3To7{mask3 | mask4To7};       // hex for 1111 1000
static const unsigned char mask2To7{mask2 | mask3To7};       // hex for 1111 1100
static const unsigned char mask1To7{mask1 | mask2To7};       // hex for 1111 1110
static const unsigned char maskNot1{mask0 | mask2To7};       // hex for 1111 1101
static const unsigned char maskNot2{mask0To1 | mask3To7};    // hex for 1111 1011
static const unsigned char maskNot3{mask0To2 | mask4To7};    // hex for 1111 0111
static const unsigned char maskNot4{mask0To3 | mask5To7};    // hex for 1110 1111
static const unsigned char maskNot5{mask0To4 | mask6To7};    // hex for 1101 1111
static const unsigned char maskNot6{mask0To5 | mask7};       // hex for 1011 1111

/// Union that hold a binary database pointer.
union Ptr {
    void* ptr;
    signed char* pchar;
    unsigned char* puchar;
    const unsigned char* pcuchar;
    int* pint;
    uint64_t* puint64;
    unsigned short* pushort;
    std::size_t val;
    /// Constructor.
    Ptr() { ptr = nullptr; }
    /**
     * @brief Constructor.
     * @param p The pointer to initialize the union.
     */
    Ptr(void* p) { ptr = p; }
    /**
     * @brief Sum a pointer and an offset.
     * @param b The offset.
     * @return Result of the sum.
     */
    Ptr operator+(std::size_t b) const { return pchar + b; }
};

}    // End of namespace binarysaver
}    // End of namespace onsem

#endif    // ONSEM_COMMON_SAVER_BINARYSAVER_HPP
