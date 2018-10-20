//===----------------------------------------------------------------------===//
// Minimalism guided practice of golang compiler and runtime bundled 
// implementation, I try to do all works within 5 named functions. 
//
// Written by racaljk@github<1948638989@qq.com>
//===----------------------------------------------------------------------===//
#include <cctype>
#include <cstdio>
#include <exception>
#include <fstream>
#include <functional>
#include <string>
#include <vector>
#include <tuple>
#include <map>
#define LAMBDA_FUN(X) function<Ast##X*(Token&)> parse##X;
#define _ND :public AstNode
using namespace std;

//===----------------------------------------------------------------------===//
// various declarations 
//===----------------------------------------------------------------------===//
enum TokenType : signed int {
    INVALID = 0, KW_break, KW_default, KW_func, KW_interface, KW_select, KW_case,
    KW_defer, KW_go, KW_map, KW_struct, KW_chan, KW_else, KW_goto, KW_package,
    KW_switch, KW_const, KW_fallthrough, KW_if, KW_range, KW_type, KW_continue,
    KW_for, KW_import, KW_return, KW_var, OP_ADD, OP_BITAND, OP_ADDAGN, OP_BITANDAGN,
    OP_AND, OP_EQ, OP_NE, OP_LPAREN, OP_RPAREN, OP_SUB, OP_BITOR, OP_SUBAGN,
    OP_BITORAGN, OP_OR, OP_LT, OP_LE, OP_LBRACKET, OP_RBRACKET, OP_MUL, OP_XOR,
    OP_MULAGN, OP_BITXORAGN, OP_CHAN, OP_GT, OP_GE, OP_LBRACE, OP_RBRACE,
    OP_DIV, OP_LSHIFT, OP_DIVAGN, OP_LSFTAGN, OP_INC, OP_AGN, OP_SHORTAGN,
    OP_COMMA, OP_SEMI, OP_MOD, OP_RSHIFT, OP_MODAGN, OP_RSFTAGN, OP_DEC,
    OP_NOT, OP_VARIADIC, OP_DOT, OP_COLON, OP_ANDXOR, OP_ANDXORAGN, TK_ID,
    LIT_INT, LIT_FLOAT, LIT_IMG, LIT_RUNE, LIT_STR, TK_EOF = -1,
};
//todo: add destructor for these structures
// Common
struct AstExpr;
struct AstNode { virtual ~AstNode() = default; };
struct AstIdentList _ND { vector<string> identList; };
struct AstExprList _ND { vector<AstExpr*> exprList; };
struct AstStmtList _ND { vector<AstNode*> stmtList; };

// Declaration
struct AstImportDecl _ND { map<string, string> imports; };
struct AstConstDecl _ND {
    vector<AstIdentList*> identList;
    vector<AstNode*> type;
    vector<AstExprList*> exprList;
};
struct AstTypeSpec _ND { string ident; AstNode* type; };
struct AstTypeDecl _ND { vector<AstTypeSpec*> typeSpec; };
struct AstVarDecl _ND { vector<AstNode*> varSpec; };
struct AstVarSpec _ND {
    AstNode* identList{};
    AstExprList* exprList{};
    AstNode* type{};
};
struct AstFuncDecl _ND {
    string funcName;
    AstNode* receiver{};
    AstNode* signature{};
    AstStmtList* funcBody{};
};
struct AstSourceFile _ND {
    vector<AstImportDecl*> importDecl;
    vector<AstConstDecl*> constDecl;
    vector<AstTypeDecl*> typeDecl;
    vector<AstFuncDecl*> funcDecl;
    vector<AstVarDecl*> varDecl;
};
// Type
struct AstType _ND { AstNode* type{}; };
struct AstName _ND { string name; };
struct AstArrayType _ND {
    AstNode* length{};
    AstNode* elemType{};
    bool automaticLen;
};
struct AstStructType _ND {
    vector<tuple<AstNode*, AstNode*, string, bool>> fields;// <IdentList/Name,Type,Tag,isEmbeded>
};
struct AstPtrType _ND { AstNode* baseType{}; };
struct AstSignature _ND {
    AstNode* param{};
    AstNode* result{};
};
struct AstFuncType _ND { AstSignature * signature{}; };
struct AstParam _ND { vector<AstNode*> paramList; };
struct AstParamDecl _ND {
    bool isVariadic = false;
    bool hasName = false;
    AstNode* type{};
    string name;
};
struct AstResult _ND {
    AstNode* param;
    AstNode* type;
};
struct AstMethodSpec _ND {
    AstName* name;
    AstSignature* signature;
};
struct AstInterfaceType _ND { vector<AstMethodSpec*> methodSpec; };
struct AstSliceType _ND { AstNode* elemType{}; };
struct AstMapType _ND {AstNode* keyType{};AstNode* elemType{};};
struct AstChanType _ND { AstNode* elemType{}; };
// Statement
struct AstGoStmt _ND { AstExpr* expr{}; AstGoStmt(AstExpr* expr) :expr(expr) {} };
struct AstReturnStmt _ND { AstExprList* exprList{}; AstReturnStmt(AstExprList* el) :exprList(el) {} };
struct AstBreakStmt _ND { string label; AstBreakStmt(const string&s) :label(s) {} };
struct AstDeferStmt _ND { AstExpr* expr{}; AstDeferStmt(AstExpr* expr) :expr(expr) {} };
struct AstContinueStmt _ND { string label; AstContinueStmt(const string&s) :label(s) {} };
struct AstGotoStmt _ND { string label; AstGotoStmt(const string&s) :label(s) {} };
struct AstFallthroughStmt _ND {};
struct AstLabeledStmt _ND {
    string label;
    AstNode* stmt{};
};
struct AstIfStmt _ND {
    AstNode* init{};
    AstExpr* cond{};
    AstNode* ifBlock{}, *elseBlock{};
};
struct AstSwitchCase _ND {
    AstNode* exprList{};
    AstNode* stmtList{};
};
struct AstSwitchStmt _ND {
    AstNode* init{};
    AstNode* cond{};
    vector<AstSwitchCase*> caseList{};
};
struct AstSelectCase _ND {
    AstNode* cond{};
    AstStmtList* stmtList{};
};
struct AstSelectStmt _ND {
    vector<AstSelectCase*> caseList;
};
struct AstForStmt _ND {
    AstNode* init{}, *cond{}, *post{};
    AstNode* block{};
};
struct AstSRangeClause _ND {
    vector<string> lhs;
    AstExpr* rhs{};
};
struct AstRangeClause _ND {
    AstExprList* lhs{};
    TokenType op;
    AstExpr* rhs{};
};
struct AstExprStmt _ND { AstExpr* expr{}; };
struct AstSendStmt _ND { AstExpr* receiver{}, *sender{}; };
struct AstIncDecStmt _ND {
    AstExpr* expr{};
    bool isInc{};
};
struct AstAssignStmt _ND {
    AstExprList* lhs{}, *rhs{};
    TokenType op{};
};
struct AstSAssignStmt _ND {//short assign
    vector<string> lhs{};
    AstExprList* rhs{};
};
// Expression
struct AstPrimaryExpr _ND {
    AstNode* expr{};//one of SelectorExpr, TypeSwitchExpr,TypeAssertionExpr,IndexExpr,SliceExpr,CallExpr,Operand
};
struct AstUnaryExpr _ND {
    AstNode*expr{};
    TokenType op{};
};
struct AstExpr _ND {
    AstUnaryExpr* lhs{};
    TokenType op{};
    AstExpr* rhs{};
};
struct AstSelectorExpr _ND {
    AstNode* operand{};
    string selector;
};
struct AstTypeSwitchExpr _ND {
    AstNode* operand{};
};
struct AstTypeAssertionExpr _ND {
    AstNode* operand{};
    AstNode* type{};
};
struct AstIndexExpr _ND {
    AstNode* operand{};
    AstNode* index{};
};
struct AstSliceExpr _ND {
    AstNode* operand{};
    AstNode* begin{};
    AstNode* end{};
    AstNode* step{};
};
struct AstCallExpr _ND {
    AstNode* operand{};
    AstNode* arguments{};
    AstNode* type{};
    bool isVariadic{};
};
struct AstLitValue _ND { vector< AstNode*> keyedElement; };
struct AstKeyedElement _ND {
    AstNode*key{};
    AstNode*element{};
};
struct AstBasicLit _ND { TokenType type{}; string value; AstBasicLit(TokenType t, const string&s) :type(t), value(s) {} };
struct AstCompositeLit _ND { AstNode* litName{}; AstLitValue* litValue{}; };
//===----------------------------------------------------------------------===//
// global data
//===----------------------------------------------------------------------===//
string keywords[] = { "break",    "default",     "func",   "interface", "select",
                     "case",     "defer",       "go",     "map",       "struct",
                     "chan",     "else",        "goto",   "package",   "switch",
                     "const",    "fallthrough", "if",     "range",     "type",
                     "continue", "for",         "import", "return",    "var" };
static int line = 1, column = 1, lastToken = -1, shouldEof = 0, nestLev = 0;
struct Token { TokenType type{}; string lexeme; };
static struct goruntime {
    string package;
} grt;

//===----------------------------------------------------------------------===//
// Implementation of golang compiler and runtime within 5 functions
//===----------------------------------------------------------------------===//

Token next(fstream& f) {
    auto consumePeek = [&](char& c) {
        f.get();
        column++;
        char oc = c;
        c = static_cast<char>(f.peek());
        return oc;
    };
    auto c = static_cast<char>(f.peek());

skip_comment_and_find_next:

    for (; c == ' ' || c == '\r' || c == '\t' || c == '\n'; column++) {
        if (c == '\n') {
            line++;
            column = 1;
            if ((lastToken >= TK_ID && lastToken <= LIT_STR)
                || lastToken == KW_fallthrough || lastToken == KW_continue
                || lastToken == KW_return || lastToken == KW_break
                || lastToken == OP_INC || lastToken == OP_DEC
                || lastToken == OP_RPAREN
                || lastToken == OP_RBRACKET || lastToken == OP_RBRACE) {
                consumePeek(c);
                lastToken = OP_SEMI;
                return Token{ OP_SEMI, ";" };
            }
        }
        consumePeek(c);
    }
    if (f.eof()) {
        if (shouldEof) {
            lastToken = TK_EOF;
            return Token{TK_EOF, ""};
        }
        shouldEof = 1;
        lastToken = OP_SEMI;
        return Token{OP_SEMI, ";"};
    }

    string lexeme;


    // identifier = letter { letter | unicode_digit } .
    if (isalpha(c) || c == '_') {
        while (isalnum(c) || c == '_') {
            lexeme += consumePeek(c);
        }

        for (int i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++)
            if (keywords[i] == lexeme) {
                lastToken = static_cast<TokenType>(i + 1);
                return Token{ static_cast<TokenType>(i + 1), lexeme };
            }
        lastToken = TK_ID;
        return Token{TK_ID, lexeme};
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
                } while (isdigit(c) || c >= 'a' && c <= 'f' || c >= 'A' && c <= 'F');
                lastToken = LIT_INT;
                return Token{LIT_INT, lexeme};
            }
            else if ((c >= '0' && c <= '9') ||
                (c == '.' || c == 'e' || c == 'E' || c == 'i')) {
                while ((c >= '0' && c <= '9') ||
                    (c == '.' || c == 'e' || c == 'E' || c == 'i')) {
                    if (c >= '0' && c <= '7') {
                        lexeme += consumePeek(c);
                    }
                    else {
                        goto shall_float;
                    }
                }
                lastToken = LIT_INT;
                return Token{LIT_INT, lexeme};
            }
            goto may_float;
        }
        else {  // 1-9 or . or just a single 0
        may_float:
            TokenType type = LIT_INT;
            if (c == '.') {
                lexeme += consumePeek(c);
                if (c == '.') {
                    lexeme += consumePeek(c);
                    if (c == '.') {
                        lexeme += consumePeek(c);
                        lastToken = OP_VARIADIC;
                        return Token{OP_VARIADIC, lexeme};
                    }
                    else {
                        throw runtime_error(
                            "expect variadic notation(...) but got .." + c);
                    }
                }
                else if (c >= '0'&&c <= '9') {
                    type = LIT_FLOAT;
                }
                else {
                    lastToken = OP_DOT;
                    return Token{OP_DOT, "."};
                }
                goto shall_float;
            }
            else if (c >= '1'&&c <= '9') {
                lexeme += consumePeek(c);
            shall_float:  // skip char consuming and appending since we did that before jumping here;
                bool hasDot = false, hasExponent = false;
                while ((c >= '0' && c <= '9') || c == '.' || c == 'e' || c == 'E' ||
                    c == 'i') {
                    if (c >= '0' && c <= '9') {
                        lexeme += consumePeek(c);
                    }
                    else if (c == '.' && !hasDot) {
                        lexeme += consumePeek(c);
                        type = LIT_FLOAT;
                    }
                    else if ((c == 'e' && !hasExponent) ||
                        (c == 'E' && !hasExponent)) {
                        hasExponent = true;
                        type = LIT_FLOAT;
                        lexeme += consumePeek(c);
                        if (c == '+' || c == '-') {
                            lexeme += consumePeek(c);
                        }
                    }
                    else {
                        f.get();
                        column++;
                        lexeme += c;
                        lastToken = LIT_IMG;
                        return Token{LIT_IMG, lexeme};
                    }
                }
                lastToken = type;
                return Token{type, lexeme};
            }
            else {
                lastToken = type;
                return Token{type, lexeme};
            }
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

            if (c == 'U' || c == 'u' || c == 'x' || c == 'X') {
                do {
                    lexeme += consumePeek(c);
                } while (isdigit(c) || (c >= 'a' && c <= 'f') ||
                    (c >= 'A' && c <= 'F'));
            }
            else if (c >= '0' && c <= '7') {
                do {
                    lexeme += consumePeek(c);
                } while (c >= '0' && c <= '7');
            }
            else if (c == 'a' || c == 'b' || c == 'f' || c == 'n' || c == 'r' || c == 't' ||
                c == 'v' || c == '\\' || c == '\'' || c == '"') {
                lexeme += consumePeek(c);
            }
            else {
                throw runtime_error("illegal rune");
            }

        }
        else {
            lexeme += consumePeek(c);
        }

        if (c != '\'') {
            throw runtime_error(
                "illegal rune at least in current implementation of g8");
        }
        lexeme += consumePeek(c);
        lastToken = LIT_RUNE;
        return Token{LIT_RUNE, lexeme};
    }

    // string_lit             = raw_string_lit | interpreted_string_lit .
    // raw_string_lit         = "`" { unicode_char | newline } "`" .
    // interpreted_string_lit = `"` { unicode_value | byte_value } `"` .
    if (c == '`') {
        do {
            lexeme += consumePeek(c);
            if (c == '\n') line++;
        } while (f.good() && c != '`');
        if (c != '`') {
            throw runtime_error(
                "raw string literal does not have a closed symbol \"`\"");
        }
        lexeme += consumePeek(c);
        lastToken = LIT_STR;
        return Token{LIT_STR, lexeme};
    }
    else if (c == '"') {
        do {
            lexeme += consumePeek(c);
            if (c == '\\') {
                lexeme += consumePeek(c);
                lexeme += consumePeek(c);
            }
        } while (f.good() && (c != '\n' && c != '\r' && c != '"'));
        if (c != '"') {
            throw runtime_error(
                R"(string literal does not have a closed symbol """)");
        }
        lexeme += consumePeek(c);
        lastToken = LIT_STR;
        return Token{LIT_STR, lexeme};
    }

    // operators
    switch (c) {
    case '+':  //+  += ++
        lexeme += consumePeek(c);
        if (c == '=') {
            lexeme += consumePeek(c);
            lastToken = OP_ADDAGN;
            return Token{OP_ADDAGN, lexeme};
        }
        else if (c == '+') {
            lexeme += consumePeek(c);
            lastToken = OP_INC;
            return Token{OP_INC, lexeme};
        }
        return Token{OP_ADD, lexeme};
    case '&':  //&  &=  &&  &^  &^=
        lexeme += consumePeek(c);
        if (c == '=') {
            lexeme += consumePeek(c);
            lastToken = OP_BITANDAGN;
            return Token{OP_BITANDAGN, lexeme};
        }
        else if (c == '&') {
            lexeme += consumePeek(c);
            lastToken = OP_AND;
            return Token{OP_AND, lexeme};
        }
        else if (c == '^') {
            lexeme += consumePeek(c);
            if (c == '=') {
                lexeme += consumePeek(c);
                lastToken = OP_ANDXORAGN;
                return Token{OP_ANDXORAGN, lexeme};
            }
            lastToken = OP_ANDXOR;
            return Token{OP_ANDXOR, lexeme};
        }
        lastToken = OP_BITAND;
        return Token{OP_BITAND, lexeme};
    case '=':  //=  ==
        lexeme += consumePeek(c);
        if (c == '=') {
            lexeme += consumePeek(c);
            lastToken = OP_EQ;
            return Token{OP_EQ, lexeme};
        }
        lastToken = OP_AGN;
        return Token{OP_AGN, lexeme};
    case '!':  //!  !=
        lexeme += consumePeek(c);
        if (c == '=') {
            lexeme += consumePeek(c);
            lastToken = OP_NE;
            return Token{OP_NE, lexeme};
        }
        lastToken = OP_NOT;
        return Token{OP_NOT, lexeme};
    case '(':
        lexeme += consumePeek(c);
        lastToken = OP_LPAREN;
        return Token{OP_LPAREN, lexeme};
    case ')':
        lexeme += consumePeek(c);
        lastToken = OP_RPAREN;
        return Token{OP_RPAREN, lexeme};
    case '-':  //-  -= --
        lexeme += consumePeek(c);
        if (c == '=') {
            lexeme += consumePeek(c);
            lastToken = OP_SUBAGN;
            return Token{OP_SUBAGN, lexeme};
        }
        else if (c == '-') {
            lexeme += consumePeek(c);
            lastToken = OP_DEC;
            return Token{OP_DEC, lexeme};
        }
        lastToken = OP_SUB;
        return Token{OP_SUB, lexeme};
    case '|':  //|  |=  ||
        lexeme += consumePeek(c);
        if (c == '=') {
            lexeme += consumePeek(c);
            lastToken = OP_BITORAGN;
            return Token{OP_BITORAGN, lexeme};
        }
        else if (c == '|') {
            lexeme += consumePeek(c);
            lastToken = OP_OR;
            return Token{OP_OR, lexeme};
        }
        lastToken = OP_BITOR;
        return Token{OP_BITOR, lexeme};
    case '<':  //<  <=  <- <<  <<=
        lexeme += consumePeek(c);
        if (c == '=') {
            lexeme += consumePeek(c);
            lastToken = OP_LE;
            return Token{OP_LE, lexeme};
        }
        else if (c == '-') {
            lexeme += consumePeek(c);
            lastToken = OP_CHAN;
            return Token{OP_CHAN, lexeme};
        }
        else if (c == '<') {
            lexeme += consumePeek(c);
            if (c == '=') {
                lexeme += consumePeek(c);
                lastToken = OP_LSFTAGN;
                return Token{OP_LSFTAGN, lexeme};
            }
            lastToken = OP_LSHIFT;
            return Token{OP_LSHIFT, lexeme};
        }
        lastToken = OP_LT;
        return Token{OP_LT, lexeme};
    case '[':
        lexeme += consumePeek(c);
        lastToken = OP_LBRACKET;
        return Token{OP_LBRACKET, lexeme};
    case ']':
        lexeme += consumePeek(c);
        lastToken = OP_RBRACKET;
        return Token{OP_RBRACKET, lexeme};
    case '*':  //*  *=
        lexeme += consumePeek(c);
        if (c == '=') {
            lexeme += consumePeek(c);
            lastToken = OP_MULAGN;
            return Token{OP_MULAGN, lexeme};
        }
        lastToken = OP_MUL;
        return Token{OP_MUL, lexeme};
    case '^':  //^  ^=
        lexeme += consumePeek(c);
        if (c == '=') {
            lexeme += consumePeek(c);
            lastToken = OP_BITXORAGN;
            return Token{OP_BITXORAGN, lexeme};
        }
        lastToken = OP_XOR;
        return Token{OP_XOR, lexeme};
    case '>':  //>  >=  >>  >>=
        lexeme += consumePeek(c);
        if (c == '=') {
            lexeme += consumePeek(c);
            lastToken = OP_GE;
            return Token{OP_GE, lexeme};
        }
        else if (c == '>') {
            lexeme += consumePeek(c);
            if (c == '=') {
                lexeme += consumePeek(c);
                lastToken = OP_RSFTAGN;
                return Token{OP_RSFTAGN, lexeme};
            }
            lastToken = OP_RSHIFT;
            return Token{OP_RSHIFT, lexeme};
        }
        lastToken = OP_GT;
        return Token{OP_GT, lexeme};
    case '{':
        lexeme += consumePeek(c);
        lastToken = OP_LBRACE;
        return Token{OP_LBRACE, lexeme};
    case '}':
        lexeme += consumePeek(c);
        lastToken = OP_RBRACE;
        return Token{OP_RBRACE, lexeme};
    case '/': {  // /  /= // /*...*/
        char pending = consumePeek(c);
        if (c == '=') {
            lexeme += pending;
            lexeme += consumePeek(c);
            lastToken = OP_DIVAGN;
            return Token{OP_DIVAGN, lexeme};
        }
        else if (c == '/') {
            do {
                consumePeek(c);
            } while (f.good() && (c != '\n' && c != '\r'));
            goto skip_comment_and_find_next;
        }
        else if (c == '*') {
            do {
                consumePeek(c);
                if (c == '\n') line++;
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
        lastToken = OP_DIV;
        return Token{OP_DIV, lexeme};
    }
    case ':':  // :=
        lexeme += consumePeek(c);
        if (c == '=') {
            lexeme += consumePeek(c);
            lastToken = OP_SHORTAGN;
            return Token{OP_SHORTAGN, lexeme};
        }
        lastToken = OP_COLON;
        return Token{OP_COLON, lexeme};
    case ',':
        lexeme += consumePeek(c);
        lastToken = OP_COMMA;
        return Token{OP_COMMA, lexeme};
    case ';':
        lexeme += consumePeek(c);
        lastToken = OP_SEMI;
        return Token{OP_SEMI, lexeme};
    case '%':  //%  %=
        lexeme += consumePeek(c);
        if (c == '=') {
            lexeme += consumePeek(c);
            lastToken = OP_MODAGN;
            return Token{OP_MODAGN, lexeme};
        }
        lastToken = OP_MOD;
        return Token{OP_MOD, lexeme};
        // case '.' has already checked
    }

    throw runtime_error("illegal token in source file");
}

const AstNode* parse(const string & filename) {
    fstream f(filename, ios::binary | ios::in);
    auto t = next(f);

    auto eat = [&f, &t](TokenType tk, const string&msg) {
        if (t.type != tk) throw runtime_error(msg);
        t = next(f);
    };
    auto eatOptionalSemi = [&]() { if (t.type == OP_SEMI) t = next(f); };

    auto expect = [&f, &t](TokenType tk, const string& msg) {
        t = next(f);
        if (t.type != tk) throw runtime_error(msg);
        return t;
    };
    LAMBDA_FUN(TypeDecl); LAMBDA_FUN(VarDecl); LAMBDA_FUN(ConstDecl); LAMBDA_FUN(LitValue);
    LAMBDA_FUN(ImportDecl); LAMBDA_FUN(Expr); LAMBDA_FUN(Signature); LAMBDA_FUN(UnaryExpr);
    LAMBDA_FUN(PrimaryExpr); LAMBDA_FUN(MethodSpec); LAMBDA_FUN(TypeSpec); LAMBDA_FUN(SwitchStmt);
    LAMBDA_FUN(SelectCase); LAMBDA_FUN(SwitchCase);
    function<AstFuncDecl*(bool, Token&)> parseFuncDecl;
    function<AstNode*(AstExprList *, Token&)> parseSimpleStmt;
    function<AstStmtList*(Token&)> parseBlock;
    function<AstNode*(Token&)> parseTypeAssertion,parseType,
        parseArrayOrSliceType, parseStructType, parsePtrType, parseFuncType,
        parseParam, parseParamDecl, parseResult, parseInterfaceType,
        parseMapType, parseChanType,
        parseVarSpec, parseStmt,
        parseIfStmt, 
        parseSelectStmt, parseForStmt,
        parseOperand, 
        parseKeyedElement, parseKey;

#pragma region Common
    auto parseName = [&](bool couldFullName, Token&t)->AstName* {
        AstName * node{};
        if (t.type == TK_ID) {
            node = new AstName;
            string name;
            name += t.lexeme;
            t = next(f);
            if (couldFullName && t.type == OP_DOT) {
                t = next(f);
                name.operator+=(".").operator+=(t.lexeme);
                t = next(f);
            }
            node->name = name;
        }
        return node;
    };
    auto parseIdentList = [&](Token&t)->AstIdentList* {
        AstIdentList* node{};
        if (t.type == TK_ID) {
            node = new  AstIdentList;
            node->identList.emplace_back(t.lexeme);
            t = next(f);
            while (t.type == OP_COMMA) {
                t = next(f);
                node->identList.emplace_back(t.lexeme);
                t = next(f);
            }
        }
        return node;
    };
    auto parseExprList = [&](Token&t)->AstExprList* {
        AstExprList* node{};
        if (auto* tmp = parseExpr(t); tmp != nullptr) {
            node = new  AstExprList;
            node->exprList.emplace_back(tmp);
            while (t.type == OP_COMMA) {
                t = next(f);
                node->exprList.emplace_back(parseExpr(t));
            }
        }
        return node;
    };
    auto parseStmtList = [&](Token&t)->AstStmtList* {
        AstStmtList * node{};
        AstNode* tmp = nullptr;
        while ((tmp = parseStmt(t))) {
            if (node == nullptr) {
                node = new AstStmtList;
            }
            node->stmtList.push_back(tmp);
            eatOptionalSemi();
        }
        return node;
    };
#pragma endregion
#pragma region Declaration
    auto parseSourceFile = [&](Token&t)->AstNode* {
        AstSourceFile * node = new AstSourceFile;
        eat(KW_package, "a go source file must start with package declaration");
        grt.package = t.lexeme;
        eat(TK_ID, "name required at the package declaration");
        eat(OP_SEMI, "expect ; at the end of package declaration");
        while (t.type != TK_EOF) {
            switch (t.type) {
            case KW_import:node->importDecl.push_back(parseImportDecl(t)); break;
            case KW_const:node->constDecl.push_back(parseConstDecl(t)); break;
            case KW_type:node->typeDecl.push_back(parseTypeDecl(t)); break;
            case KW_var:node->varDecl.push_back(parseVarDecl(t)); break;
            case KW_func:node->funcDecl.push_back(parseFuncDecl(false, t)); break;
            case OP_SEMI:t = next(f); break;
            default:throw runtime_error("unknown top level declaration");
            }
        }
        return node;
    };
    parseImportDecl = [&](Token&t)->AstImportDecl* {
        auto node = new AstImportDecl;
        eat(KW_import, "it should be import declaration");
        if (t.type == OP_LPAREN) {
            t = next(f);
            do {
                string importName, alias;
                if (t.type == OP_DOT || t.type == TK_ID) {
                    alias = t.lexeme;
                    t = next(f);
                    importName = t.lexeme;
                }
                else {
                    importName = t.lexeme;
                }
                importName = importName.substr(1, importName.length() - 2);
                node->imports[importName] = alias;
                t = next(f);
                eatOptionalSemi();
            } while (t.type != OP_RPAREN);
            eat(OP_RPAREN, "expect )");
        }
        else {
            string importName, alias;
            if (t.type == OP_DOT || t.type == TK_ID) {
                alias = t.lexeme;
                t = next(f);
                importName = t.lexeme;
                t = next(f);
            }
            else {
                importName = t.lexeme;
                t = next(f);
            }
            importName = importName.substr(1, importName.length() - 2);
            node->imports[importName] = alias;
        }
        return node;
    };
    parseConstDecl = [&](Token&t)->AstConstDecl* {
        auto * node = new AstConstDecl;
        eat(KW_const, "it should be const declaration");
        if (t.type == OP_LPAREN) {
            t = next(f);
            do {
                node->identList.push_back(parseIdentList(t));
                if (auto*tmp = parseType(t); tmp != nullptr) {
                    node->type.push_back(tmp);
                }
                else {
                    node->type.push_back(nullptr);
                }
                if (t.type == OP_AGN) {
                    t = next(f);
                    node->exprList.push_back(parseExprList(t));
                }
                else {
                    node->exprList.push_back(nullptr);
                }
                eatOptionalSemi();
            } while (t.type != OP_RPAREN);
            eat(OP_RPAREN, "eat right parenthesis");
        }
        else {
            node->identList.push_back(parseIdentList(t));
            if (auto*tmp = parseType(t); tmp != nullptr) {
                node->type.push_back(tmp);
            }
            else {
                node->type.push_back(nullptr);
            }
            if (t.type == OP_AGN) {
                t = next(f);
                node->exprList.push_back(parseExprList(t));
            }
            else {
                node->exprList.push_back(nullptr);
            }
            if (t.type != OP_SEMI) throw runtime_error("expect an explicit semicolon");
        }

        return node;
    };
    parseTypeDecl = [&](Token&t)->AstTypeDecl* {
        auto * node = new AstTypeDecl;
        eat(KW_type, "it should be type declaration");
        if (t.type == OP_LPAREN) {
            t = next(f);
            do {
                node->typeSpec.push_back(parseTypeSpec(t));
                eatOptionalSemi();
            } while (t.type != OP_RPAREN);
            eat(OP_RPAREN, "expect )");
        }
        else {
            node->typeSpec.push_back(parseTypeSpec(t));
        }
        return node;
    };
    parseTypeSpec = [&](Token&t)->AstTypeSpec* {
        AstTypeSpec* node{};
        if (t.type == TK_ID) {
            node = new AstTypeSpec;
            node->ident = t.lexeme;
            t = next(f);
            if (t.type == OP_AGN) {
                t = next(f);
            }
            node->type = parseType(t);
        }
        return node;
    };
    parseVarDecl = [&](Token&t)->AstVarDecl* {
        auto * node = new AstVarDecl;
        eat(KW_var, "it should be var declaration");
        if (t.type == OP_LPAREN) {
            do {
                node->varSpec.push_back(parseVarSpec(t));
                t = next(f);
            } while (t.type != OP_RPAREN);
            eat(OP_RPAREN, "expect )");
        }
        else {
            node->varSpec.push_back(parseVarSpec(t));
        }

        return node;
    };
    parseVarSpec = [&](Token&t)->AstNode* {
        AstVarSpec* node{};
        if (auto*tmp = parseIdentList(t); tmp != nullptr) {
            node = new AstVarSpec;
            node->identList = tmp;
            if(t.type == OP_AGN) {
                t = next(f);
                node->exprList = parseExprList(t);
            }
            else {
                node->type = parseType(t);
                if (t.type == OP_AGN) {
                    t = next(f);
                    node->exprList = parseExprList(t);
                }
            }
        }
        return node;
    };
    parseFuncDecl = [&](bool anonymous, Token&t)->AstFuncDecl* {
        auto * node = new AstFuncDecl;
        eat(KW_func, "it should be function declaration");
        if (!anonymous) {
            if (t.type == OP_LPAREN) {
                node->receiver = parseParam(t);
            }
            node->funcName = t.lexeme;
            t = next(f);
        }
        node->signature = parseSignature(t);
        nestLev++;
        node->funcBody = parseBlock(t);
        nestLev--;
        return node;
    };
    parseFuncType = [&](Token&t)->AstNode* {
        AstFuncType* node{};
        node = new AstFuncType;
        t = next(f);
        node->signature = parseSignature(t);
        return node;
    };
    parseSignature = [&](Token&t)->AstSignature* {
        AstSignature* node{};
        if (t.type == OP_LPAREN) {
            node = new AstSignature;
            node->param = parseParam(t);
            node->result = parseResult(t);
        }
        return node;
    };
    parseParam = [&](Token&t)->AstNode* {
        AstParam* node{};
        if (t.type == OP_LPAREN) {
            node = new AstParam;
            t = next(f);
            do {
                if (auto * tmp = parseParamDecl(t); tmp != nullptr) {
                    node->paramList.push_back(tmp);
                }
                if (t.type == OP_COMMA) {
                    t = next(f);
                }
            } while (t.type != OP_RPAREN);
            t = next(f);

            for (int i = 0, rewriteStart = 0; i < node->paramList.size(); i++) {
                if (dynamic_cast<AstParamDecl*>(node->paramList[i])->hasName) {
                    for (int k = rewriteStart; k < i; k++) {
                        string name = dynamic_cast<AstName*>(
                            dynamic_cast<AstParamDecl*>(node->paramList[k])->type)->name;
                        dynamic_cast<AstParamDecl*>(node->paramList[k])->type = dynamic_cast<AstParamDecl*>(node->paramList[i])->type;
                        dynamic_cast<AstParamDecl*>(node->paramList[k])->name = name;
                        dynamic_cast<AstParamDecl*>(node->paramList[k])->hasName = true; //It's not necessary
                    }
                    rewriteStart = i + 1;
                }
            }
        }
        return node;
    };
    parseParamDecl = [&](Token&t)->AstNode* {
        AstParamDecl* node{};
        if (t.type == OP_VARIADIC) {
            node = new AstParamDecl;
            node->isVariadic = true;
            t = next(f);
            node->type = parseType(t);
        }
        else if (t.type != OP_RPAREN) {
            node = new AstParamDecl;
            auto*mayIdentOrType = parseType(t);
            if (t.type != OP_COMMA && t.type != OP_RPAREN) {
                node->hasName = true;
                if (t.type == OP_VARIADIC) {
                    node->isVariadic = true;
                    t = next(f);
                }
                node->name = dynamic_cast<AstName*>(mayIdentOrType)->name;
                node->type = parseType(t);
            }
            else {
                node->type = mayIdentOrType;
            }
        }
        return node;
    };
    parseResult = [&](Token&t)->AstNode* {
        AstResult* node{};
        if (auto*tmp = parseParam(t); tmp != nullptr) {
            node = new AstResult;
            node->param = tmp;
        }
        else  if (auto*tmp = parseType(t); tmp != nullptr) {
            node = new AstResult;
            node->type = tmp;
        }
        return node;
    };
#pragma endregion
#pragma region Type
    parseType = [&](Token&t)->AstNode* {
        switch (t.type) {
        case TK_ID:       return parseName(true, t);
        case OP_LBRACKET: return parseArrayOrSliceType(t);
        case KW_struct:   return parseStructType(t);
        case OP_MUL:      return parsePtrType(t);
        case KW_func:     return parseFuncType(t);
        case KW_interface:return parseInterfaceType(t);
        case KW_map:      return parseMapType(t);
        case KW_chan:     return parseChanType(t);
        case OP_LPAREN:   {t = next(f); auto*tmp = parseType(t); t = next(f); return tmp; }
        default:return nullptr;
        }
    };
    parseArrayOrSliceType = [&](Token&t)->AstNode* {
        AstNode* node{};
        eat(OP_LBRACKET, "array/slice type requires [ to denote that");
        nestLev++;
        if (t.type != OP_RBRACKET) {
            node = new AstArrayType;
            if (t.type == OP_VARIADIC) {
                dynamic_cast<AstArrayType*>(node)->automaticLen = true;
                t = next(f);
            }
            else {
                dynamic_cast<AstArrayType*>(node)->length = parseExpr(t);
            } 
            nestLev--;
            t = next(f);
            dynamic_cast<AstArrayType*>(node)->elemType = parseType(t);
        }
        else {
            node = new AstSliceType;
            nestLev--;
            t = next(f);
            dynamic_cast<AstSliceType*>(node)->elemType = parseType(t);
        }
        return node;
    };
    parseStructType = [&](Token&t)->AstNode* {
        auto * node = new  AstStructType;
        eat(KW_struct, "structure type requires struct keyword");
        eat(OP_LBRACE, "a { is required after struct");
        do {
            tuple<AstNode*, AstNode*, string, bool> field;
            if (auto * tmp = parseIdentList(t); tmp != nullptr) {
                get<0>(field) = tmp;
                get<1>(field) = parseType(t);
                get<3>(field) = false;
            }
            else {
                if (t.type == OP_MUL) {
                    get<3>(field) = true;
                    t = next(f);
                }
                get<0>(field) = parseName(true, t);
            }
            if (t.type == LIT_STR) {
                get<2>(field) = t.lexeme;
            }
            node->fields.push_back(field);
            eatOptionalSemi();
        } while (t.type != OP_RBRACE);
        eat(OP_RBRACE, "expect }");
        eatOptionalSemi();
        return node;
    };
    parsePtrType = [&](Token&t)->AstNode* {
        auto * node = new AstPtrType;
        eat(OP_MUL, "pointer type requires * to denote that");
        node->baseType = parseType(t);
        return node;
    };
    parseInterfaceType = [&](Token&t)->AstNode* {
        auto * node = new AstInterfaceType;
        eat(KW_interface, "interface type requires keyword interface");
        eat(OP_LBRACE, "{ is required after interface");
        while (t.type != OP_RBRACE) {
            if (auto*tmp = parseMethodSpec(t); tmp != nullptr) {
                node->methodSpec.push_back(tmp);
                eatOptionalSemi();
            }
        }
        t = next(f);
        return node;
    };
    parseMethodSpec = [&](Token&t)->AstMethodSpec* {
        auto * node = new AstMethodSpec;
        if (auto* tmp = parseName(true, t); tmp != nullptr && tmp->name.find('.') == string::npos) {
            node->name = tmp;
            node->signature = parseSignature(t);
        }
        else {
            node->name = tmp;
        }
        return node;
    };
    parseMapType = [&](Token&t)->AstNode* {
        auto * node = new AstMapType;
        eat(KW_map, "map type requires keyword map to denote that");
        eat(OP_LBRACKET, "[ is required after map");
        node->keyType = parseType(t);
        eat(OP_RBRACKET, "] is required after map[Key");
        node->elemType = parseType(t);
        return node;
    };
    parseChanType = [&](Token&t)->AstNode* {
        AstChanType* node{};
        if (t.type == KW_chan) {
            node = new AstChanType;
            t = next(f);
            if (t.type == OP_CHAN) {
                t = next(f);
                node->elemType = parseType(t);
            }
            else {
                node->elemType = parseType(t);
            }
        }
        else if (t.type == OP_CHAN) {
            node = new AstChanType;
            t = next(f);
            if (t.type == KW_chan) {
                node->elemType = parseType(t);
            }
        }
        return node;
    };
#pragma endregion
#pragma region Statement
    parseStmt = [&](Token&t)->AstNode* {
        switch (t.type) {
        case KW_type:       { return parseTypeDecl(t);  }
        case KW_const:      { return parseConstDecl(t);  }
        case KW_var:        { return parseVarDecl(t);  }
        case KW_fallthrough:{t = next(f);  return new AstFallthroughStmt();  }
        case KW_go:         {t = next(f);  return new AstGoStmt(parseExpr(t));  }
        case KW_return:     {t = next(f);  return new AstReturnStmt(parseExprList(t));  }
        case KW_break:      {t = next(f);  return new AstBreakStmt(t.type == TK_ID ? t.lexeme : "");  }
        case KW_continue:   {t = next(f);  return new AstContinueStmt(t.type == TK_ID ? t.lexeme : "");  }
        case KW_goto:       {t = next(f);  return new AstGotoStmt(t.lexeme);  }
        case KW_defer:      {t = next(f);  return new AstDeferStmt(parseExpr(t));  }

        case KW_if:         return parseIfStmt(t);
        case KW_switch:     return parseSwitchStmt(t);
        case KW_select:     return parseSelectStmt(t);
        case KW_for:        return parseForStmt(t);
        case OP_LBRACE:     return parseBlock(t);
        case OP_SEMI:       return nullptr;

        case OP_ADD:case OP_SUB:case OP_NOT:case OP_XOR:case OP_MUL:case OP_CHAN:
        case LIT_STR:case LIT_INT:case LIT_IMG:case LIT_FLOAT:case LIT_RUNE:
        case KW_func:
        case KW_struct:case KW_map:case OP_LBRACKET:case TK_ID: case OP_LPAREN:
        {
            auto* exprList = parseExprList(t);
            if (t.type == OP_COLON) {
                //it shall a labeled statement(not part of simple stmt so we handle it here)
                t = next(f);
                auto* labeledStmt = new AstLabeledStmt;
                labeledStmt->label = dynamic_cast<AstName*>(
                    dynamic_cast<AstPrimaryExpr*>(
                        exprList->exprList[0]->lhs->expr)->expr)->name;
                labeledStmt->stmt = parseStmt(t);
                return labeledStmt;
            }
            else return parseSimpleStmt(exprList, t);
        }
        }
        return nullptr;
    };
    parseSimpleStmt = [&](AstExprList* lhs, Token&t)->AstNode* {
        if (t.type == KW_range){    //special case for ForStmt
            auto*stmt = new AstSRangeClause;
            t = next(f);
            stmt->rhs = parseExpr(t);
            return stmt;
        }
        if (lhs == nullptr) lhs = parseExprList(t);

        switch (t.type) {
        case OP_CHAN: {
            if (lhs->exprList.size() != 1) throw runtime_error("one expr required");
            t = next(f);
            auto* stmt = new AstSendStmt;
            stmt->receiver = lhs->exprList[0];
            stmt->sender = parseExpr(t);
            return stmt;
        }
        case OP_INC:case OP_DEC: {
            if (lhs->exprList.size() != 1) throw runtime_error("one expr required");
            auto* stmt = new AstIncDecStmt;
            stmt->isInc = t.type == OP_INC;
            t = next(f);
            stmt->expr = lhs->exprList[0];
            return stmt;
        }
        case OP_SHORTAGN: {
            if (lhs->exprList.empty()) throw runtime_error("one expr required");

            vector<string> identList;
            for (auto* e : lhs->exprList) {
                string identName =
                    dynamic_cast<AstName*>(
                        dynamic_cast<AstPrimaryExpr*>(
                            e->lhs->expr)->expr)->name;

                identList.push_back(identName);
            }
            t = next(f);
            if (t.type == KW_range) {
                t = next(f);
                auto* stmt = new AstSRangeClause;
                stmt->lhs = move(identList);
                stmt->rhs = parseExpr(t);
                return stmt;
            }
            else {
                auto*stmt = new AstSAssignStmt;
                stmt->lhs = move(identList);
                stmt->rhs = parseExprList(t);
                return stmt;
            }
        }
        case OP_ADDAGN:case OP_SUBAGN:case OP_BITORAGN:case OP_BITXORAGN:case OP_MULAGN:case OP_DIVAGN:
        case OP_MODAGN:case OP_LSFTAGN:case OP_RSFTAGN:case OP_BITANDAGN:case OP_ANDXORAGN:case OP_AGN: {
            if (lhs->exprList.empty()) throw runtime_error("one expr required");
            auto op = t.type;
            t = next(f);
            if (t.type == KW_range) {
                t = next(f);
                auto* stmt = new AstRangeClause;
                stmt->lhs = lhs;
                stmt->op = op;
                stmt->rhs = parseExpr(t);
                return stmt;
            }
            else {
                auto* stmt = new AstAssignStmt;
                stmt->lhs = lhs;
                stmt->op = op;
                stmt->rhs = parseExprList(t);
                return stmt;
            }

        }
        default: {//ExprStmt
            if (lhs->exprList.size() != 1) throw runtime_error("one expr required");
            auto* stmt = new AstExprStmt;
            stmt->expr = lhs->exprList[0];
            return stmt;
        }
        }
    };
    parseBlock = [&](Token&t)->AstStmtList* {
        AstStmtList * node{};
        if (t.type == OP_LBRACE) {
            t = next(f);
            if (t.type != OP_RBRACE) {
                node = parseStmtList(t);
                eat(OP_RBRACE, "expect } around code block");
            }
            else {
                t = next(f);
            }
        }
        return node;
    };
    parseIfStmt = [&](Token&t)->AstIfStmt* {
        const int outLev = nestLev;
        nestLev = -1;
        eat(KW_if, "expect keyword if");
        auto * node = new AstIfStmt;
        if (t.type == OP_LBRACE) throw runtime_error("if statement requires a condition");
        auto* tmp = parseSimpleStmt(nullptr, t);
        if (t.type == OP_SEMI) {
            node->init = tmp;
            t = next(f);
            node->cond = parseExpr(t);
        }
        else {
            node->cond = dynamic_cast<AstExprStmt*>(tmp)->expr;
        }
        node->ifBlock = parseBlock(t);
        if (t.type == KW_else) {
            t = next(f);
            if (t.type == KW_if) {
                node->elseBlock = parseIfStmt(t);
            }
            else if (t.type == OP_LBRACE) {
                node->elseBlock = parseBlock(t);
            }
            else throw runtime_error("only else-if or else could place here");
        }
        nestLev = outLev;
        return node;
    };
    parseSwitchStmt = [&](Token&t)->AstSwitchStmt* {
        const int outLev = nestLev;
        nestLev = -1;
        eat(KW_switch, "expect keyword switch");
        auto * node = new AstSwitchStmt;
        if (t.type != OP_LBRACE) {
            node->init = parseSimpleStmt(nullptr, t);
            if (t.type == OP_SEMI) t = next(f); 
            if (t.type != OP_LBRACE) node->cond = parseSimpleStmt(nullptr, t);
        }
        eat(OP_LBRACE, "expec { after switch header");
        do {
            if (auto*tmp = parseSwitchCase(t); tmp != nullptr) {
                node->caseList.push_back(tmp);
            }
        } while (t.type != OP_RBRACE);
        t = next(f);
        nestLev = outLev;
        return node;
    };
    parseSwitchCase = [&](Token&t)->AstSwitchCase* {
        AstSwitchCase* node{};
        if (t.type == KW_case) {
            node = new AstSwitchCase;
            t = next(f);
            node->exprList = parseExprList(t);
            eat(OP_COLON, "statements in each case requires colon to separate");
            node->stmtList = parseStmtList(t);
        }
        else if (t.type == KW_default) {
            node = new AstSwitchCase;
            t = next(f);
            eat(OP_COLON, "expect : after default label");
            node->stmtList = parseStmtList(t);
        }
        return node;
    };
    parseSelectStmt = [&](Token&t)->AstNode* {
        const int outLev = nestLev;
        nestLev = -1;
        eat(KW_select, "expect keyword select");
        eat(OP_LBRACE, "expect { after select keyword");
        auto* node = new AstSelectStmt;
        do {
            if (auto*tmp = parseSelectCase(t); tmp != nullptr) {
                node->caseList.push_back(tmp);
            }
        } while (t.type != OP_RBRACE);
        t = next(f);
        nestLev = outLev;
        return node;
    };
    parseSelectCase = [&](Token&t)->AstSelectCase* {
        AstSelectCase* node{};
        if (t.type == KW_case) {
            node = new AstSelectCase;
            t = next(f);
            auto*tmp = parseSimpleStmt(nullptr, t);
            eat(OP_COLON, "statements in each case requires colon to separate");
            node->stmtList = parseStmtList(t);
        }
        else if (t.type == KW_default) {
            node = new AstSelectCase;
            t = next(f);
            eat(OP_COLON, "expect : after default label");
            node->stmtList = parseStmtList(t);
        }
        return node;
    };
    parseForStmt = [&](Token&t)->AstNode* {
        const int outLev = nestLev;
        nestLev = -1;
        eat(KW_for, "expect keyword for");
        auto* node = new AstForStmt;
        if (t.type != OP_LBRACE) {
            if (t.type != OP_SEMI) {
                auto*tmp = parseSimpleStmt(nullptr, t);
                switch (t.type) {
                case OP_LBRACE:
                    node->cond = tmp;
                    if (typeid(*tmp) == typeid(AstSRangeClause) || typeid(*tmp) == typeid(AstRangeClause))
                        nestLev = outLev;
                    break;
                case OP_SEMI:
                    node->init = tmp;
                    eat(OP_SEMI, "for syntax are as follows: [init];[cond];[post]{...}");
                    node->cond = parseExpr(t);
                    eat(OP_SEMI, "for syntax are as follows: [init];[cond];[post]{...}");
                    if (t.type != OP_LBRACE)node->post = parseSimpleStmt(nullptr, t);  
                    break;
                default:throw runtime_error("expect {/;/range/:=/=");
                }
            }
            else {
                // for ;cond;post{}
                t = next(f);
                node->init = nullptr;
                node->cond = parseExpr(t);
                eat(OP_SEMI, "for syntax are as follows: [init];[cond];[post]{...}");
                if (t.type != OP_LBRACE)node->post = parseSimpleStmt(nullptr, t);
            }
        }
        node->block = parseBlock(t);
        nestLev = outLev;
        return node;
    };
#pragma endregion
#pragma region Expression
    parseExpr = [&](Token&t)->AstExpr* {
        AstExpr* node{};
        if (auto*tmp = parseUnaryExpr(t); tmp != nullptr) {
            node = new  AstExpr;
            node->lhs = tmp;
            if (t.type == OP_OR || t.type == OP_AND || t.type == OP_EQ || t.type == OP_NE || t.type == OP_LT || t.type == OP_LE || t.type == OP_XOR ||
                t.type == OP_GT || t.type == OP_GE || t.type == OP_ADD || t.type == OP_SUB || t.type == OP_BITOR || t.type == OP_XOR || t.type == OP_ANDXOR ||
                t.type == OP_MUL || t.type == OP_DIV || t.type == OP_MOD || t.type == OP_LSHIFT || t.type == OP_RSHIFT || t.type == OP_BITAND ) {
                node->op = t.type;
                t = next(f);
                node->rhs = parseExpr(t);
            }
        }
        return node;
    };
    parseUnaryExpr = [&](Token&t)->AstUnaryExpr* {
        AstUnaryExpr* node{};
        switch (t.type) {
        case OP_ADD:case OP_SUB:case OP_NOT:
        case OP_XOR:case OP_MUL:case OP_BITAND:case OP_CHAN:
            node = new AstUnaryExpr;
            node->op = t.type;
            t = next(f);
            node->expr = parseUnaryExpr(t);
            break;
        case TK_ID:
        case LIT_INT:case LIT_FLOAT:case LIT_IMG:case LIT_RUNE:case LIT_STR:
        case KW_struct:case KW_map:case OP_LBRACKET:case KW_chan:
        case KW_interface:case KW_func:case OP_LPAREN:
            node = new AstUnaryExpr;
            node->expr = parsePrimaryExpr(t);
            break;
        default:break;
        }
        return node;
    };
    parsePrimaryExpr = [&](Token&t)->AstPrimaryExpr* {
        AstPrimaryExpr*node{};
        if (auto*tmp = parseOperand(t); tmp != nullptr) {
            node = new AstPrimaryExpr;
            // eliminate left-recursion by infinite loop; these code referenced golang official impl
            // since this work requires somewhat familiarity of golang syntax
            while (true) {
                if (t.type == OP_DOT) {
                    t = next(f);
                    if (t.type == TK_ID) {
                        auto* e = new AstSelectorExpr;
                        e->operand = tmp;
                        e->selector = t.lexeme;
                        tmp = e;
                        t = next(f);
                    }
                    else if (t.type == OP_LPAREN) {
                        t = next(f);
                        if (t.type == KW_type) {
                            auto* e = new AstTypeSwitchExpr;
                            e->operand = tmp;
                            tmp = e;
                            t = next(f);
                        }
                        else {
                            auto* e = new AstTypeAssertionExpr;
                            e->operand = tmp;
                            e->type = parseType(t);
                            tmp = e;
                        }
                        eat(OP_RPAREN, "expect )");
                    }
                }
                else if (t.type == OP_LBRACKET) {
                    nestLev++;
                    t = next(f);
                    AstNode* start = nullptr;//Ignore start if next token is :(syntax of operand[:xxx])
                    if (t.type != OP_COLON) {
                        start = parseExpr(t);
                        if (t.type == OP_RBRACKET) {
                            auto* e = new AstIndexExpr;
                            e->operand = tmp;
                            e->index = start;
                            tmp = e;
                            t = next(f);
                            nestLev--;
                            continue;
                        }
                    }
                    auto* e = new AstSliceExpr;
                    e->operand = tmp;
                    e->begin = start;
                    eat(OP_COLON, "expect : in slice expression");
                    e->end = parseExpr(t);//may nullptr
                    if (t.type == OP_COLON) {
                        t = next(f);
                        e->step = parseExpr(t);
                        eat(OP_RBRACKET, "expect ] at the end of slice expression");
                    }
                    else if (t.type == OP_RBRACKET) {
                        t = next(f);
                    }
                    tmp = e;
                    nestLev--;
                }
                else if (t.type == OP_LPAREN) {
                    t = next(f);
                    auto* e = new AstCallExpr;
                    e->operand = tmp;
                    nestLev++;
                    if (auto*tmp1 = parseExprList(t); tmp1 != nullptr) {
                        e->arguments = tmp1;
                    }
                    if (t.type == OP_VARIADIC) {
                        e->isVariadic = true;
                        t = next(f);
                    }
                    nestLev--;
                    eat(OP_RPAREN, "() must match in call expr");
                    tmp = e;
                }
                else if (t.type == OP_LBRACE) {
                    // only operand has literal value, otherwise, treats it as a block
                    if (typeid(*tmp) == typeid(AstArrayType) ||typeid(*tmp) == typeid(AstSliceType) ||
                        typeid(*tmp) == typeid(AstStructType) ||typeid(*tmp) == typeid(AstMapType) ||
                        ((typeid(*tmp) == typeid(AstName) || typeid(*tmp) == typeid(AstSelectorExpr)) && nestLev >= 0)) {
                        // it's somewhat curious since official implementation treats literal type
                        // and literal value as separate parts
                        auto* e = new AstCompositeLit;
                        e->litName = tmp;
                        e->litValue = parseLitValue(t);
                        tmp = e;
                    }
                    else break; 
                }
                else break;
            }
            node->expr = tmp;
        }
        return node;
    };
    parseOperand = [&](Token&t)->AstNode* {
        switch (t.type) {
        case TK_ID:
            return parseName(false, t);
        case LIT_INT:case LIT_FLOAT:case LIT_IMG:case LIT_RUNE:case LIT_STR:
        {auto*tmp = new AstBasicLit(t.type, t.lexeme); t = next(f); return tmp; }
        case KW_struct:case KW_map:case OP_LBRACKET:case KW_chan:case KW_interface:
            return parseType(t);
        case KW_func:
            return parseFuncDecl(true, t);
        case OP_LPAREN: {
            t = next(f); nestLev++; auto* e = parseExpr(t); nestLev--; eat(OP_RPAREN, "expect )"); return e;
        }
        default:return nullptr;
        }
    };
    parseLitValue = [&](Token&t)->AstLitValue* {
        AstLitValue*node{};
        if (t.type == OP_LBRACE) {
            nestLev++;
            node = new AstLitValue;
            do {
                t = next(f);
                if (t.type == OP_RBRACE) break; // it's necessary since both {a,b} or {a,b,} are legal form
                node->keyedElement.push_back(parseKeyedElement(t));
            } while (t.type != OP_RBRACE);
            eat(OP_RBRACE, "brace {} must match");
            nestLev--;
        }
        return node;
    };
    parseKeyedElement = [&](Token&t)->AstNode* {
        AstKeyedElement*node{};
        if (auto*tmp = parseKey(t); tmp != nullptr) {
            node = new AstKeyedElement;
            node->element = tmp;
            if (t.type == OP_COLON) {
                node->key = tmp;
                t = next(f);
                if (auto*tmp1 = parseExpr(t); tmp1 != nullptr)          node->element = tmp1;
                else if (auto*tmp1 = parseLitValue(t); tmp1 != nullptr) node->element = tmp1;
                else node->element = nullptr;
            }
        }
        return node;
    };
    parseKey = [&](Token&t)->AstNode* {
        if (t.type == TK_ID) return parseName(false, t);
        else if (auto*tmp = parseLitValue(t); tmp != nullptr)return tmp;
        else if (auto*tmp = parseExpr(t); tmp != nullptr) return tmp;
        return nullptr;
    };
#pragma endregion
    // parsing startup
    return parseSourceFile(t);
}

void emitStub() {}

void runtimeStub() {}

//===----------------------------------------------------------------------===//
// debug auxiliary functions, they are not part of 5 functions
//===----------------------------------------------------------------------===//
void printLex(const string & filename) {
    fstream f(filename, ios::binary | ios::in);
    while (lastToken != TK_EOF) {
        auto[token, lexeme] = next(f);
        fprintf(stdout, "<%d,%s,%d,%d>\n", token, lexeme.c_str(), line, column);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2 || argv[1] == nullptr) {
        fprintf(stderr, "specify your go source file\n");
        return 1;
    }
    printLex(argv[1]);
    const AstNode* ast = parse(argv[1]);
    fprintf(stdout, "parsing passed\n");
    return 0;
}