#include <iostream>
#include <fstream>
#include <string>
using namespace std;

enum Token{
    KW_break,KW_default,KW_func,KW_interface,KW_select,KW_case,KW_defer,KW_go,KW_map,KW_struct,KW_chan,KW_else,KW_goto,KW_package,KW_switch,
    KW_const,KW_fallthrough,KW_if,KW_range,KW_type,KW_continue,KW_for,KW_import,KW_return,KW_var,OP_ADD,OP_BITAND,OP_ADDASSIGN,P_BITANDASSIGN,
    OP_AND,OP_EQ,OP_NE,OP_LPAREN,OP_RPAREN,OP_SUB,OP_BITOR,OP_SUBASSIGN,OP_BITORASSIGN,OP_OR,OP_LT,OP_LE,OP_LBRACKET,OP_RBRACKET,OP_MUL,OP_XOR,
    OP_MULASSIGN,OP_XORASSIGN,OP_CHAN,OP_GT,OP_GE,OP_LBRACK,OP_RBRACE,OP_DIV,OP_LSHIFT,OP_DIVASSIGN,OP_LSFTASSIGN,OP_INC,OP_ASSIGN,OP_SHORTASSIGN,
    OP_COMMA,OP_SEMI,OP_MOD,OP_RSHIFT,OP_MODASSIGN,OP_RSFTASSIGN,OP_DEC,OP_NOT,OP_VARIADIC,OP_DOT,OP_COLON,OP_ANDNOT,OP_ANDNOTASSIGN,TK_ID,
    LITERAL_INT,LITERAL_FLOAT,LITERAL_IMG,LITERAL_RUNE,LITERAL_STR};


void tokenize() {

}

int main() { 
    fstream f("basic.go");
    char c;
    while (f>>c) {
        std::cout << c;
    }
    system("pause");
    return 0; 
}