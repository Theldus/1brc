# 1brc
This repository contains all my attempts to solve the [1brc (The One Billion Row Challenge)] in C.

## So, what is this 1brc?
The 1brc is a competition that started on January 1, 2024, and runs until January 31 of the same year. Its purpose is to efficiently process 1 billion lines of a text file. Specifically, the challenge consists of parsing a text file containing weather station readings, where each line represents a reading from a specific station.

Each line in the file includes the station's name (encoded in UTF-8 and up to 100 characters) and the temperature reading, expressed with a fractional digit in the format `X.Y, XY.Z, -X.Y, -XY.Z`. Each entry is separated by a semicolon, like the input below:
```text
Hamburg;12.0
Bulawayo;8.9
Palembang;38.8
St. John's;15.2
Abha;67.8
Cracow;12.6
Clacöw;12.6
Cwacpw;12.6
Bridgetown;26.9
Istanbul;6.2
Abidjan;10.5
Roseau;34.4
Abecha;4.5
Conakry;31.2
Abéché;-5.6
Istanbul;23.0
Accra;-9.8
```

The challenge's objective is to calculate the minimum, average, and maximum values for all station readings, displaying the results sorted alphabetically by the station's name on the stdout. The result should follow the format below:
```text
{Abecha=4.5/4.5/4.5, Abha=67.8/67.8/67.8, Abidjan=10.5/10.5/10.5, Abéché=-5.6/-5.6/-5.6, Accra=-9.8/-9.8/-9.8, Bridgetown=26.9/26.9/26.9, Bulawayo=8.9/8.9/8.9, Clacöw=12.6/12.6/12.6, Conakry=31.2/31.2/31.2, Cracow=12.6/12.6/12.6, Cwacpw=12.6/12.6/12.6, Hamburg=12.0/12.0/12.0, Istanbul=6.2/14.6/23.0, Palembang=38.8/38.8/38.8, Roseau=34.4/34.4/34.4, St. John's=15.2/15.2/15.2}
```

Originally proposed in Java, the challenge has attracted interest for development in various other languages, as evident in the [Discussions] section of the repository. While official submissions are restricted to Java, unofficial participation is also encouraged. For more specific details about the challenge and its rules, refer to the [official 1brc repository].

## My results
All the tests were performed on an i5 7300HQ with 8GB of RAM, running Slackware 14.2-current (15-ish), with GCC 9.3 and OpenJDK 21.0.1. It’s important to note that due to memory constraints (the 1B rows file is 12GB!), the following results were obtained with an input of 400M rows (5.3GB), but these should scale proportionally for larger inputs.

There are 11 attempts/code versions, which you can check in the commit history (also tagged from `v1` to `v11` for easier checkout).

The results:
|            **Version**           | **Time (s) - 400M rows** | **Speedup (baseline)** | **Commit** |      **Small notes**      |
|:--------------------------------:|:------------------------:|:----------------------:|:----------:|:-------------------------:|
| CalculateAverage.java (baseline) |          ~100.3s         |           --           |  [2155286] |             --            |
| CalculateAverage_royvanrijn.java |           ~4.2s          |         23.88x         |  [e665d71] |             --            |
|                v1                |          ~367.5s         |   **~3.66x slowdown**  |  [105320f] |        No hashtable       |
|                v2                |          ~47.6s          |          2.10x         |  [866ee85] |       Add hashtable       |
|                v3                |          ~15.1s          |          6.61x         |  [5420d9d] |    Temperatures as ints   |
|                v4                |          ~11.38s         |          8.81x         |  [b95ba11] | Remove collision handling |
|                v5                |          ~72.3s          |          1.38x         |  [5bea85b] |   Initial thread support  |
|                v6                |          ~3.135s         |         32.04x         |  [8fba1ae] |    Rework threads code    |
|                v7                |          ~2.855s         |         35.13x         |  [ffdf7d0] | Remove redundant memchr() |
|                v8                |          ~2.51s          |         39.96x         |  [718e9b3] |  Branchless temp parsing  |
|                v9                |          ~2.26s          |         44.38x         |  [db8d462] |  AVX2 routine to find ';' |
|                v10               |          ~2.17s          |         46.22x         |  [fa1debb] |  Remove MAP_POPULATE flag |
|                v11               |          ~1.54s          |         65.12x         |  [3b9466f] |  New hash function        |

[1brc (The One Billion Row Challenge)]: https://github.com/gunnarmorling/1brc
[official 1brc repository]: https://github.com/gunnarmorling/1brc
[Discussions]: https://github.com/gunnarmorling/1brc/discussions

[2155286]: https://github.com/gunnarmorling/1brc/commit/2155286d7a02ee3b92596d8905607277a0bcfbe7
[e665d71]: https://github.com/gunnarmorling/1brc/commit/e665d715499b2f2cac765cd3314c948a56602061

[105320f]: https://github.com/Theldus/1brc/commit/105320f71adc27699078db9474d7562343458b71
[866ee85]: https://github.com/Theldus/1brc/commit/866ee8566657f06f93745a93dbc1724c5728e4ff
[5420d9d]: https://github.com/Theldus/1brc/commit/5420d9d05e933a65bb3474282433f49488af43e7
[b95ba11]: https://github.com/Theldus/1brc/commit/b95ba11c457ecc00b7f47ec1480f1a52a0219777
[5bea85b]: https://github.com/Theldus/1brc/commit/5bea85b52453c70ccf2edeb7e7aff863cfcaec77
[8fba1ae]: https://github.com/Theldus/1brc/commit/8fba1ae8cd27cd606e05cba4e964f6fbb574f1cd
[ffdf7d0]: https://github.com/Theldus/1brc/commit/ffdf7d09eac0d51ac95a382365aca4ed1c25f065
[718e9b3]: https://github.com/Theldus/1brc/commit/718e9b314a918a3a4d1ee19fca93223890dbd13c
[db8d462]: https://github.com/Theldus/1brc/commit/db8d462179fb5a6f7bb402916673c3eb6f680473
[fa1debb]: https://github.com/Theldus/1brc/commit/fa1debb5abc7ada841021399a231e18555f0f623
[3b9466f]: https://github.com/Theldus/1brc/commit/3b9466f519eef5e204997964179ba0ead142c362

## ChangeLog

Below is an explanation of each version created for the challenge and the thought process behind each of them.

### v1
This is the initial and simplest version of all. I just wanted to create a quick-and-dirty solution to the problem, with no optimizations and without even using hash tables. I believe it's a good starting point for my own baseline and also as a curiosity about what would be the 'worst' possible implementation for the problem, even losing to the Java baseline.

---

### v2
Adds a fairly simple implementation of a hash table with open addressing (using SDBM as the hash function) and is finally ~8x faster than v1 and twice as fast as the Java baseline. The real challenge truly begins from here.

---

### v3
Removes the use of the `strtof()` function and manually parses temperatures as integers. This is easily possible given the challenge's conditions, where numbers have only one fractional digit. Surprisingly, v3 brought a 3x speedup over v2. Apparently, `strtof()` adds *a lot* of overhead to the code.

---

### v4
Some minor changes such as inline functions and `likely()/unlikely()`, but the most significant change was in the hash table: since it proved to have no collisions, I completely removed the open addressing and string comparisons, which added significant overhead to execution. Some may consider this 'cheating' since there might be collisions for different inputs, and I don't blame them, but the following versions will continue with this applied change. These changes gave us a 1.32x speedup over v3.

---

### v5
Initial (and intentionally poor) version of threads. I am not taking on this challenge to compete with anyone or racing against time, so I also like to explore intentionally bad solutions (like v1) to see how bad things can get.

In this version, the hash table is shared among all threads and protected by a mutex, adding a huge performance penalty. Specifically, this version is ~6.17x slower than v4.

---

### v6
Correctly implemented threads! In this version, each thread has its own individual hash table, eliminating race conditions between threads and the need for locks. After all threads finish, the results are merged into a single hash table and then sorted by the station's name. The merging and sorting process is done in a single thread since it is fast and gains little benefit from multi-threading.

In addition to fixing the thread implementation, this version also unrolls the loop for parsing temperatures, as literally a single if/else statement is sufficient for parsing. This alone resulted in an 8% speedup compared to the previous loop method.

Overall, this version is ~3.57x faster than the previous version on a quad-core CPU.

---

### v7
Removes an unnecessary `memchr()` used to find '\n' in parsing temperatures. The temperature parsing itself becomes predictable about where the newline character will be, so the function call can be safely removed. This brought us a 9% speedup over the previous code.

---

### v8
This version removes _all_ if statements in the `read_temperature()` function and replaces them with entirely branchless code, bringing a speedup of **~14%** over the previous version. Since this modification might not be immediately obvious (and I really **liked** it), I believe it deserves a more detailed explanation.

It is well-known, or almost, that branch instructions strongly impact CPU performance, and there are various hardware and software attempts to mitigate such issues, such as branch predictions and compilation hints, for the compiler to better organize if statements, etc. Nonetheless, this problem will always persist, and code that avoid the use of branches often benefit significantly.

The `read_temperature()` function until v7 looked like this:
```c
 1 static inline int
 2 read_temperature(const char *line, const char **ptr)
 3 {
 4     const char *p = line;
 5     int temp = 0;
 6     int sign = 1;
 7 
 8     if (*p == '-') {
 9         sign = -1;
10         p++;
11     }
12 
13     /* Numbers of format:
14      *    012345
15      *    X.Y
16      *    XY.Z
17      */
18     if (p[1] == '.') {
19         temp = ((p[0] - '0') * 10) + (p[2] - '0');
20         p += 3;
21     }
22 
23     /* Numbers of format: XY.Z */
24     else {
25         temp = ((p[0] - '0') * 100) +
26             ((p[1] - '0') * 10) + (p[3] - '0');
27         p += 4;
28     }
29 
30     *ptr  = p;
31     temp *= sign;
32     return (temp);
33 }
```

Surprisingly, these two innocent if statements were weighing heavily on the code. But how to remove them? The answer is: lookup tables!

Take the conditional increment of the pointer `p` as an example:
```c
if (p[1] == '.')
    p += 3;
else
    p += 4;
```

This could be translated into a simple lookup table like:
```c
static char p_incr[256] = {
  ['.'] = 3,
  /* everything else: 4 */
};
```

and used as:
```c
p += p_incr[ p[1] ];
```

That is, the character at position '.' (or 46) in our table contains the value 3, and all other positions contain 4. By using `p[1]` as the index for this table, the table conditionally returns two distinct values based on the character of `p[1]`. Effectively, we have an if/else without conditional instructions =).

The same idea can be repeated for the rest of the code as long as the tables are carefully constructed, which brings us to the final code:
```c
1 #define lookup_char(type,name,first_char,vfirst_char,vsecond_char) \
2     static type name[256] = {\
3         [first_char]  = vfirst_char,\
4         ['0'] = vsecond_char, ['1'] = vsecond_char, ['2'] = vsecond_char,\
5         ['3'] = vsecond_char, ['4'] = vsecond_char, ['5'] = vsecond_char,\
6         ['6'] = vsecond_char, ['7'] = vsecond_char, ['8'] = vsecond_char,\
7         ['9'] = vsecond_char}
8 
9 static inline int
10 read_temperature(const char *line, const char **ptr)
11 {
12     const char *p = line;
13     int temp = 0;
14 
15     lookup_char(int8_t,  sign,      '-', -1, 1);
16     lookup_char(uint8_t, first_inc, '-',  1, 0);
17     lookup_char(uint8_t, m1,        '.', 10, 100);
18     lookup_char(uint8_t, nn1,       '.',  2, 1);
19     lookup_char(uint8_t, m2,        '.',  1, 10);
20     lookup_char(uint8_t, nn2,       '.',  0, 3);
21     lookup_char(uint8_t, m3,        '.',  0, 1);
22     lookup_char(uint8_t, incr,      '.',  3, 4);
23 
24     p += first_inc[p[0]];
25 
26     temp =
27         (p[0]-'0')*m1[p[1]] +
28         (p[nn1[p[1]]]-'0')*m2[p[1]] +
29         (p[nn2[p[1]]]-'0')*m3[p[1]];
30 
31     p += incr[p[1]];
32 
33     *ptr  = p;
34     temp *= sign[line[0]];
35     return (temp);
36 }
```

The tables above, built with macros for code organization, should not be complicated to understand: the first character is what will be 'compared', the first number (`vfirst_char`) is the return if `true`, and `vsecond_char` is the return if `false`. For example, the first macro (`sign`) directly translates to the following ternary expression: `(c == '-') ? -1 : 1`, and the idea repeats for the others.

Thus, the tables serve to, according to the character at `p[1]`, decide which is the next character to be read or multiplied (`nn1 = next number 1`, `m1 = multiply 1`, etc.).

Note that in `temp = ...`, there is an additional third multiplication, and the reason for it is quite simple: if `p[1] == '.'`, the third addition should not occur. In this case, multiplication is by 0 and invalidates the sum; otherwise, it multiplies by 1, and the sum is performed. Clever, isn't it?

---

### v9
This version introduces the `mchar()` function for an extremely optimized search for `';'`, implemented in SIMD/AVX2, replacing the traditional `memchr()`. But before proceeding, let me address the first probable question:
> Q: 'GNU libc's `memchr()` is heavily optimized and also uses SIMD, why reinvent the wheel?'
> 
> A: Because it was necessary, you will understand.

Allow me to briefly walk through how a possible implementation of `memchr()` works in SIMD (and possibly in Glibc) and the problems associated with it.

The `memchr()` function has the following signature:
```c
#include <string.h>
void *memchr(const void *s, int c, size_t n);
```

That is, it takes a buffer, the character to be searched, and the size of that buffer. If found, the function returns a pointer to the character; otherwise, it returns NULL. Simple enough, but this _function signature_ __hurts__ the SIMD implementation! Let's see why.

#### How would a basic implementation of `memchr()` in SIMD look like?
Thinking about AVX2 (or even SSE2), only a few instructions are needed to build an efficient character searcher. Take the following input of 32 bytes/characters as an example:
```text
123;45678;9123456;7890123;123456
```

The `vpcmpeqb` instruction (or `_mm256_cmpeq_epi8()`) compares the contents of 2 YMM registers, and for each equal byte, it saves `0xFF` in the corresponding byte of the destination register.

Assuming the following C code:
```c
char mask_vec[32];
memset(mask_vec, ';', 32);

__m256i mask_semic   = _mm256_loadu_si256((const __m256i*)mask_vec);
__m256i memory       = _mm256_loadu_si256((const __m256i*)s);
__m256i cmp_semic_ff = _mm256_cmpeq_epi8(memory, mask_semic);
```

After the comparison, `cmp_semic_ff` will contain `0x00` in the bytes that do not match and `0xFF` in the bytes that match. Do you see where this is going? 32 bytes were compared at once!

But that alone is not enough; we need a way to obtain the respective positions for a regular variable that can be read normally later. This can be done with the `vpmovmskb` instruction (or `_mm256_movemask_epi8()`), which reads the most significant bit of each byte of a YMM and saves this bit in a normal 32-bit register. And that's exactly what we want:
```c
int mask = _mm256_movemask_epi8(cmp_semic_ff);
```

The initial text after comparisons and the movemask turns into the following 32-bit number: `0x2020208` or in binary: `0000 0010 0000 0010 0000 0010 0000 1000`, or if we reverse the order for clarity: `0001 0000 0100 0000 0100 0000 0100 0000`. Notice that each '1' bit in our final number corresponds to the positions found, and '0' bits are non-matches!

If there were a way to retrieve the position of these bits without a loop... there is! The `tzcnt` (Count the Number of Trailing Zero Bits) instruction, like GCC's `__builtin_ffs() - Find First Set`, does exactly that: for a given register, it returns the number of zeros before the first '1', or in a simplified way, the position of the first '1' bit!

With all that said, `__builtin_ffs(mask)` returns '4', precisely the first position of the `';'` found in the string. Amazing, right?

#### Why all this explanation?
Simply to say two things:
- SIMD instructions _read_ multiple bytes at once from memory.
- SIMD instructions _can find_ the desired character in multiple positions at once!

It may seem obvious to state this, but that is the biggest advantage of using SIMD, and also its Achilles' heel for this function.

A traditional use of `memchr()` to find all the `';'` in the above string would be something like:
```c
char *s = "123;45678;9123456;7890123;123456";

char *prev = s;
char *next;

size_t len = strlen(s);
while ((next = memchr(prev, ';', len)) != NULL) {
	len  -= (next - prev + 1);
	prev  = (next + 1);
}
```

Note, however:
- Although `memchr()` can find all `;` in this string at once, it is _required_ to return at the first found occurrence.
- Subsequent calls force `memchr()` to analyze the same memory it had already read!

Worse yet, consider the case where the search string is: `123;4567891234567891234567891234`. Although `memchr()` _knows_ that the next 28 characters do not have `;`, the user does not know (and could not know!), and a second invocation of `memchr()` again forces the routine to read 28 (out of 32) bytes of memory, entirely unnecessarily.

**In summary**: the interface/signature of the `memchr()` function forces it to have suboptimal performance and discard values that the function already knew in advance, leading the routine to read the same portion of memory repeatedly!

#### Introducing `mchar()`
The `mchar()` created for v9 has the following function signature:
```c
struct mchar_ctx {
	uint32_t cmask;
	char *cptr;
	char *prev_ptr;
};

char *mchar(const char *ptr, size_t rem_bytes, struct mchar_ctx *ctx);
```

And what does this change? Everything! Unlike `memchr()`, `mchar()` has a context structure where it stores the data obtained from a previous call. The `cmask` variable stores the mask of the last read, so if the function finds more than one occurrence of the searched term, it _does not_ search in memory but only returns the offset of that byte, which it already knows. The `cptr` variable stores the last read position in memory, so even if it finds nothing, the function will not repeat the search in repeated memory addresses.

With these two features, `mchar()` _guarantees_ that it will not search the same memory segment repeatedly, and multiple results found are cached and returned in future invocations without additional memory reads.

It is important to note that `mchar()` works well with the 'cache' concept only for memory blocks that have no changes between function invocations; this is a prerequisite. For memory blocks that may have changes, the function should not assume anything about the already-read memory portion, and there is no better alternative than the traditional `memchr()`.

---

### v10
Ironically, removing the `MAP_POPULATE` flag from `mmap()` slightly improved performance by 4%... I can't say exactly why, if anyone has any ideas...

Edit:
There is some discussion about it on issue [v10 MAP_POPULATE speed up possibile explanation], you might want to take a look.

[v10 MAP_POPULATE speed up possibile explanation]: https://github.com/Theldus/1brc/issues/1

---

### v11
These days I was watching the amazing video: ["Faster than Rust and C++: the PERFECT hash table"], and among the many interesting insights, one caught my attention: "know your data" and "there are no purely random data". The Achilles' heel of every hash table is being generic, but if you know your data beforehand...

Since `v4`, my code no longer handles collisions, and therefore, it only works for a fixed set of station names. Considering this, what harm would it do to rethink the hash function and try to create one that is lightweight and specifically tailored to our dataset?

Based on the ideas from the video, I came up with the following code below:
```c
static inline uint64_t
sugoi_hash(const char *l, size_t len)
{
    uint64_t hash = 0;
    if (likely(len >= 5)) {
        hash |= ((uint64_t)l[0] & 0x1F) << 40;
        hash |= ((uint64_t)l[1] & 0x1F) << 32;
        hash |= ((uint64_t)l[2] & 0x1F) << 24;
        hash |= ((uint64_t)l[3] & 0x1F) << 16;
        hash |= ((uint64_t)l[len-2] & 0x1F) << 8;
        hash |= ((uint64_t)l[len-1] & 0x1F) << 0;
    } else {
        hash |= ((uint64_t)l[0] & 0x1F) << 24;
        hash |= ((uint64_t)l[1] & 0x1F) << 16;
        hash |= ((uint64_t)l[len-2] & 0x1F) << 8;
        hash |= ((uint64_t)l[len-1] & 0x1F) << 0;
    }
    return (hash);
}
```

The code is quite simple: according to the length of the key, it takes the first and last characters of each station name and assembles a 64-bit value from that.
The `& 0x1F` gets the character position in the ascii table... this isn't really necessary, but for some reason it helped remove the collisions.

This simple code brought a speedup of 40%!!!: I imagine that the code running in (almost) constant time and without loops helped the CPU a lot.

["Faster than Rust and C++: the PERFECT hash table"]: https://www.youtube.com/watch?v=DMQ_HcNSOAI&ab_channel=strager

## Final Thoughts
I enjoyed the challenge _a lot_ and had a lot of fun in the process. It is always interesting to see how optimization opportunities can hide in seemingly harmless places and how much they cost in the final code.

Some may say that challenges like this have no use for real-world code, but I strongly disagree. Challenges like this make us think outside the box and that reflects directly on any code in the future. I’m not talking about premature optimizations, but about having a critical sense of the code that is written.

Moreover, I learned a lot during the process, which is why I created this repository and tried to document all the changes I made. Anyway, I highly recommend it =).
