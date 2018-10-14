# g5 : it works in progress, stay tuned~
[![BCH compliance](https://bettercodehub.com/edge/badge/racaljk/g5?branch=master)](https://bettercodehub.com/)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/7a2ba9735d27408f8ca617cb0a0b9a05)](https://www.codacy.com/app/racaljk/g5?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=racaljk/g5&amp;utm_campaign=Badge_Grade)
 [![Build Status](https://travis-ci.org/racaljk/g5.svg?branch=master)](https://travis-ci.org/racaljk/g5)

Implement a **g**olang compiler and minimal runtime environment within **5** functions. Inspired by [c4](https://github.com/rswier/c4) project. To compile it, we need a modern compiler, that is, it should support cpp17 language standard.(Recommend to use msvc2017 or gcc7.0+/clang5.0+).

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