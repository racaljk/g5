# g5 : golang compiler and runtime in 5 named functions
| MSVC2017 | g++7.3.0 | Code alert | Code quality |
| :--------: | :--------: |  :--------: | :--------: |
| [![Build status](https://ci.appveyor.com/api/projects/status/61b9imkcd1ibi3gt?svg=true)](https://ci.appveyor.com/project/racaljk/g5) | [ ![Build Status](https://travis-ci.org/racaljk/g5.svg?branch=master)](https://travis-ci.org/racaljk/g5) | [![Total alerts](https://img.shields.io/lgtm/alerts/g/racaljk/g5.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/racaljk/g5/alerts/) | [![Codacy Badge](https://api.codacy.com/project/badge/Grade/7a2ba9735d27408f8ca617cb0a0b9a05)](https://www.codacy.com/app/racaljk/g5?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=racaljk/g5&amp;utm_campaign=Badge_Grade) |

 it works in progress, stay tuned~

Implement a **g**olang compiler and minimal runtime environment within **5** functions. Inspired by [c4](https://github.com/rswier/c4) project. To compile it, we need a modern compiler, that is, it should support cpp17 language standard.(Recommend to use msvc2017 or gcc7.0+).

*ps: code commit might be frequent, you can star it rather than watching.*

# 5 is all
+ **next()** lexer
+ **parse()** parser
+ **runtime()** create runtime environment
+ **emit()** assembly IR generator
+ **main()** launcher


# appendix
> [The Go Programming Language Specification](https://golang.org/ref/spec)

> [c++17 quick overview](https://github.com/changkun/modern-cpp-tutorial)
