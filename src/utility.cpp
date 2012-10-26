
#ifdef HEADER

#define STRINGIFY(v) #v
#define CE constexpr
typedef const char* CStr;
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

 // Constants
const float FPS = 10.0;
const float FR = 1/10.0;
const float PX = 1/16.0;
const float PI = M_PI;

#endif

