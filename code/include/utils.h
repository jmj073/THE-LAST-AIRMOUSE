#ifndef _UTILS_H_
#define _UTILS_H_

#if __cplusplus < 201700
namespace std {
    template<typename _Tp>
    constexpr const _Tp&
    clamp(const _Tp& __val, const _Tp& __lo, const _Tp& __hi)
    {
        __glibcxx_assert(!(__hi < __lo));
        return (__val < __lo) ? __lo : (__hi < __val) ? __hi : __val;
    }
}
#else
#include <algorithm>
#endif

#endif /* _UTILS_H_ */