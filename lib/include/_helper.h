#ifndef LOS_LIB__HELPER_H
#define LOS_LIB__HELPER_H

#define __MACRO_PLACEHOLDER_1 1,

#define __lambda_false(__x, y, ...) y

#define __is_defined(x) ___is_defined(x)
#define ___is_defined(val) ____is_defined(__MACRO_PLACEHOLDER_##val)
#define ____is_defined(arg1_opt) __lambda_false(arg1_opt 1, 0)

// The flag_enum attribute doesn't work on gcc :(
// There is no real good way of checking for the gcc compiler, so this is
// checking for the minimum gcc compiler that supports the features we require,
// as currently clang seems to set __GNUC__ to 4
#if __has_attribute(flag_enum) && __GNUC__ < 13
        #define FLAG_ENUM __attribute__((flag_enum))
#else
        #define FLAG_ENUM
#endif

#if __has_include(<stddefer.h>)
        #include <stddefer.h>
        #if defined(__clang__)
                #if __is_identifier(_Defer)
                        #error                                                 \
                            "clang may need the option -fdefer-ts for the _Defer feature"
                #endif
        #endif
#elif __GNUC__ > 8
        #define defer _Defer
        #define _Defer _Defer_A(__COUNTER__)
        #define _Defer_A(N) _Defer_B(N)
        #define _Defer_B(N) _Defer_C(_Defer_func_##N, _Defer_var_##N)
        #define _Defer_C(F, V)                                                 \
                auto void F(int *);                                            \
                __attribute__((                                                \
                    __cleanup__(F), __deprecated__, __unused__)) int V;        \
                __attribute__((__always_inline__,                              \
                               __deprecated__,                                 \
                               __unused__)) inline auto void                   \
                F(__attribute__((__unused__)) int *V)
#else
        #error "The _Defer feature seems not available"
#endif

#define max(x, y) (x) > (y) ? (x) : (y)
#define min(x, y) (x) < (y) ? (x) : (y)

#endif
