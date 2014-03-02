#ifndef HAVE_UTIL_HONESTLY_H
#define HAVE_UTIL_HONESTLY_H

#include <inttypes.h>
#include <math.h>
#include <stdlib.h>

 // These are things that I couldn't fit elsewhere.  Many of these are things
 //  that the language really ought to have built in.

#define INIT_SAFE(type, name, ...) type& name () { static type r __VA_ARGS__; return r; }
#define EXTERN_INIT_SAFE(type, name) type& name ();

#define STRINGIFY(v) #v
#define _COMMA ,
#define CE constexpr
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint32 uint;

#define WARN(...) fprintf(stderr, __VA_ARGS__)
#define ANON_HELPER_1(a, b) a##b
#define ANON_HELPER_2(a, b) ANON_HELPER_1(a, b)
#define ANON ANON_HELPER_2(_anon_, __COUNTER__)

 // Constants
CE float FR = 1/60.0;
CE float PX = 1/16.0;
CE float PI = M_PI;
 // Can't be a CE because apparently division by 0 is not constant?
#ifndef INF
static const float INF = 1.f/0.f;
#endif

static float frand () {
    return rand() * 1.f / RAND_MAX;
}

#endif
