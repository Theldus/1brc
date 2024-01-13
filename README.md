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

There are 9 attempts/code versions, which you can check in the commit history (also tagged from `v1` to `v9` for easier checkout).

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
