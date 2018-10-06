//===----------------------------------------------------------------------===//
// golang specification can be found here: https://golang.org/ref/spe
// 
// in development mode, i consider raise exception when met error , they will be 
// replaced with error stream later
//===----------------------------------------------------------------------===//

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>
#include <fstream>
#include <functional>
#include <string>
#include <vector>
#include <exception>

using namespace std;

string keywords[] = {"break",    "default",     "func",   "interface", "select",
                     "case",     "defer",       "go",     "map",       "struct",
                     "chan",     "else",        "goto",   "package",   "switch",
                     "const",    "fallthrough", "if",     "range",     "type",
                     "continue", "for",         "import", "return",    "var"};

enum Token{
    KW_break,KW_default,KW_func,KW_interface,KW_select,KW_case,KW_defer,KW_go,KW_map,KW_struct,KW_chan,KW_else,KW_goto,KW_package,KW_switch,
    KW_const,KW_fallthrough,KW_if,KW_range,KW_type,KW_continue,KW_for,KW_import,KW_return,KW_var,OP_ADD,OP_BITAND,OP_ADDASSIGN,P_BITANDASSIGN,
    OP_AND,OP_EQ,OP_NE,OP_LPAREN,OP_RPAREN,OP_SUB,OP_BITOR,OP_SUBASSIGN,OP_BITORASSIGN,OP_OR,OP_LT,OP_LE,OP_LBRACKET,OP_RBRACKET,OP_MUL,OP_XOR,
    OP_MULASSIGN,OP_XORASSIGN,OP_CHAN,OP_GT,OP_GE,OP_LBRACK,OP_RBRACE,OP_DIV,OP_LSHIFT,OP_DIVASSIGN,OP_LSFTASSIGN,OP_INC,OP_ASSIGN,OP_SHORTASSIGN,
    OP_COMMA,OP_SEMI,OP_MOD,OP_RSHIFT,OP_MODASSIGN,OP_RSFTASSIGN,OP_DEC,OP_NOT,OP_VARIADIC,OP_DOT,OP_COLON,OP_ANDNOT,OP_ANDNOTASSIGN,TK_ID,
    LITERAL_INT,LITERAL_FLOAT,LITERAL_IMG,LITERAL_RUNE,LITERAL_STR};


int line = 1, column = 1;

tuple<Token, string> next(fstream& f) {
    char c = f.peek();
    auto isdelimiter = [](char c) {
        return c == ' ' || c == '\n' || c == '\t' || c == '\r';
    };

    for (; c == ' ' || c == '\r' || c == '\t'; column++) {
        f.get();
        c = f.peek();
    }

    for (; c == '\n'; line++, column = 0) {
        f.get();
        c = f.peek();
    }

    // identifier = letter { letter | unicode_digit } .
    if (isalpha(c) || c == '_') {
        string tempIdent;
        while (!isdelimiter(c)) {
            if(isalnum(c) || c == '_') {
                tempIdent += c;
                c == '\n' ? (line += 1, column = 0) : (column++);
                f.get();
                c = f.peek();
            } else {
                throw exception("invalid identifier name");
            }
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

    if (isdigit(c)) {
        string tempDigit;
        // int_lit = octal_lit | hex_lit
        if (c == '0') {
            f.get();
            column++;
            tempDigit += c;
            c = f.peek();
            // hex_lit     = "0" ( "x" | "X" ) hex_digit { hex_digit } .
            if (c == 'x' || c == 'X') {
                f.get();
                column++;
                tempDigit += c;
                c = f.peek();
                while (!isdelimiter(c)) {
                    if (isdigit(c) || c >= 'a' && c <= 'f' ||
                        c >= 'A' && c <= 'F') {
                        f.get();
                        column++;
                        tempDigit += c;
                        c = f.peek();
                    } else {
                        throw exception("invalid hexadecimal number");
                    }
                }
                return make_tuple(LITERAL_INT, tempDigit);
            }
            // octal_lit   = "0" { octal_digit } .
            else if (c >= '0' && c <= '7') {
                f.get();
                column++;
                tempDigit += c;
                c = f.peek();
                while (c >= '0' && c <= '7') {
                    f.get();
                    column++;
                    tempDigit += c;
                    c = f.peek();
                }
                return make_tuple(LITERAL_INT, tempDigit);
            } else {
                throw exception("invalid hexadecimal or octal number");
            }
        }
        // int_lit = decimal_lit
        else if (c >= '1' && c <= '9') {
            f.get();
            column++;
            tempDigit += c;
            c = f.peek();
            while (c >= '0' && c <= '9') {
                f.get();
                column++;
                tempDigit += c;
                c = f.peek();
            }
            return make_tuple(LITERAL_INT, tempDigit);
        }
    }
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