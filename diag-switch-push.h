#if _MSC_VER >= 1200
#pragma warning (push)
#pragma warning (disable:5031) // unbalanced push/pop
#endif

#if __GNUC__ || __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
#endif

#if _MSC_VER
#pragma warning (disable:4061) // unhandled case
#pragma warning (disable:4062) // unhandled case
#endif
