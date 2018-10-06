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
    KW_break,
    KW_default,
    KW_func,
    KW_interface,
    KW_select,
    KW_case,
    KW_defer,
    KW_go,
    KW_map,
    KW_struct,
    KW_chan,
    KW_else,
    KW_goto,
    KW_package,
    KW_switch,
    KW_const,
    KW_fallthrough,
    KW_if,
    KW_range,
    KW_type,
    KW_continue,
    KW_for,
    KW_import,
    KW_return,
    KW_var,
    OP_ADD,
    OP_BITAND,
    OP_ADDASSIGN,
    P_BITANDASSIGN,
    OP_AND,
    OP_EQ,
    OP_NE,
    OP_LPAREN,
    OP_RPAREN,
    OP_SUB,
    OP_BITOR,
    OP_SUBASSIGN,
    OP_BITORASSIGN,
    OP_OR,
    OP_LT,
    OP_LE,
    OP_LBRACKET,
    OP_RBRACKET,
    OP_MUL,
    OP_XOR,
    OP_MULASSIGN,
    OP_XORASSIGN,
    OP_CHAN,
    OP_GT,
    OP_GE,
    OP_LBRACK,
    OP_RBRACE,
    OP_DIV,
    OP_LSHIFT,
    OP_DIVASSIGN,
    OP_LSFTASSIGN,
    OP_INC,
    OP_ASSIGN,
    OP_SHORTASSIGN,
    OP_COMMA,
    OP_SEMI,
    OP_MOD,
    OP_RSHIFT,
    OP_MODASSIGN,
    OP_RSFTASSIGN,
    OP_DEC,
    OP_NOT,
    OP_VARIADIC,
    OP_DOT,
    OP_COLON,
    OP_ANDNOT,
    OP_ANDNOTASSIGN,
    TK_ID,
    LITERAL_INT,
    LITERAL_FLOAT,
    LITERAL_IMG,
    LITERAL_RUNE,
    LITERAL_STR
};

int line = 1, column = 1;

tuple<Token, string> next(fstream& f) {
    char c = f.peek();

    for (; c == ' ' || c == '\r' || c == '\t' || c == '\n'; column++) {
        f.get();
        c = f.peek();
        if (c == '\n') {
            line++;
            column = 1;
        }
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
                lexeme += consumePeek(c);
                while (isdigit(c) || c >= 'a' && c <= 'f' ||
                       c >= 'A' && c <= 'F') {
                    lexeme += consumePeek(c);
                }
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
        } else {
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
            lexeme += consumePeek(c);
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
        } while (f.good() && c!='`');
        if (c != '`') {
            throw runtime_error("raw string literal does not have a closed symbol \"`\"");
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
        } while (f.good() && c != '\n' && c != '\r' && c != '"');
        if (c != '"') {
            throw runtime_error("string literal does not have a closed symbol \"\"\"");
        }
        lexeme += consumePeek(c);
        return make_tuple(LITERAL_STR, lexeme);
    }

    return make_tuple(TK_ID, "not implemented yet");
}

void tokenize(fstream& f) {
    while (f.good()) {
        auto [token, lexeme] = next(f);
        fprintf(stdout, "<%d,%s,%d,%d>\n", token, lexeme.c_str(), line, column);
    }
}

int main() {
    fstream f("lex.go", ios::binary | ios::in);
    tokenize(f);
    system("pause");
    return 0;
}