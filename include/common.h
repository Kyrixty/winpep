#ifndef RCC_COMMON_H
#define RCC_COMMON_H
#include <stdint.h>
#include <stdbool.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef u8 b8;
typedef u32 b32;

#define KB(n) ((u64) (n) << 10)
#define MB(n) ((u64) (n) << 20)
#define GB(n) ((u64) (n) << 30)

#define MIN(a, b) (((a) < (b) ? (a) : (b)))
#define MAX(a, b) (((a) > (b) ? (a) : (b)))

#define WORD_SIZE (sizeof(void*))

#endif
