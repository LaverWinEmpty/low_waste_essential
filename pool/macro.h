#ifdef __cplusplus
#    if(__cplusplus >= 201103L)
#        define CPP11 1
#    endif
#    if(__cplusplus >= 201402L)
#        define CPP14 1
#    endif
#    if(__cplusplus >= 201703L)
#        define CPP17 1
#    endif
#    if(__cplusplus >= 202002L)
#        define CPP20 1
#    endif
#endif

#ifndef CONST_IF
#    ifdef CPP17
#        define CONST_IF if constexpr
#    else
#        define CONST_IF if
#    endif
#endif

#ifndef STATIC_FIELD
#    ifdef CPP17
#        defein STATIC_FIELD inline static
#    else
#        define STATIC_FIELD static
#    endif
#endif