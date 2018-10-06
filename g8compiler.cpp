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
    auto consumeAndPeek = [&]() {
        f.get();
        return f.peek();
    };

    char c = f.peek();

    for (; c == ' ' || c == '\r' || c == '\t' || c=='\n'; column++) {
        f.get();
        c = f.peek();
        if (c == '\n') {
            line++;
            column = 1;
        }
    }

    // identifier = letter { letter | unicode_digit } .
    if (isalpha(c) || c == '_') {
        string tempIdent;
        while (isalnum(c) || c == '_') {
            tempIdent += c;
            c == '\n' ? (line += 1, column = 0) : (column++);
            f.get();
            c = f.peek();
        }

        for (int i = 0; i < sizeof(keywords); i++)
            if (keywords[i] == tempIdent) {
                return make_tuple(static_cast<Token>(i), tempIdent);
            }

        return make_tuple(TK_ID, tempIdent);
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
        string tempDigit;
        if (c == '0') {
            f.get();
            column++;
            tempDigit += c;
            c = f.peek();
            if (c == 'x' || c == 'X') {
                f.get();
                column++;
                tempDigit += c;
                c = f.peek();
                while (isdigit(c) || c >= 'a' && c <= 'f' || c >= 'A' && c <= 'F') {
                    f.get();
                    column++;
                    tempDigit += c;
                    c = f.peek();
                }
                return make_tuple(LITERAL_INT, tempDigit);
            } else if (c >= '0' && c <= '7') {
                f.get();
                column++;
                tempDigit += c;
                c = f.peek();
                while ((c >= '0' && c <= '9') ||
                       (c == '.' || c == 'e' ||
                        c == 'E' || c == 'i') ) {
                    if (c >= '0' && c <= '7') {
                        f.get();
                        column++;
                        tempDigit += c;
                        c = f.peek();
                    } else {
                        goto shall_float;
                    }
                }
                return make_tuple(LITERAL_INT, tempDigit);
            }
            goto shall_float;
        } else{
            f.get();
            column++;
            tempDigit += c;
            c = f.peek();
        shall_float:  // skip char consuming and appending since we did that before jumping here;
            bool hasDot = false, hasExponent = false;
            while ((c >= '0' && c <= '9') || c == '.' || c == 'e' || c =='E' || c == 'i'){
                if (c >= '0' && c <= '9') {
                    f.get();
                    column++;
                    tempDigit += c;
                    c = f.peek();
                } else if (c == '.' && !hasDot) {
                    f.get();
                    column++;
                    tempDigit += c;
                    c = f.peek();
                } else if ((c == 'e' && !hasExponent) || (c == 'E' && !hasExponent)) {
                    hasExponent = true;
                    f.get();
                    column++;
                    tempDigit += c;
                    c = f.peek();
                    if (c == '+' || c == '-') {
                        f.get();
                        column++;
                        tempDigit += c;
                        c = f.peek();
                    }
                } else {
                    f.get();
                    column++;
                    tempDigit += c;
                    return make_tuple(LITERAL_IMG, tempDigit);
                }
            }
            return make_tuple(LITERAL_INT, tempDigit);
        }
    }
    return make_tuple(TK_ID,"not implemented yet");
}

void tokenize(fstream& f) {
    while (f.good()) {
        auto [token, lexeme] = next(f);
        fprintf(stdout, "<%d,\"%s\",%d,%d>\n", token, lexeme.c_str(), line,
                column);
    }
}

int main() {
    fstream f("lex.go", ios::binary | ios::in);
    tokenize(f);
    system("pause");
    return 0;
}