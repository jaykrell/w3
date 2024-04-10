
template <uint32_t N> struct uintLEn_to_native_exact;
template <uint32_t N> struct uintLEn_to_native_fast;

template <> struct uintLEn_to_native_exact<16> { typedef uint16_t T; };
template <> struct uintLEn_to_native_exact<32> { typedef uint32_t T; };
template <> struct uintLEn_to_native_exact<64> { typedef uint64_t T; };
template <> struct uintLEn_to_native_fast<16> { typedef uint32_t T; };
template <> struct uintLEn_to_native_fast<32> { typedef uint32_t T; };
template <> struct uintLEn_to_native_fast<64> { typedef uint64_t T; };

template <uint32_t N>
struct uintLEn // unsigned little endian integer, size n bits
{
    union
    {
        typename uintLEn_to_native_exact<N>::T native;
        unsigned char data [N / 8];
    };

    operator typename uintLEn_to_native_fast<N>::T ()
    {
#if BYTE_ORDER == LITTLE_ENDIAN
        return native;
#else
        typename uintLEn_to_native_fast<N>::T a = 0;
        for (uint32_t i = N / 8; i; )
            a = (a << 8) | data [--i];
        return a;
#endif
    }
    void operator= (uint32_t);
};
