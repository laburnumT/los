#ifndef LOS_LIB__HELPER_H
#define LOS_LIB__HELPER_H

#define __MACRO_PLACEHOLDER_1 1,

#define __lambda_false(__x, y, ...) y

#define __is_defined(x) ___is_defined(x)
#define ___is_defined(val) ____is_defined(__MACRO_PLACEHOLDER_##val)
#define ____is_defined(arg1_opt) __lambda_false(arg1_opt 1, 0)

#if __has_attribute(flag_enum)
        #define FLAG_ENUM __attribute__((flag_enum))
#else
        #define FLAG_ENUM
#endif

#endif
