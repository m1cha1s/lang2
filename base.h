#ifndef _BASE_H
#define _BASE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef size_t usize;
typedef ptrdiff_t ssize;

typedef float f32;
typedef double f64;

#define IsAlpha(x) (('a' <= x && x <= 'z') || ('A' <= x && x <= 'Z'))
#define IsNum(x) ('0' <= x && x <= '9')
#define IsWhitespace(x) (x == ' ' || x == '\n' || x == '\t')

#define STR_(s) #s
#define STR(s) STR_(s)
#define CONCAT_(a,b) a ## b
#define CONCAT(a,b) CONCAT_(a,b)

// Memory

#define KB(x) (x*1024)
#define MB(x) (KB(x)*1024)

#ifndef BASE_BLOCK_SIZE
#define BASE_BLOCK_SIZE KB(4)
#endif

typedef struct ArenaBlock ArenaBlock;
struct ArenaBlock {
    u8 *block;
    u64 size;
    u64 end;
    ArenaBlock *next;
};

typedef struct Arena {
    ArenaBlock *first;
    ArenaBlock *current;
} Arena;

// void ArenaInit(Arena *arena);
void arena_free(Arena *arena);

void *arena_alloc(Arena *arena, u64 size);
void arena_reset(Arena *arena);

// ----

typedef struct string {
     u8 *data;
     usize len;
} string;

#define S(x) ((string){.data=(u8*)(x), .len=(sizeof(x)-1)})

string string_view_from_bytes(u8 *cstr, usize len);
string string_from_bytes(Arena *arena, u8 *cstr, usize len);

string string_concat(Arena *arena, string a, string b);

#endif
