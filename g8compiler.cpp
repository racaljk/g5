//===----------------------------------------------------------------------===//
// golang specification can be found here: https://golang.org/ref/spec
//
// in development mode, i consider raise runtime_error when met error , they
// will be replaced with error stream later
//===----------------------------------------------------------------------===//

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>
#include <exception>
#include <fstream>
#include <functional>
#include <string>
#include <vector>

using namespace std;

string keywords[] = {"break",    "default",     "func",   "interface", "select",
                     "case",     "defer",       "go",     "map",       "struct",
                     "chan",     "else",        "goto",   "package",   "switch",
                     "const",    "fallthrough", "if",     "range",     "type",
                     "continue", "for",         "import", "return",    "var"};
enum Token {
    KW_break,KW_default,KW_func,KW_interface,KW_select,KW_case,KW_defer,KW_go,KW_map,KW_struct,KW_chan,KW_else,KW_goto,KW_package,KW_switch,
    KW_const,KW_fallthrough,KW_if,KW_range,KW_type,KW_continue,KW_for,KW_import,KW_return,KW_var,OP_ADD,OP_BITAND,OP_ADDASSIGN,OP_BITANDASSIGN,
    OP_AND,OP_EQ,OP_NE,OP_LPAREN,OP_RPAREN,OP_SUB,OP_BITOR,OP_SUBASSIGN,OP_BITORASSIGN,OP_OR,OP_LT,OP_LE,OP_LBRACKET,OP_RBRACKET,OP_MUL,OP_XOR,
    OP_MULASSIGN,OP_BITXORASSIGN,OP_CHAN,OP_GT,OP_GE,OP_LBRACE,OP_RBRACE,OP_DIV,OP_LSHIFT,OP_DIVASSIGN,OP_LSFTASSIGN,OP_INC,OP_ASSIGN,
    OP_SHORTASSIGN,OP_COMMA,OP_SEMI,OP_MOD,OP_RSHIFT,OP_MODASSIGN,OP_RSFTASSIGN,OP_DEC,OP_NOT,OP_VARIADIC,OP_DOT,OP_COLON,OP_ANDXOR,
    OP_ANDXORASSIGN,TK_ID,LITERAL_INT,LITERAL_FLOAT,LITERAL_IMG,LITERAL_RUNE,LITERAL_STR,TK_EOF
};

int line = 1, column = 1;

tuple<Token, string> next(fstream& f) {
    char c = f.peek();

skip_comment_and_find_next:

    for (; c == ' ' || c == '\r' || c == '\t' || c == '\n'; column++) {
        f.get();
        c = f.peek();
        if (c == '\n') {
            line++;
            column = 1;
        }
    }
    if (f.eof()) {
        return make_tuple(TK_EOF,"");
    }

    string lexeme;
    auto consumePeek = [&](char& c) {
        f.get();
        column++;
        char oc = c;
        c = f.peek();
        return oc;
    };

    // identifier = letter { letter | unicode_digit } .
    if (isalpha(c) || c == '_') {
        while (isalnum(c) || c == '_') {
            lexeme += c;
            c == '\n' ? (line += 1, column = 0) : (column++);
            f.get();
            c = f.peek();
        }

        for (int i = 0; i < sizeof(keywords); i++)
            if (keywords[i] == lexeme) {
                return make_tuple(static_cast<Token>(i), lexeme);
            }

        return make_tuple(TK_ID, lexeme);
    }

    // int_lit     = decimal_lit | octal_lit | hex_lit .
    // decimal_lit = ( "1" … "9" ) { decimal_digit } .
    // octal_lit   = "0" { octal_digit } .
    // hex_lit     = "0" ( "x" | "X" ) hex_digit { hex_digit } .

    // float_lit = decimals "." [ decimals ] [ exponent ] |
    //         decimals exponent |
    //         "." decimals [ exponent ] .
    // decimals  = decimal_digit { decimal_digit } .
    // exponent  = ( "e" | "E" ) [ "+" | "-" ] decimals .

    // imaginary_lit = (decimals | float_lit) "i" .
    if (isdigit(c) || c == '.') {
        if (c == '0') {
            lexeme += consumePeek(c);
            if (c == 'x' || c == 'X') {
                do {
                    lexeme += consumePeek(c);
                }while (isdigit(c) || c >= 'a' && c <= 'f' || c >= 'A' && c <= 'F');

                return make_tuple(LITERAL_INT, lexeme);
            } else if (c >= '0' && c <= '7') {
                lexeme += consumePeek(c);
                while ((c >= '0' && c <= '9') ||
                       (c == '.' || c == 'e' || c == 'E' || c == 'i')) {
                    if (c >= '0' && c <= '7') {
                        lexeme += consumePeek(c);
                    } else {
                        goto shall_float;
                    }
                }
                return make_tuple(LITERAL_INT, lexeme);
            }
            goto shall_float;
        } else {  // 1-9 or .

            if (c == '.') {
                lexeme += consumePeek(c);
                if (c == '.') {
                    lexeme += consumePeek(c);
                    if (c == '.') {
                        lexeme += consumePeek(c);
                        return make_tuple(OP_VARIADIC, lexeme);
                    } else {
                        throw runtime_error(
                            "expect variadic notation(...) but got .." + c);
                    }
                }
                goto shall_float;
            }

            lexeme += consumePeek(c);
        shall_float:  // skip char consuming and appending since we did that
                      // before jumping here;
            bool hasDot = false, hasExponent = false;
            while ((c >= '0' && c <= '9') || c == '.' || c == 'e' || c == 'E' ||
                   c == 'i') {
                if (c >= '0' && c <= '9') {
                    lexeme += consumePeek(c);
                } else if (c == '.' && !hasDot) {
                    lexeme += consumePeek(c);
                } else if ((c == 'e' && !hasExponent) ||
                           (c == 'E' && !hasExponent)) {
                    hasExponent = true;
                    lexeme += consumePeek(c);
                    if (c == '+' || c == '-') {
                        lexeme += consumePeek(c);
                    }
                } else {
                    f.get();
                    column++;
                    lexeme += c;
                    return make_tuple(LITERAL_IMG, lexeme);
                }
            }
            return make_tuple(LITERAL_INT, lexeme);
        }
    }

    //! NOT FULLY SUPPORT UNICODE RELATED LITERALS

    // rune_lit         = "'" ( unicode_value | byte_value ) "'" .
    // unicode_value    = unicode_char | little_u_value | big_u_value |
    // escaped_char . byte_value       = octal_byte_value | hex_byte_value .
    // octal_byte_value = `\` octal_digit octal_digit octal_digit .
    // hex_byte_value   = `\` "x" hex_digit hex_digit .
    // little_u_value   = `\` "u" hex_digit hex_digit hex_digit hex_digit .
    // big_u_value      = `\` "U" hex_digit hex_digit hex_digit hex_digit
    //                            hex_digit hex_digit hex_digit hex_digit .
    // escaped_char     = `\` ( "a" | "b" | "f" | "n" | "r" | "t" | "v" | `\` |
    // "'" | `"` ) .
    if (c == '\'') {
        lexeme += consumePeek(c);
        if (c == '\\') {
            lexeme += consumePeek(c);

            if (c == 'U' || c == 'u') {
                do {
                    lexeme += consumePeek(c);
                } while (isdigit(c) || (c >= 'a' && c <= 'f') ||
                         (c >= 'A' && c <= 'F'));
            } else {//\t \n \a
                lexeme += consumePeek(c);
            }
            
        } else {
            lexeme += consumePeek(c);
        }

        if (c != '\'') {
            throw runtime_error(
                "illegal rune at least in current implementation of g8");
        }
        lexeme += consumePeek(c);
        return make_tuple(LITERAL_RUNE, lexeme);
    }

    // string_lit             = raw_string_lit | interpreted_string_lit .
    // raw_string_lit         = "`" { unicode_char | newline } "`" .
    // interpreted_string_lit = `"` { unicode_value | byte_value } `"` .
    if (c == '`') {
        do {
            lexeme += consumePeek(c);
        } while (f.good() && c != '`');
        if (c != '`') {
            throw runtime_error(
                "raw string literal does not have a closed symbol \"`\"");
        }
        lexeme += consumePeek(c);
        return make_tuple(LITERAL_STR, lexeme);
    } else if (c == '"') {
        do {
            lexeme += consumePeek(c);
            if (c == '\\') {
                lexeme += consumePeek(c);
                lexeme += consumePeek(c);
            }
        } while (f.good() && (c != '\n' && c != '\r' && c != '"'));
        if (c != '"') {
            throw runtime_error(
                "string literal does not have a closed symbol \"\"\"");
        }
        lexeme += consumePeek(c);
        return make_tuple(LITERAL_STR, lexeme);
    }

    // operators
    switch (c) {
        case '+':  //+  += ++
            lexeme += consumePeek(c);
            if (c == '=') {
                lexeme += consumePeek(c);
                return make_tuple(OP_ADDASSIGN, lexeme);
            } else if (c == '+') {
                lexeme += consumePeek(c);
                return make_tuple(OP_INC, lexeme);
            }
            return make_tuple(OP_ADD, lexeme);
        case '&':  //&  &=  &&  &^  &^=
            lexeme += consumePeek(c);
            if (c == '=') {
                lexeme += consumePeek(c);
                return make_tuple(OP_BITANDASSIGN, lexeme);
            } else if (c == '&') {
                lexeme += consumePeek(c);
                return make_tuple(OP_AND, lexeme);
            } else if (c == '^') {
                lexeme += consumePeek(c);
                if (c == '=') {
                    lexeme += consumePeek(c);
                    return make_tuple(OP_ANDXORASSIGN, lexeme);
                }
                return make_tuple(OP_ANDXOR, lexeme);
            }
            return make_tuple(OP_BITAND, lexeme);
        case '=':  //=  ==
            lexeme += consumePeek(c);
            if (c == '=') {
                return make_tuple(OP_EQ, lexeme);
            }
            return make_tuple(OP_ASSIGN, lexeme);
        case '!':  //!  !=
            lexeme += consumePeek(c);
            if (c == '=') {
                lexeme += consumePeek(c);
                return make_tuple(OP_NE, lexeme);
            }
            return make_tuple(OP_NOT, lexeme);
        case '(':
            lexeme += consumePeek(c);
            return make_tuple(OP_LPAREN, lexeme);
        case ')':
            lexeme += consumePeek(c);
            return make_tuple(OP_RPAREN, lexeme);
        case '-':  //-  -= --
            lexeme += consumePeek(c);
            if (c == '=') {
                lexeme += consumePeek(c);
                return make_tuple(OP_SUBASSIGN, lexeme);
            } else if (c == '-') {
                lexeme += consumePeek(c);
                return make_tuple(OP_DEC, lexeme);
            }
            return make_tuple(OP_SUB, lexeme);
        case '|':  //|  |=  ||
            lexeme += consumePeek(c);
            if (c == '=') {
                lexeme += consumePeek(c);
                return make_tuple(OP_BITORASSIGN, lexeme);
            } else if (c == '|') {
                lexeme += consumePeek(c);
                return make_tuple(OP_OR, lexeme);
            }
            return make_tuple(OP_BITOR, lexeme);
        case '<':  //<  <=  <- <<  <<=
            lexeme += consumePeek(c);
            if (c == '=') {
                lexeme += consumePeek(c);
                return make_tuple(OP_LE, lexeme);
            } else if (c == '-') {
                lexeme += consumePeek(c);
                return make_tuple(OP_CHAN, lexeme);
            } else if (c == '<') {
                lexeme += consumePeek(c);
                if (c == '=') return make_tuple(OP_LSFTASSIGN, lexeme);
                return make_tuple(OP_LSHIFT, lexeme);
            }
            return make_tuple(OP_LT, lexeme);
        case '[':
            lexeme += consumePeek(c);
            return make_tuple(OP_LBRACKET, lexeme);
        case ']':
            lexeme += consumePeek(c);
            return make_tuple(OP_RBRACKET, lexeme);
        case '*':  //*  *=
            lexeme += consumePeek(c);
            if (c == '=') {
                return make_tuple(OP_MULASSIGN, lexeme);
            }
            return make_tuple(OP_MUL, lexeme);
        case '^':  //^  ^=
            lexeme += consumePeek(c);
            if (c == '=') {
                return make_tuple(OP_BITXORASSIGN, lexeme);
            }
            return make_tuple(OP_XOR, lexeme);
        case '>':  //>  >=  >>  >>=
            lexeme += consumePeek(c);
            if (c == '=') {
                lexeme += consumePeek(c);
                return make_tuple(OP_GE, lexeme);
            } else if (c == '>') {
                lexeme += consumePeek(c);
                if (c == '=') return make_tuple(OP_RSFTASSIGN, lexeme);
                return make_tuple(OP_RSHIFT, lexeme);
            }
            return make_tuple(OP_GT, lexeme);
        case '{':
            lexeme += consumePeek(c);
            return make_tuple(OP_LBRACE, lexeme);
        case '}':
            lexeme += consumePeek(c);
            return make_tuple(OP_RBRACE, lexeme);
        case '/': {  // /  /= // /*...*/
            char pending = consumePeek(c);
            if (c == '=') {
                lexeme += pending;
                lexeme += consumePeek(c);
                return make_tuple(OP_DIVASSIGN, lexeme);
            } else if (c == '/') {
                do {
                    consumePeek(c);
                } while (f.good() && (c != '\n' && c != '\r'));
                goto skip_comment_and_find_next;
            } else if (c == '*') {
                do {
                    consumePeek(c);
                    if (c == '*') {
                        consumePeek(c);
                        if (c == '/') {
                            consumePeek(c);
                            goto skip_comment_and_find_next;
                        }
                    }
                } while (f.good());
            }
            lexeme += pending;
            return make_tuple(OP_DIV, lexeme);
        }
        case ':':  // :=
            lexeme += consumePeek(c);
            if (c == '=') {
                lexeme += consumePeek(c);
                return make_tuple(OP_SHORTASSIGN, lexeme);
            }
            return make_tuple(OP_COLON, lexeme);
        case ',':
            lexeme += consumePeek(c);
            return make_tuple(OP_COMMA, lexeme);
        case ';':
            lexeme += consumePeek(c);
            return make_tuple(OP_SEMI, lexeme);
        case '%':  //%  %=
            lexeme += consumePeek(c);
            if (c == '=') {
                lexeme += consumePeek(c);
                return make_tuple(OP_MODASSIGN, lexeme);
            }
            return make_tuple(OP_MOD, lexeme);
            // case '.' has already checked
    }

    throw runtime_error("illegal token in source file");
}


int main() {
    fstream f("entity.go", ios::binary | ios::in);
    while (f.good()) {
        auto [token, lexeme] = next(f);
        fprintf(stdout, "<%d,%s,%d,%d>\n", token, lexeme.c_str(), line, column);
    }
    system("pause");
    return 0;
}