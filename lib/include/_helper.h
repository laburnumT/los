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

#define max(x, y) (x) > (y) ? (x) : (y)
#define min(x, y) (x) < (y) ? (x) : (y)

#endif
