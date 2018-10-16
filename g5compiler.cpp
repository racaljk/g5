//===----------------------------------------------------------------------===//
// Golang specification can be found here: https://golang.org/ref/spec
//
// In development, I consider raise runtime_error  since it's helpful to locate 
// where error occurred and do further debugging. 
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
#define ASTNODE :public AstNode
using namespace std;


//===----------------------------------------------------------------------===//
// various declarations 
//===----------------------------------------------------------------------===//
string keywords[] = { "break",    "default",     "func",   "interface", "select",
                     "case",     "defer",       "go",     "map",       "struct",
                     "chan",     "else",        "goto",   "package",   "switch",
                     "const",    "fallthrough", "if",     "range",     "type",
                     "continue", "for",         "import", "return",    "var" };

enum TokenType : signed int {
    KW_break, KW_default, KW_func, KW_interface, KW_select, KW_case, KW_defer,
    KW_go, KW_map, KW_struct, KW_chan, KW_else, KW_goto, KW_package, KW_switch,
    KW_const, KW_fallthrough, KW_if, KW_range, KW_type, KW_continue, KW_for,
    KW_import, KW_return, KW_var, OP_ADD, OP_BITAND, OP_ADDAGN, OP_BITANDAGN,
    OP_AND, OP_EQ, OP_NE, OP_LPAREN, OP_RPAREN, OP_SUB, OP_BITOR, OP_SUBAGN,
    OP_BITORAGN, OP_OR, OP_LT, OP_LE, OP_LBRACKET, OP_RBRACKET, OP_MUL, OP_XOR,
    OP_MULAGN, OP_BITXORAGN, OP_CHAN, OP_GT, OP_GE, OP_LBRACE, OP_RBRACE,
    OP_DIV, OP_LSHIFT, OP_DIVAGN, OP_LSFTAGN, OP_INC, OP_AGN, OP_SHORTAGN,
    OP_COMMA, OP_SEMI, OP_MOD, OP_RSHIFT, OP_MODAGN, OP_RSFTAGN, OP_DEC,
    OP_NOT, OP_VARIADIC, OP_DOT, OP_COLON, OP_ANDXOR, OP_ANDXORAGN, TK_ID,
    LITERAL_INT, LITERAL_FLOAT, LITERAL_IMG, LITERAL_RUNE, LITERAL_STR, TK_EOF
};
//todo: add destructors for these structures
// List parsing
struct AstNode { virtual ~AstNode() {} };
struct AstIdentifierList ASTNODE { vector<string> identifierList; };
struct AstExpressionList ASTNODE { vector<AstNode*> expressionList; };
struct AstStatementList ASTNODE { vector<AstNode*> statements; };
// Declaration
struct AstSourceFile ASTNODE {
    vector<AstNode*> importDecl;
    vector<AstNode*> topLevelDecl;
};
struct AstPackageClause ASTNODE { string packageName; };
struct AstImportDecl ASTNODE { map<string, string> imports; };
struct AstTopLevelDecl ASTNODE {
    union {
        AstNode* decl;
        AstNode* functionDecl;
        AstNode* methodDecl;
    }atld;
};
struct AstDeclaration ASTNODE {
    union {
        AstNode* constDecl;
        AstNode* typeDecl;
        AstNode* varDecl;
    }ad;
};
struct AstConstDecl ASTNODE {
    vector<AstNode*> identifierList;
    vector<AstNode*> type;
    vector<AstNode*> expressionList;
};
struct AstTypeDecl ASTNODE { vector<AstNode*> typeSpec; };
struct AstTypeSpec ASTNODE {
    string identifier;
    AstNode* type;
};
struct AstVarDecl ASTNODE { vector<AstNode*> varSpec; };
struct AstVarSpec ASTNODE {
    AstNode* identifierList;
    union {
        struct {
            AstNode* type;
            AstNode* expressionList;
        }named;
        AstNode* expressionList;
    }avs;
};
struct AstFunctionDecl ASTNODE {
    string funcName;
    AstNode* receiver;
    AstNode* signature;
    AstNode* functionBody;
};
// Type
struct AstType ASTNODE {
    AstNode* type;
};
struct AstTypeName ASTNODE { string typeName; };
struct AstArrayType ASTNODE {
    AstNode* length;
    AstNode* elementType;
};
struct AstStructType ASTNODE {
    union _FieldDecl {
        struct {
            AstNode* identifierList;
            AstNode* type;
        }named;
        AstNode* typeName;
    };

    vector<tuple<_FieldDecl, string>> fields;
};
struct AstPointerType ASTNODE { AstNode * baseType; };
struct AstFunctionType ASTNODE { AstNode * signature; };
struct AstSignature ASTNODE {
    AstNode* parameters;
    AstNode* result;
};
struct AstParameter ASTNODE { vector<AstNode*> parameterList; };
struct AstParameterDecl ASTNODE {
    bool isVariadic = false;
    bool hasName = false;
    AstNode* type;
    string name;
};
struct AstResult ASTNODE {
    union {
        AstNode* parameter;
        AstNode* type;
    }ar;
};
struct AstInterfaceType ASTNODE { vector<AstNode*> methodSpec; };
struct AstMethodSpec ASTNODE {
    union {
        struct _MethodSignature {
            AstNode* methodName;
            AstNode* signature;
        }named;
        AstNode* interfaceTypeName;
    }ams;
};
struct AstMethodName ASTNODE { string methodName; };
struct AstSliceType ASTNODE { AstNode* elementType; };
struct AstMapType ASTNODE {
    AstNode* keyType;
    AstNode* elementType;
};
struct AstChannelType ASTNODE { AstNode* elementType; };
// Statement

struct AstStatement ASTNODE {
    AstNode* stmt;
};
struct AstBlock ASTNODE { AstNode* statementList; };
struct AstLabeledStmt ASTNODE {
    string identifier;
    AstNode* statement;
};
struct AstSimpleStmt ASTNODE {
    union {
        AstNode* expressionStmt;
        AstNode* sendStmt;
        AstNode* incDecStmt;
        AstNode* assignment;
        AstNode* shortVarDecl;
    }ass;
};
struct AstGoStmt ASTNODE { AstNode* expression; };
struct AstReturnStmt ASTNODE { AstNode* expressionList; };
struct AstBreakStmt ASTNODE { string label; };
struct AstContinueStmt ASTNODE { string label; };
struct AstGotoStmt ASTNODE { string label; };
struct AstFallthroughStmt ASTNODE {};
struct AstIfStmt ASTNODE {
    AstNode* condition;
    AstNode* expression;
    AstNode* block;
    union {
        AstNode* ifStmt;
        AstNode* block;
    }ais;
};
struct AstSwitchStmt ASTNODE {
    AstNode* condition;
    AstNode* conditionExpr;
    vector<AstNode*> exprCaseClause;
};
struct AstExprCaseClause ASTNODE {
    AstNode* exprSwitchCase;
    AstNode* statementList;
};
struct AstExprSwitchCase ASTNODE {
    AstNode * expressionList;
    bool isDefault;
};
struct AstSelectStmt ASTNODE {
    vector<AstNode*> commClause;
};
struct AstCommClause ASTNODE {
    AstNode* commCase;
    AstNode* statementList;
};
struct AstCommCase ASTNODE {
    union {
        AstNode* sendStmt;
        AstNode* recvStmt;
    }acc;
    bool isDefault;
};
struct AstRecvStmt ASTNODE {
    union {
        AstNode* identifierList;
        AstNode* expressionList;
    }ars;
    AstNode* recvExpr;
};
struct AstForStmt ASTNODE {
    union {
        AstNode* condition;
        AstNode* forClause;
        AstNode* rangeClause;
    }afs;
    AstNode* block;
};
struct AstForClause ASTNODE {
    AstNode* initStmt;
    AstNode* condition;
    AstNode* postStmt;
};
struct AstRangeClause ASTNODE {
    union {
        AstNode* expressionList;
        AstNode* identifierList;
    }arc;
    AstNode* expression;
};
struct AstDeferStmt ASTNODE { AstNode* expression; };
struct AstExpressionStmt ASTNODE { AstNode* expression; };
struct AstSendStmt ASTNODE {
    AstNode* receiver;
    AstNode* sender;
};
struct AstIncDecStmt ASTNODE {
    AstNode* expression;
    bool isInc;
};
struct AstAssignment ASTNODE {
    AstNode* lhs;
    AstNode* rhs;
    TokenType assignOp;
};
struct AstShortVarDecl ASTNODE {
    AstNode* lhs;
    AstNode* rhs;
};
// Expression
struct AstExpression ASTNODE {
    union {
        struct {
            AstNode* lhs;
            TokenType binaryOp;
            AstNode* rhs;
        }named;
        AstNode* unaryExpr;
    }ae;
};
struct AstUnaryExpr ASTNODE {
    union {
        AstNode*primaryExpr;
        struct {
            AstNode* unaryExpr;
            TokenType unaryOp;
        }named;
    }aue;
};
struct AstPrimaryExpr ASTNODE {
    AstNode* expr;
};
struct AstSelectorExpr ASTNODE {
    AstNode* operand;
    string selector;
};
struct AstTypeSwitchGuardExpr ASTNODE {
    AstNode* operand;
    // AstNode* lhs;
};
struct AstTypeAssertionExpr ASTNODE {
    AstNode* operand;
    AstNode* type;
};
struct AstIndexExpr ASTNODE {
    AstNode* operand;
    AstNode* index;
};
struct AstSliceExpr ASTNODE {
    AstNode* operand;
    AstNode* begin;
    AstNode* end;
    AstNode* step;
};
struct AstCallExpr ASTNODE {
    AstNode* operand;
    AstNode* arguments;
    AstNode* type;
    bool isVariadic;
};
struct AstOperand ASTNODE {
    union {
        AstNode*literal;
        AstNode*operandName;
        AstNode*expression;
    }ao;
};
struct AstOperandName ASTNODE { string operandName; };
struct AstLiteral ASTNODE {
    union {
        AstNode*basicLit;
        AstNode*compositeLit;
        AstNode*functionLit;
    }al;
};
struct AstBasicLit ASTNODE { TokenType type; string value; };
struct AstCompositeLit ASTNODE {
    union {
        AstNode* structType;
        struct {
            AstNode* elementType;
            bool automaticLength;
        }automaticLengthArrayType;
        struct {
            AstNode* arrayLength;
            AstNode* elementType;
        }arrayType;
        AstNode* sliceType;
        AstNode* mapType;
        AstNode* typeName;
    }acl;
    AstNode*literalValue;
};
struct AstLiteralValue ASTNODE { vector< AstNode*> keyedElement; };
struct AstKeyedElement ASTNODE {
    AstNode*key;
    AstNode*element;
};
struct AstKey ASTNODE {
    union {
        AstNode* fieldName;
        AstNode* expression;
        AstNode* literalValue;
    }ak;
};
struct AstFieldName ASTNODE {
    string fieldName;
};
struct AstElement ASTNODE {
    union {
        AstNode*expression;
        AstNode*literalValue;
    }ae;
};
struct AstFunctionLit ASTNODE {
    AstNode*signature;
    AstNode*functionBody;
};
//===----------------------------------------------------------------------===//
// global data
//===----------------------------------------------------------------------===//
static int line = 1, column = 1, lastToken = -1, shouldEof = 0;
struct Token {
    TokenType type; string lexeme;
    Token(TokenType a, const string&b) :type(a), lexeme(b) {}
};
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
        c = f.peek();
        return oc;
    };
    char c = f.peek();

skip_comment_and_find_next:

    for (; c == ' ' || c == '\r' || c == '\t' || c == '\n'; column++) {
        if (c == '\n') {
            line++;
            column = 1;
            if ((lastToken >= TK_ID && lastToken <= LITERAL_STR)
                || lastToken == KW_fallthrough || lastToken == KW_continue
                || lastToken == KW_return || lastToken == KW_break
                || lastToken == OP_INC || lastToken == OP_DEC
                || lastToken == OP_RPAREN
                || lastToken == OP_RBRACKET || lastToken == OP_RBRACE) {
                consumePeek(c);
                lastToken = OP_SEMI;
                return Token(OP_SEMI, ";");
            }
        }
        consumePeek(c);
    }
    if (f.eof()) {
        if (shouldEof) {
            lastToken = TK_EOF;
            return Token(TK_EOF, "");
        }
        shouldEof = 1;
        lastToken = OP_SEMI;
        return Token(OP_SEMI, ";");
    }

    string lexeme;


    // identifier = letter { letter | unicode_digit } .
    if (isalpha(c) || c == '_') {
        while (isalnum(c) || c == '_') {
            lexeme += consumePeek(c);
        }

        for (int i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++)
            if (keywords[i] == lexeme) {
                lastToken = static_cast<TokenType>(i);
                return Token(static_cast<TokenType>(i), lexeme);
            }
        lastToken = TK_ID;
        return Token(TK_ID, lexeme);
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
                lastToken = LITERAL_INT;
                return Token(LITERAL_INT, lexeme);
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
                lastToken = LITERAL_INT;
                return Token(LITERAL_INT, lexeme);
            }
            goto may_float;
        }
        else {  // 1-9 or . or just a single 0
        may_float:
            TokenType type = LITERAL_INT;
            if (c == '.') {
                lexeme += consumePeek(c);
                if (c == '.') {
                    lexeme += consumePeek(c);
                    if (c == '.') {
                        lexeme += consumePeek(c);
                        lastToken = OP_VARIADIC;
                        return Token(OP_VARIADIC, lexeme);
                    }
                    else {
                        throw runtime_error(
                            "expect variadic notation(...) but got .." + c);
                    }
                }
                else if (c >= '0'&&c <= '9') {
                    type = LITERAL_FLOAT;
                }
                else {
                    lastToken = OP_DOT;
                    return Token(OP_DOT, ".");
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
                        type = LITERAL_FLOAT;
                    }
                    else if ((c == 'e' && !hasExponent) ||
                        (c == 'E' && !hasExponent)) {
                        hasExponent = true;
                        type = LITERAL_FLOAT;
                        lexeme += consumePeek(c);
                        if (c == '+' || c == '-') {
                            lexeme += consumePeek(c);
                        }
                    }
                    else {
                        f.get();
                        column++;
                        lexeme += c;
                        lastToken = LITERAL_IMG;
                        return Token(LITERAL_IMG, lexeme);
                    }
                }
                lastToken = type;
                return Token(type, lexeme);
            }
            else {
                lastToken = type;
                return Token(type, lexeme);
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
        lastToken = LITERAL_RUNE;
        return Token(LITERAL_RUNE, lexeme);
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
        lastToken = LITERAL_STR;
        return Token(LITERAL_STR, lexeme);
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
                "string literal does not have a closed symbol \"\"\"");
        }
        lexeme += consumePeek(c);
        lastToken = LITERAL_STR;
        return Token(LITERAL_STR, lexeme);
    }

    // operators
    switch (c) {
    case '+':  //+  += ++
        lexeme += consumePeek(c);
        if (c == '=') {
            lexeme += consumePeek(c);
            lastToken = OP_ADDAGN;
            return Token(OP_ADDAGN, lexeme);
        }
        else if (c == '+') {
            lexeme += consumePeek(c);
            lastToken = OP_INC;
            return Token(OP_INC, lexeme);
        }
        return Token(OP_ADD, lexeme);
    case '&':  //&  &=  &&  &^  &^=
        lexeme += consumePeek(c);
        if (c == '=') {
            lexeme += consumePeek(c);
            lastToken = OP_BITANDAGN;
            return Token(OP_BITANDAGN, lexeme);
        }
        else if (c == '&') {
            lexeme += consumePeek(c);
            lastToken = OP_AND;
            return Token(OP_AND, lexeme);
        }
        else if (c == '^') {
            lexeme += consumePeek(c);
            if (c == '=') {
                lexeme += consumePeek(c);
                lastToken = OP_ANDXORAGN;
                return Token(OP_ANDXORAGN, lexeme);
            }
            lastToken = OP_ANDXOR;
            return Token(OP_ANDXOR, lexeme);
        }
        lastToken = OP_BITAND;
        return Token(OP_BITAND, lexeme);
    case '=':  //=  ==
        lexeme += consumePeek(c);
        if (c == '=') {
            lastToken = OP_EQ;
            return Token(OP_EQ, lexeme);
        }
        lastToken = OP_AGN;
        return Token(OP_AGN, lexeme);
    case '!':  //!  !=
        lexeme += consumePeek(c);
        if (c == '=') {
            lexeme += consumePeek(c);
            lastToken = OP_NE;
            return Token(OP_NE, lexeme);
        }
        lastToken = OP_NOT;
        return Token(OP_NOT, lexeme);
    case '(':
        lexeme += consumePeek(c);
        lastToken = OP_LPAREN;
        return Token(OP_LPAREN, lexeme);
    case ')':
        lexeme += consumePeek(c);
        lastToken = OP_RPAREN;
        return Token(OP_RPAREN, lexeme);
    case '-':  //-  -= --
        lexeme += consumePeek(c);
        if (c == '=') {
            lexeme += consumePeek(c);
            lastToken = OP_SUBAGN;
            return Token(OP_SUBAGN, lexeme);
        }
        else if (c == '-') {
            lexeme += consumePeek(c);
            lastToken = OP_DEC;
            return Token(OP_DEC, lexeme);
        }
        lastToken = OP_SUB;
        return Token(OP_SUB, lexeme);
    case '|':  //|  |=  ||
        lexeme += consumePeek(c);
        if (c == '=') {
            lexeme += consumePeek(c);
            lastToken = OP_BITORAGN;
            return Token(OP_BITORAGN, lexeme);
        }
        else if (c == '|') {
            lexeme += consumePeek(c);
            lastToken = OP_OR;
            return Token(OP_OR, lexeme);
        }
        lastToken = OP_BITOR;
        return Token(OP_BITOR, lexeme);
    case '<':  //<  <=  <- <<  <<=
        lexeme += consumePeek(c);
        if (c == '=') {
            lexeme += consumePeek(c);
            lastToken = OP_LE;
            return Token(OP_LE, lexeme);
        }
        else if (c == '-') {
            lexeme += consumePeek(c);
            lastToken = OP_CHAN;
            return Token(OP_CHAN, lexeme);
        }
        else if (c == '<') {
            lexeme += consumePeek(c);
            if (c == '=') {
                lastToken = OP_LSFTAGN;
                return Token(OP_LSFTAGN, lexeme);
            }
            lastToken = OP_LSHIFT;
            return Token(OP_LSHIFT, lexeme);
        }
        lastToken = OP_LT;
        return Token(OP_LT, lexeme);
    case '[':
        lexeme += consumePeek(c);
        lastToken = OP_LBRACKET;
        return Token(OP_LBRACKET, lexeme);
    case ']':
        lexeme += consumePeek(c);
        lastToken = OP_RBRACKET;
        return Token(OP_RBRACKET, lexeme);
    case '*':  //*  *=
        lexeme += consumePeek(c);
        if (c == '=') {
            lastToken = OP_MULAGN;
            return Token(OP_MULAGN, lexeme);
        }
        lastToken = OP_MUL;
        return Token(OP_MUL, lexeme);
    case '^':  //^  ^=
        lexeme += consumePeek(c);
        if (c == '=') {
            lastToken = OP_BITXORAGN;
            return Token(OP_BITXORAGN, lexeme);
        }
        lastToken = OP_XOR;
        return Token(OP_XOR, lexeme);
    case '>':  //>  >=  >>  >>=
        lexeme += consumePeek(c);
        if (c == '=') {
            lexeme += consumePeek(c);
            lastToken = OP_GE;
            return Token(OP_GE, lexeme);
        }
        else if (c == '>') {
            lexeme += consumePeek(c);
            if (c == '=') {
                lastToken = OP_RSFTAGN;
                return Token(OP_RSFTAGN, lexeme);
            }
            lastToken = OP_RSHIFT;
            return Token(OP_RSHIFT, lexeme);
        }
        lastToken = OP_GT;
        return Token(OP_GT, lexeme);
    case '{':
        lexeme += consumePeek(c);
        lastToken = OP_LBRACE;
        return Token(OP_LBRACE, lexeme);
    case '}':
        lexeme += consumePeek(c);
        lastToken = OP_RBRACE;
        return Token(OP_RBRACE, lexeme);
    case '/': {  // /  /= // /*...*/
        char pending = consumePeek(c);
        if (c == '=') {
            lexeme += pending;
            lexeme += consumePeek(c);
            lastToken = OP_DIVAGN;
            return Token(OP_DIVAGN, lexeme);
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
        return Token(OP_DIV, lexeme);
    }
    case ':':  // :=
        lexeme += consumePeek(c);
        if (c == '=') {
            lexeme += consumePeek(c);
            lastToken = OP_SHORTAGN;
            return Token(OP_SHORTAGN, lexeme);
        }
        lastToken = OP_COLON;
        return Token(OP_COLON, lexeme);
    case ',':
        lexeme += consumePeek(c);
        lastToken = OP_COMMA;
        return Token(OP_COMMA, lexeme);
    case ';':
        lexeme += consumePeek(c);
        lastToken = OP_SEMI;
        return Token(OP_SEMI, lexeme);
    case '%':  //%  %=
        lexeme += consumePeek(c);
        if (c == '=') {
            lexeme += consumePeek(c);
            lastToken = OP_MODAGN;
            return Token(OP_MODAGN, lexeme);
        }
        lastToken = OP_MOD;
        return Token(OP_MOD, lexeme);
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

    auto expect = [&f, &t](TokenType tk, const string& msg) {
        t = next(f);
        if (t.type != tk) throw runtime_error(msg);
        return t;
    };

    function<AstNode*(Token&)> parseImportDecl, parseTopLevelDecl, parseTypeAssertion,
        parseDeclaration, parseConstDecl, parseIdentifierList, parseType, parseTypeName,
        parseArrayOrSliceType, parseStructType, parsePointerType, parseFunctionType,
        parseSignature, parseParameter, parseParameterDecl, parseResult, parseInterfaceType,
        parseMethodSpec, parseMethodName, parseSliceType, parseMapType, parseChannelType,
        parseTypeDecl, parseTypeSpec, parseVarDecl, parseVarSpec, parseFunctionDecl,
        parseStatementList, parseStatement, parseCompositeLit, parseFieldName, parseBasicLit,
        parseLabeledStmt, parseSimpleStmt, parseGoStmt, parseReturnStmt, parseBreakStmt,
        parseContinueStmt, parseGotoStmt, parseFallthroughStmt, parseBlock, parseIfStmt,
        parseSwitchStmt, parseSelectStmt, parseForStmt, parseDeferStmt, parseExpressionStmt,
        parseSendStmt, parseIncDecStmt, parseAssignment, parseShortVarDecl, parseExprCaseClause,
        parseExprSwitchCase, parseCommClause, parseCommCase, parseRecvStmt, parseForClause,
        parseRangeClause, parseSourceFile, parseExpressionList, parseExpression,
        parseUnaryExpr, parsePrimaryExpr, parseOperand, parseOperandName, parseLiteral, 
        parseLiteralValue, parseElementList, parseKeyedElement, parseKey, parseElement,
        parseFunctionLit;

    // List parsing
    parseIdentifierList = [&](Token&t)->AstNode* {
        AstIdentifierList* node = nullptr;
        if (t.type = TK_ID) {
            node = new  AstIdentifierList;
            node->identifierList.emplace_back(t.lexeme);
            t = next(f);
            while (t.type == OP_COMMA) {
                node->identifierList.emplace_back(expect(TK_ID, "it shall be an identifier").lexeme);
                t = next(f);
            }
        }
        return node;
    };
    parseExpressionList = [&](Token&t)->AstNode* {
        AstExpressionList* node = nullptr;
        if (auto* tmp = parseExpression(t); tmp != nullptr) {
            node = new  AstExpressionList;
            node->expressionList.emplace_back(tmp);
            while (t.type == OP_COMMA) {
                t = next(f);
                node->expressionList.emplace_back(parseExpression(t));
            }
        }
        return node;
    };
    parseStatementList = [&](Token&t)->AstNode* {
        AstStatementList * node = nullptr;
        if (auto * tmp = parseStatement(t); tmp != nullptr) {
            node = new AstStatementList;
            node->statements.push_back(tmp);
            if (t.type == OP_SEMI) {
                t = next(f);
            }
            if (t.type != OP_RBRACE) {
                while ((tmp = parseStatement(t))) {
                    node->statements.push_back(tmp);
                    eat(OP_SEMI, "statement should seperate by semicolon");
                    if (t.type == OP_RBRACE) {
                        break;
                    }
                }
            }
        }
        return node;
    };
    // Declaration
    parseSourceFile = [&](Token&t)->AstNode* {
        AstSourceFile * node = nullptr;
        if (t.type == KW_package) {
            grt.package = expect(TK_ID, "expect identifier").lexeme;
            node = new AstSourceFile;
            expect(OP_SEMI, "expect a semicolon after package declaration");
            t = next(f);
            while (t.type == KW_import) {
                node->importDecl.push_back(parseImportDecl(t));
                if (t.type == OP_SEMI) {
                    t = next(f);
                }
                t = next(f);
            }
            while (t.type != TK_EOF) {
                node->topLevelDecl.push_back(parseTopLevelDecl(t));
                t = next(f);
            }
        }


        return node;
    };
    parseImportDecl = [&](Token&t)->AstNode* {
        if (t.type == KW_import) {
            auto node = new AstImportDecl;
            t = next(f);
            if (t.type == OP_LPAREN) {
                t = next(f);
                do {
                    string importName, alias;
                    if (t.type == OP_DOT || t.type == TK_ID) {
                        alias = t.lexeme;
                        importName = expect(LITERAL_STR, "import path should not empty").lexeme;
                    }
                    else {
                        importName = t.lexeme;
                    }
                    importName = importName.substr(1, importName.length() - 2);
                    expect(OP_SEMI, "expect an explicit semicolon after import declaration");
                    node->imports[importName] = alias;
                    t = next(f);
                } while (t.type != OP_RPAREN);
            }
            else {
                string importName, alias;
                if (t.type == OP_DOT || t.type == TK_ID) {
                    alias = t.lexeme;
                    importName = expect(LITERAL_STR, "import path should not empty").lexeme;
                }
                else {
                    importName = t.lexeme;
                }
                importName = importName.substr(1, importName.length() - 2);
                node->imports[importName] = alias;
            }
            return node;
        }
        return nullptr;
    };
    parseTopLevelDecl = [&](Token&t)->AstNode* {
        AstTopLevelDecl* node = nullptr;
        if (auto* tmp = parseDeclaration(t); tmp != nullptr) {
            node = new AstTopLevelDecl;
            node->atld.decl = tmp;
        }
        else if (auto* tmp = parseFunctionDecl(t); tmp != nullptr) {
            node = new AstTopLevelDecl;
            node->atld.functionDecl = tmp;
        }
        return node;
    };
    parseDeclaration = [&](Token&t)->AstNode* {
        AstDeclaration * node = nullptr;
        if (auto*tmp = parseConstDecl(t); tmp != nullptr) {
            node = new AstDeclaration;
            node->ad.constDecl = tmp;
        }
        else  if (auto*tmp = parseTypeDecl(t); tmp != nullptr) {
            node = new AstDeclaration;
            node->ad.typeDecl = tmp;
        }
        else  if (auto*tmp = parseVarDecl(t); tmp != nullptr) {
            node = new AstDeclaration;
            node->ad.varDecl = tmp;
        }
        return node;
    };
    parseConstDecl = [&](Token&t)->AstNode* {
        AstConstDecl * node = nullptr;
        if (t.type == KW_const) {
            node = new AstConstDecl;
            t = next(f);
            if (t.type == OP_LPAREN) {
                t = next(f);
                do {
                    node->identifierList.push_back(parseIdentifierList(t));
                    if (auto*tmp = parseType(t); tmp != nullptr) {
                        node->type.push_back(tmp);
                    }
                    else {
                        node->type.push_back(nullptr);
                    }
                    if (t.type == OP_AGN) {
                        t = next(f);
                        node->expressionList.push_back(parseExpressionList(t));
                    }
                    else {
                        node->expressionList.push_back(nullptr);
                    }
                    eat(OP_SEMI, "expect an explicit semicolon");
                } while (t.type != OP_RPAREN);
                eat(OP_RPAREN, "eat right parenthesis");
            }
            else {
                node->identifierList.push_back(parseIdentifierList(t));
                if (auto*tmp = parseType(t); tmp != nullptr) {
                    node->type.push_back(tmp);
                    t = next(f);
                }
                else {
                    node->type.push_back(nullptr);
                }
                if (t.type == OP_AGN) {
                    t = next(f);
                    node->expressionList.push_back(parseExpressionList(t));
                }
                else {
                    node->expressionList.push_back(nullptr);
                }
                if (t.type != OP_SEMI) {

                    throw runtime_error("expect an explicit semicolon");
                }
            }
            eat(OP_SEMI, "expect ;");
        }
        return node;
    };
    parseTypeDecl = [&](Token&t)->AstNode* {
        AstTypeDecl* node = nullptr;
        if (t.type == KW_type) {
            node = new AstTypeDecl;
            t = next(f);
            if (t.type == OP_LPAREN) {
                t = next(f);
                do {
                    node->typeSpec.push_back(parseTypeSpec(t));
                    if (t.type == OP_SEMI) {
                        t = next(f);
                    }
                } while (t.type != OP_RPAREN);
            }
            else {
                node->typeSpec.push_back(parseTypeSpec(t));
            }
        }
        return node;
    };
    parseTypeSpec = [&](Token&t)->AstNode* {
        AstTypeSpec* node = nullptr;
        if (t.type == TK_ID) {
            node = new AstTypeSpec;
            node->identifier = t.lexeme;
            t = next(f);
            if (t.type == OP_AGN) {
                t = next(f);
            }
            node->type = parseType(t);
        }
        return node;
    };
    parseVarDecl = [&](Token&t)->AstNode* {
        AstVarDecl* node = nullptr;
        if (t.type == KW_var) {
            node = new AstVarDecl;
            t = next(f);
            if (t.type == OP_LPAREN) {
                do {
                    node->varSpec.push_back(parseVarSpec(t));
                    expect(OP_SEMI, "expect a semicolon after each var specification");
                } while (t.type != OP_RPAREN);
            }
            else {
                node->varSpec.push_back(parseVarSpec(t));
            }
        }
        return node;
    };
    parseVarSpec = [&](Token&t)->AstNode* {
        AstVarSpec* node = nullptr;
        if (auto*tmp = parseIdentifierList(t); tmp != nullptr) {
            node = new AstVarSpec;
            node->identifierList = tmp;
            if (auto * tmp1 = parseType(t); tmp1 != nullptr) {
                node->avs.named.type = tmp1;
                t = next(f);
                if (t.type == OP_AGN) {
                    t = next(f);
                    node->avs.named.expressionList = parseExpressionList(t);
                }
            }
            else if (t.type == OP_AGN) {
                t = next(f);
                node->avs.expressionList = parseExpressionList(t);
            }
        }
        return node;
    };
    parseFunctionDecl = [&](Token&t)->AstNode* {
        AstFunctionDecl * node = nullptr;
        if (t.type == KW_func) {
            node = new AstFunctionDecl;
            t = next(f);
            if (t.type == OP_LPAREN) {
                node->receiver = parseParameter(t);
            }
            node->funcName = t.lexeme;
            t = next(f);
            node->signature = parseSignature(t);
            node->functionBody = parseBlock(t);
        }
        return node;
    };
    
    // Type
    parseType = [&](Token&t)->AstNode* {
        AstType * node = nullptr;
        switch (t.type) {
        case TK_ID: node = new AstType; node->type = parseTypeName(t); break;
        case OP_LBRACKET: node = new AstType; node->type = parseArrayOrSliceType(t); break;
        case KW_struct: node = new AstType; node->type = parseStructType(t); break;
        case OP_MUL: node = new AstType; node->type = parsePointerType(t); break;
        case KW_func: node = new AstType; node->type = parseFunctionType(t); break;
        case KW_interface: node = new AstType; node->type = parseInterfaceType(t); break;
        case KW_map: node = new AstType; node->type = parseMapType(t); break;
        case KW_chan: node = new AstType; node->type = parseChannelType(t); break;
        case OP_LPAREN:
            t = next(f);
            node = dynamic_cast<AstType*>(parseType(t));
            expect(OP_RPAREN, "the parenthesis () must match in type declaration");
            break;
        }

        return node;
    };
    parseTypeName = [&](Token&t)->AstNode* {
        AstTypeName * node = nullptr;
        if (t.type == TK_ID) {
            node = new AstTypeName;
            string typeName;
            typeName += t.lexeme;
            t = next(f);
            if (t.type == OP_DOT) {
                t = next(f);
                typeName.operator+=(".").operator+=(t.lexeme);
                t = next(f);
            }
            node->typeName = typeName;

        }
        return node;
    };
    parseArrayOrSliceType = [&](Token&t)->AstNode* {
        AstNode* node = nullptr;
        if (t.type == OP_LBRACKET) {
            t = next(f);
            if (t.type != OP_RBRACKET) {
                node = new AstArrayType;
                dynamic_cast<AstArrayType*>(node)->length = parseExpression(t);
                t = next(f);
                dynamic_cast<AstArrayType*>(node)->elementType = parseType(t);
            }
            else {
                node = new AstSliceType;
                t = next(f);
                dynamic_cast<AstSliceType*>(node)->elementType = parseType(t);
            }
            
        }
        return node;
    };
    parseStructType = [&](Token&t)->AstNode* {
        AstStructType* node = nullptr;
        if (t.type == KW_struct) {
            node = new  AstStructType;
            expect(OP_LBRACE, "left brace { must exist in struct type declaration");
            t = next(f);
            do {
                AstStructType::_FieldDecl fd;
                if (auto * tmp = parseIdentifierList(t); tmp != nullptr) {
                    fd.named.identifierList = tmp;
                    fd.named.type = parseType(t);
                }
                else {
                    if (t.type == OP_MUL) {
                        t = next(f);
                    }
                    fd.typeName = parseTypeName(t);
                }
                string tag;
                if (t.type == LITERAL_STR) {
                    tag = t.lexeme;
                }
                node->fields.push_back(make_tuple(fd, tag));
                if (t.type == OP_SEMI) {
                    t = next(f);
                }
            } while (t.type != OP_RBRACE);
            eat(OP_RBRACE, "expect }");
            eat(OP_SEMI, "expect ;");
        }
        return node;
    };
    parsePointerType = [&](Token&t)->AstNode* {
        AstPointerType* node = nullptr;
        if (t.type == OP_MUL) {
            node = new AstPointerType;
            t = next(f);
            node->baseType = parseType(t);
        }
        return node;
    };
    parseFunctionType = [&](Token&t)->AstNode* {
        AstFunctionType* node = nullptr;
        if (t.type == KW_func) {
            node = new AstFunctionType;
            t = next(f);
            node->signature = parseSignature(t);
        }
        return node;
    };
    parseSignature = [&](Token&t)->AstNode* {
        AstSignature* node = nullptr;
        if (t.type == OP_LPAREN) {
            node = new AstSignature;
            node->parameters = parseParameter(t);
            node->result = parseResult(t);
        }
        return node;
    };
    parseParameter = [&](Token&t)->AstNode* {
        AstParameter* node = nullptr;
        if (t.type == OP_LPAREN) {
            node = new AstParameter;
            t = next(f);
            do {
                if (auto * tmp = parseParameterDecl(t); tmp != nullptr) {
                    node->parameterList.push_back(tmp);
                }
                if (t.type == OP_COMMA) {
                    t = next(f);
                }
            } while (t.type != OP_RPAREN);
            t = next(f);

            for (int i = 0, rewriteStart = 0; i < node->parameterList.size(); i++) {
                if (dynamic_cast<AstParameterDecl*>(node->parameterList[i])->hasName == true) {
                    for (int k = rewriteStart; k < i; k++) {
                        string name = dynamic_cast<AstTypeName*>(
                            dynamic_cast<AstType*>(dynamic_cast<AstParameterDecl*>(node->parameterList[k])->type)->type)->typeName;
                        dynamic_cast<AstParameterDecl*>(node->parameterList[k])->type = dynamic_cast<AstParameterDecl*>(node->parameterList[i])->type;
                        dynamic_cast<AstParameterDecl*>(node->parameterList[k])->name = name;
                        dynamic_cast<AstParameterDecl*>(node->parameterList[k])->hasName = true; //It's not necessary
                    }
                    rewriteStart = i + 1;
                }
            }
        }
        return node;
    };
    parseParameterDecl = [&](Token&t)->AstNode* {
        AstParameterDecl* node = nullptr;
        if (t.type == OP_VARIADIC) {
            node = new AstParameterDecl;
            node->isVariadic = true;
            t = next(f);
            node->type = parseType(t);
        }
        else if (t.type != OP_RPAREN) {
            node = new AstParameterDecl;
            auto*mayIdentOrType = parseType(t);
            if (t.type != OP_COMMA && t.type != OP_RPAREN) {
                node->hasName = true;
                if (t.type == OP_VARIADIC) {
                    node->isVariadic = true;
                    t = next(f);
                }
                node->name = dynamic_cast<AstTypeName*>(dynamic_cast<AstType*>(mayIdentOrType)->type)->typeName;
                node->type = parseType(t);
            }
            else {
                node->type = mayIdentOrType;
            }
        }
        return node;
    };
    parseResult = [&](Token&t)->AstNode* {
        AstResult* node = nullptr;
        if (auto*tmp = parseParameter(t); tmp != nullptr) {
            node = new AstResult;
            node->ar.parameter = tmp;
        }
        else  if (auto*tmp = parseType(t); tmp != nullptr) {
            node = new AstResult;
            node->ar.type = tmp;
        }
        return node;
    };
    parseInterfaceType = [&](Token&t)->AstNode* {
        AstInterfaceType* node = nullptr;
        if (t.type == KW_interface) {
            node = new AstInterfaceType;
            t = next(f);
            if (t.type == OP_LBRACE) {
                t = next(f);
                do {
                    if (auto*tmp = parseMethodSpec(t); tmp != nullptr) {
                        node->methodSpec.push_back(tmp);
                        if (t.type == OP_SEMI) {
                            t = next(f);
                        }
                    }
                } while (t.type != OP_RBRACE);
                t = next(f);
            }
        }

        return node;
    };
    parseMethodSpec = [&](Token&t)->AstNode* {
        AstMethodSpec* node = nullptr;
        if (auto*tmp = parseMethodName(t); tmp != nullptr) {
            node = new AstMethodSpec;
            node->ams.named.methodName = tmp;
            node->ams.named.signature = parseSignature(t);
        }
        else  if (auto*tmp = parseTypeName(t); tmp != nullptr) {
            node = new AstMethodSpec;
            node->ams.interfaceTypeName = tmp;
        }
        return node;
    };
    parseMethodName = [&](Token&t)->AstNode* {
        AstMethodName* node = nullptr;
        if (t.type == TK_ID) {
            node = new AstMethodName;
            node->methodName = t.lexeme;
            t = next(f);
        }
        return node;
    };
    parseSliceType = [&](Token&t)->AstNode* {
        AstSliceType* node = nullptr;
        if (t.type == OP_LBRACKET) {
            node = new AstSliceType;
            expect(OP_RBRACKET, "bracket [] must match in slice type declaration");
            node->elementType = parseType(t);
        }
        return node;
    };
    parseMapType = [&](Token&t)->AstNode* {
        AstMapType* node = nullptr;
        if (t.type == KW_map) {
            node = new AstMapType;
            t = next(f);
            eat(OP_LBRACKET, "bracket [] must match in map type declaration");
            node->keyType = parseType(t);
            eat(OP_RBRACKET, "bracket [] must match in map type declaration");
            node->elementType = parseType(t);
        }
        return node;
    };
    parseChannelType = [&](Token&t)->AstNode* {
        //ChannelType = ( "chan" | "chan" "<-" | "<-" "chan" ) ElementType .
        AstChannelType* node = nullptr;
        if (t.type == KW_chan) {
            node = new AstChannelType;
            t = next(f);
            if (t.type == OP_CHAN) {
                t = next(f);
                node->elementType = parseType(t);
            }
            else {
                node->elementType = parseType(t);
            }
        }
        else if (t.type == OP_CHAN) {
            node = new AstChannelType;
            t = next(f);
            if (t.type == KW_chan) {
                node->elementType = parseType(t);
            }
        }
        return node;
    };

    // Statement
    parseStatement = [&](Token&t)->AstNode* {
        AstStatement * node = nullptr;

        switch (t.type) {
        case KW_type: node = new AstStatement; node->stmt = parseTypeDecl(t); break;
        case KW_const: node = new AstStatement; node->stmt = parseConstDecl(t); break;
        case KW_func: node = new AstStatement; node->stmt = parseFunctionDecl(t); break;
        case KW_var: node = new AstStatement; node->stmt = parseVarDecl(t); break;
        case KW_go: node = new AstStatement; node->stmt = parseGoStmt(t); break;
        case KW_return: node = new AstStatement; node->stmt = parseReturnStmt(t); break;
        case KW_break: node = new AstStatement; node->stmt = parseBreakStmt(t); break;
        case KW_continue: node = new AstStatement; node->stmt = parseContinueStmt(t); break;
        case KW_goto: node = new AstStatement; node->stmt = parseGotoStmt(t); break;
        case KW_fallthrough: node = new AstStatement; node->stmt = parseFallthroughStmt(t); break;
        case KW_if: node = new AstStatement; node->stmt = parseIfStmt(t); break;
        case KW_switch: node = new AstStatement; node->stmt = parseSwitchStmt(t); break;
        case KW_select: node = new AstStatement; node->stmt = parseSelectStmt(t); break;
        case KW_for: node = new AstStatement; node->stmt = parseForStmt(t); break;
        case KW_defer: node = new AstStatement; node->stmt = parseDeferStmt(t); break;
        case OP_LBRACE: node = new AstStatement;  node->stmt = parseBlock(t); break;
        case OP_SEMI: break;//empty statement
        default:
            if (auto*tmp = parseSimpleStmt(t); tmp != nullptr) {
                node = new AstStatement;
                node->stmt = tmp;
            }
            break;
        }
        return node;
    };
    parseBlock = [&](Token&t)->AstNode* {
        AstBlock * node = nullptr;
        if (t.type == OP_LBRACE) {
            node = new AstBlock;
            t = next(f);
            if (t.type != OP_RBRACE) {
                node->statementList = parseStatementList(t);
            }
            else {
                t = next(f);
            }
        }
        return node;
    };
    parseLabeledStmt = [&](Token&t)->AstNode* {
        AstLabeledStmt * node = nullptr;
        if (t.type == TK_ID) {
            node = new AstLabeledStmt;
            node->identifier = t.lexeme;
            expect(OP_COLON, "label statement should have a colon");
            node->statement = parseStatement(t);
        }
        return node;
    };
    parseSimpleStmt = [&](Token&t)->AstNode* {
        AstSimpleStmt * node = nullptr;
        if (auto* tmp = parseExpressionStmt(t); tmp != nullptr) {
            node = new AstSimpleStmt;
            node->ass.expressionStmt = tmp;
        }
        else if (auto* tmp = parseSendStmt(t); tmp != nullptr) {
            node = new AstSimpleStmt;
            node->ass.sendStmt = tmp;
        }
        else if (auto* tmp = parseIncDecStmt(t); tmp != nullptr) {
            node = new AstSimpleStmt;
            node->ass.incDecStmt = tmp;
        }
        else if (auto* tmp = parseAssignment(t); tmp != nullptr) {
            node = new AstSimpleStmt;
            node->ass.assignment = tmp;
        }
        else if (auto* tmp = parseShortVarDecl(t); tmp != nullptr) {
            node = new AstSimpleStmt;
            node->ass.shortVarDecl = tmp;
        }
        return node;
    };
    parseGoStmt = [&](Token&t)->AstNode* {
        AstGoStmt * node = nullptr;
        if (t.type == KW_go) {
            node = new AstGoStmt;
            node->expression = parseExpression(t);
        }
        return node;
    };
    parseReturnStmt = [&](Token&t)->AstNode* {
        AstReturnStmt * node = nullptr;
        if (t.type == KW_return) {
            node = new AstReturnStmt;
            node->expressionList = parseExpressionList(t);
        }
        return node;
    };
    parseBreakStmt = [&](Token&t)->AstNode* {
        AstBreakStmt * node = nullptr;
        if (t.type == KW_break) {
            node = new AstBreakStmt;
            t = next(f);
            if (t.type == TK_ID) {
                node->label = t.lexeme;
            }
            else {
                node->label = nullptr;
            }
        }
        return node;
    };
    parseContinueStmt = [&](Token&t)->AstNode* {
        AstContinueStmt * node = nullptr;
        if (t.type == KW_continue) {
            node = new AstContinueStmt;
            t = next(f);
            if (t.type == TK_ID) {
                node->label = t.lexeme;
            }
            else {
                node->label = nullptr;
            }
        }
        return node;
    };
    parseGotoStmt = [&](Token&t)->AstNode* {
        AstGotoStmt* node = nullptr;
        if (t.type == KW_goto) {
            node = new AstGotoStmt;
            node->label = expect(TK_ID, "goto statement must follow a label").lexeme;
        }
        return node;
    };
    parseFallthroughStmt = [&](Token&t)->AstNode* {
        AstFallthroughStmt* node = nullptr;
        if (t.type == KW_fallthrough) {
            node = new AstFallthroughStmt;
        }
        return node;
    };
    parseIfStmt = [&](Token&t)->AstNode* {
        AstIfStmt* node = nullptr;
        if (t.type == KW_fallthrough) {
            node = new AstIfStmt;
            if (auto* tmp = parseSimpleStmt(t); tmp != nullptr) {
                node->condition = tmp;
                expect(OP_SEMI, "expect an semicolon in condition part of if");
            }
            t = next(f);
            node->expression = parseExpression(t);
            t = next(f);
            node->block = parseBlock(t);
            t = next(f);
            if (t.type == KW_else) {
                t = next(f);
                if (auto *tmp1 = parseIfStmt(t); tmp1 != nullptr) {
                    node->ais.ifStmt = tmp1;
                }
                else if (auto *tmp1 = parseBlock(t); tmp1 != nullptr) {
                    node->ais.block = tmp1;
                }
                else {
                    throw runtime_error("else is empty");
                }
            }
        }
        return node;
    };
    parseSwitchStmt = [&](Token&t)->AstNode* {
        AstSwitchStmt* node = nullptr;
        if (t.type == KW_switch) {
            node = new AstSwitchStmt;
            if (auto*tmp = parseSimpleStmt(t); tmp != nullptr) {
                node->condition = tmp;
                expect(OP_SEMI, "expect semicolon in switch condition");
                if (auto*tmp1 = parseExpression(t); tmp1 != nullptr) {
                    node->conditionExpr = tmp1;
                    t = next(f);
                }
            }
            expect(OP_LBRACE, "expect left brace around case clauses");
            do {
                if (auto*tmp = parseExprCaseClause(t); tmp != nullptr) {
                    node->exprCaseClause.push_back(tmp);
                }
                t = next(f);
            } while (t.type != OP_RBRACE);
        }
        return node;
    };
    parseExprCaseClause = [&](Token&t)->AstNode* {
        AstExprCaseClause* node = nullptr;
        if (auto*tmp = parseExprSwitchCase(t); tmp != nullptr) {
            node = new AstExprCaseClause;
            node->exprSwitchCase = tmp;
            expect(OP_COLON, "expect colon in case clause of switch");
            node->statementList = parseStatementList(t);
        }
        return node;
    };
    parseExprSwitchCase = [&](Token&t)->AstNode* {
        AstExprSwitchCase* node = nullptr;
        if (t.type == KW_case) {
            node = new AstExprSwitchCase;
            t = next(f);
            if (auto*tmp = parseExpressionList(t); tmp != nullptr) {
                node->expressionList = tmp;
            }
            else if (t.type == KW_default) {
                node->isDefault = true;
            }
        }
        return node;
    };
    parseSelectStmt = [&](Token&t)->AstNode* {
        AstSelectStmt* node = nullptr;
        if (t.type == KW_select) {
            node = new AstSelectStmt;
            expect(OP_LBRACE, "expect left brace in select statement");
            do {
                if (auto*tmp = parseCommClause(t); tmp != nullptr) {
                    node->commClause.push_back(tmp);
                }
                t = next(f);
            } while (t.type != OP_RBRACE);
        }
        return node;
    };
    parseCommClause = [&](Token&t)->AstNode* {
        AstCommClause* node = nullptr;
        if (auto*tmp = parseCommCase(t); tmp != nullptr) {
            node = new AstCommClause;
            node->commCase = tmp;
            expect(OP_COLON, "expect colon in select case clause");
            node->statementList = parseStatementList(t);
        }
        return node;
    };
    parseCommCase = [&](Token&t)->AstNode* {
        AstCommCase*node = nullptr;
        if (t.type == KW_case) {
            node = new AstCommCase;
            t = next(f);
            if (auto*tmp = parseSendStmt(t); tmp != nullptr) {
                node->acc.sendStmt = tmp;
            }
            else if (auto*tmp = parseRecvStmt(t); tmp != nullptr) {
                node->acc.recvStmt = tmp;
            }
            else if (t.type == KW_default) {
                node->isDefault = true;
            }
        }
        return node;
    };
    parseRecvStmt = [&](Token&t)->AstNode* {
        AstRecvStmt*node = nullptr;
        if (auto*tmp = parseExpressionList(t); tmp != nullptr) {
            node = new AstRecvStmt;
            node->ars.expressionList = tmp;
            expect(OP_EQ, "expect =");
            node->recvExpr = parseExpression(t);
        }
        else if (auto*tmp = parseIdentifierList(t); tmp != nullptr) {
            node = new AstRecvStmt;
            node->ars.identifierList = tmp;
            expect(OP_SHORTAGN, "expect :=");
            node->recvExpr = parseExpression(t);
        }
        return node;
    };
    parseForStmt = [&](Token&t)->AstNode* {
        AstForStmt* node = nullptr;
        if (t.type == KW_for) {
            node = new AstForStmt;
            t = next(f);
            if (auto*tmp = parseExpression(t); tmp != nullptr) {
                node->afs.condition = tmp;
            }
            else if (auto*tmp = parseForClause(t); tmp != nullptr) {
                node->afs.forClause = tmp;
            }
            else if (auto*tmp = parseRangeClause(t); tmp != nullptr) {
                node->afs.rangeClause = tmp;
            }
            t = next(f);
            node->block = parseBlock(t);
        }
        return node;
    };
    parseForClause = [&](Token&t)->AstNode* {
        AstForClause * node = nullptr;
        if (auto*tmp = parseSimpleStmt(t); tmp != nullptr) {
            node = new AstForClause;
            node->initStmt = tmp;
            expect(OP_SEMI, "expect semicolon in for clause");
            node->condition = parseExpression(t);
            expect(OP_SEMI, "expect semicolon in for clause");
            node->postStmt = parseSimpleStmt(t);
        }
        return node;
    };
    parseRangeClause = [&](Token&t)->AstNode* {
        AstRangeClause*node = nullptr;
        if (auto*tmp = parseExpressionList(t); tmp != nullptr) {
            node = new AstRangeClause;
            node->arc.expressionList = tmp;
            expect(OP_EQ, "expect =");
            t = next(f);
        }
        else if (auto* tmp = parseIdentifierList(t); tmp != nullptr) {
            node = new AstRangeClause;
            node->arc.identifierList = tmp;
            expect(OP_SHORTAGN, "expect :=");
            t = next(f);
        }
        if (t.type == KW_range) {
            if (node == nullptr) {
                node = new AstRangeClause;
            }
            t = next(f);
            node->expression = parseExpression(t);
        }
        return node;
    };
    parseDeferStmt = [&](Token&t)->AstNode* {
        AstDeferStmt* node = nullptr;
        if (t.type == KW_defer) {
            node = new AstDeferStmt;
            node->expression = parseExpression(t);
        }
        return node;
    };
    parseExpressionStmt = [&](Token&t)->AstNode* {
        AstExpressionStmt* node = nullptr;
        if (auto*tmp = parseExpression(t); tmp != nullptr) {
            node = new AstExpressionStmt;
            node->expression = tmp;
        }
        return node;
    };
    parseSendStmt = [&](Token&t)->AstNode* {
        AstSendStmt* node = nullptr;
        if (auto*tmp = parseExpression(t); tmp != nullptr) {
            node = new AstSendStmt;
            node->receiver = tmp;
            expect(OP_CHAN, "expect a channel symbol");
            node->sender = parseExpression(t);
        }
        return node;
    };
    parseIncDecStmt = [&](Token&t)->AstNode* {
        AstIncDecStmt* node = nullptr;
        if (auto*tmp = parseExpression(t); tmp != nullptr) {
            node = new AstIncDecStmt;
            node->expression = tmp;
            t = next(f);
            if (t.type == OP_INC) {
                node->isInc = true;
            }
            else if (t.type == OP_DEC) {
                node->isInc = false;
            }
            else {
                throw runtime_error("expect ++/--");
            }
        }
        return node;
    };
    parseAssignment = [&](Token&t)->AstNode* {
        AstAssignment* node = nullptr;
        if (auto*tmp = parseExpressionList(t); tmp != nullptr) {
            node = new AstAssignment;
            node->lhs = tmp;
            t = next(f);
            if (t.type == OP_ADD || t.type == OP_SUB ||
                t.type == OP_BITOR || t.type == OP_XOR ||
                t.type == OP_MUL || t.type == OP_DIV ||
                t.type == OP_MOD || t.type == OP_LSHIFT ||
                t.type == OP_RSHIFT || t.type == OP_BITAND ||
                t.type == OP_ANDXOR) {
                node->assignOp = t.type;
            }
            expect(OP_EQ, "expect =");
            node->rhs = parseExpressionList(t);
        }
        return node;
    };
    parseShortVarDecl = [&](Token&t)->AstNode* {
        AstShortVarDecl* node = nullptr;
        if (auto*tmp = parseIdentifierList(t); tmp != nullptr) {
            node = new AstShortVarDecl;
            node->lhs = tmp;
            expect(OP_SHORTAGN, "expect := in short assign statement");
            node->rhs = parseExpressionList(t);
        }
        return node;
    };

    // Expression
    parseExpression = [&](Token&t)->AstNode* {
        AstExpression* node = nullptr;
        if (auto*tmp = parseUnaryExpr(t); tmp != nullptr) {
            node = new  AstExpression;
            node->ae.unaryExpr = tmp;
            if (t.type == OP_OR || t.type == OP_AND || t.type == OP_EQ ||
                t.type == OP_NE || t.type == OP_LT || t.type == OP_LE ||
                t.type == OP_GT || t.type == OP_GE || t.type == OP_ADD ||
                t.type == OP_SUB || t.type == OP_BITOR || t.type == OP_XOR ||
                t.type == OP_MUL || t.type == OP_DIV || t.type == OP_MOD ||
                t.type == OP_LSHIFT || t.type == OP_RSHIFT || t.type == OP_BITAND ||
                t.type == OP_XOR) {
                node->ae.named.binaryOp = t.type;
                node->ae.named.lhs = tmp;
                t = next(f);
                node->ae.named.rhs = parseExpression(t);
            }
        }
        return node;
    };
    parseUnaryExpr = [&](Token&t)->AstNode* {
        AstUnaryExpr* node = nullptr;
        if (t.type == OP_ADD || t.type == OP_SUB || t.type == OP_NOT ||
            t.type == OP_XOR || t.type == OP_MUL || t.type == OP_BITAND || t.type == OP_CHAN) {
            node = new AstUnaryExpr;
            node->aue.named.unaryOp = t.type;
            t = next(f);
            node->aue.named.unaryExpr = parseUnaryExpr(t);
        }
        else if (auto*tmp = parsePrimaryExpr(t); tmp != nullptr) {
            node = new AstUnaryExpr;
            node->aue.primaryExpr = tmp;
        }
        return node;
    };

    parsePrimaryExpr = [&](Token&t)->AstNode* {
        AstPrimaryExpr*node = nullptr;
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
                        if (t.lexeme == "type") {
                            auto* e = new AstTypeSwitchGuardExpr;
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
                    t = next(f);
                    AstNode* start = nullptr;//Ignore start if next token is :(syntax of operand[:xxx])
                    if (t.type != OP_COLON) {
                        // operand[6] index syntax
                        start = parseExpression(t);
                        if (t.type == OP_RBRACKET) {
                            auto* e = new AstIndexExpr;
                            e->operand = tmp;
                            e->index = start;
                            tmp = e;
                            t = next(f);
                        }
                        
                    }
                    auto* sliceExpr = new AstSliceExpr;
                    sliceExpr->operand = tmp;
                    sliceExpr->begin = start;
                    eat(OP_COLON, "expect : in slice expression");
                    sliceExpr->end = parseExpression(t);//may nullptr
                    if (t.type == OP_COLON) {
                        t = next(f);
                        sliceExpr->step = parseExpression(t);
                        eat(OP_RBRACKET, "expect ] at the end of slice expression");
                    }
                    else if (t.type == OP_RBRACKET) {
                        t = next(f);
                    }
                    tmp = sliceExpr;
                }
                else if (t.type == OP_LPAREN) {
                    auto* e = new AstCallExpr;
                    e->operand = tmp;
                    if (auto*tmp1 = parseExpressionList(t); tmp1 != nullptr) {
                        e->arguments = tmp1;
                    }
                    else if (auto*tmp = parseType(t); tmp1 != nullptr) {
                        e->type = tmp1;
                        t = next(f);
                        if (t.type == OP_COMMA) {
                            e->arguments = parseExpressionList(t);
                        }
                    }
                    if (t.type == OP_VARIADIC) {
                        e->isVariadic = true;
                        t = next(f);
                    }
                    tmp = e;
                }
                else if (t.type == OP_LBRACE) {

                }
                else {
                    break;
                }
            }
            node->expr = tmp;
        }
        return node;
    };
    parseOperand = [&](Token&t)->AstNode* {
        AstOperand*node = nullptr;
        if (auto * tmp = parseLiteral(t); tmp != nullptr) {
            node = new AstOperand;
            node->ao.literal = tmp;
        }
        else if (auto *tmp = parseOperandName(t); tmp != nullptr) {
            node = new AstOperand;
            node->ao.operandName = tmp;
        }
        else if (t.type == OP_LPAREN) {
            node = new AstOperand;
            t = next(f);
            node->ao.expression = parseExpression(t);
            eat(OP_RPAREN, "expect )");
        }
        return node;
    };
    parseOperandName = [&](Token&t)->AstNode* {
        AstOperandName*node = nullptr;
        if (t.type == TK_ID) {
            node = new AstOperandName;
            string operandName = t.lexeme;
            t = next(f);
            if (t.type == OP_DOT) {
                t = next(f);
                operandName.operator+=(".").operator+=(t.lexeme);
                node->operandName = operandName;
            }
            else {
                node->operandName = operandName;
            }
        }
        return node;
    };
    parseLiteral = [&](Token&t)->AstNode* {
        AstLiteral*node = nullptr;
        if (auto*tmp = parseBasicLit(t); tmp != nullptr) {
            node = new AstLiteral;
            node->al.basicLit = tmp;
        }
        else if (auto*tmp = parseCompositeLit(t); tmp != nullptr) {
            node = new AstLiteral;
            node->al.compositeLit = tmp;
        }
        else if (auto*tmp = parseFunctionLit(t); tmp != nullptr) {
            node = new AstLiteral;
            node->al.functionLit = tmp;
        }
        return node;
    };
    parseBasicLit = [&](Token&t)->AstNode* {
        AstBasicLit* node = nullptr;
        if (t.type == LITERAL_INT || t.type == LITERAL_FLOAT || t.type == LITERAL_IMG ||
            t.type == LITERAL_RUNE || t.type == LITERAL_STR) {
            node = new AstBasicLit;
            node->type = t.type;
            node->value = t.lexeme;
            t = next(f);
        }
        return node;
    };
    parseCompositeLit = [&](Token&t)->AstNode* {
        AstCompositeLit* node = nullptr;
        if (auto*tmp = parseStructType(t); tmp != nullptr) {
            node = new AstCompositeLit;
            node->acl.structType = tmp;
            node->literalValue = parseLiteralValue(t);
        }
        else if (t.type == OP_LBRACKET) {
            node = new AstCompositeLit;
            t = next(f);
            if (t.type == OP_VARIADIC) {
                node->acl.automaticLengthArrayType.automaticLength = true;
                expect(OP_RBRACKET, "expect ]");
                t = next(f);
                node->acl.automaticLengthArrayType.elementType = parseType(t);
            }
            else {
                if (t.type != OP_RBRACKET) {
                    node->acl.arrayType.arrayLength = parseExpression(t);
                }
                else {
                    t = next(f);
                }
                node->acl.arrayType.elementType = parseType(t);
            }
            node->literalValue = parseLiteralValue(t);
        }
        else if (auto*tmp = parseSliceType(t); tmp != nullptr) {
            node = new AstCompositeLit;
            node->acl.sliceType = tmp;
            node->literalValue = parseLiteralValue(t);
        }
        else if (auto*tmp = parseMapType(t); tmp != nullptr) {
            node = new AstCompositeLit;
            node->acl.mapType = tmp;
            node->literalValue = parseLiteralValue(t);
        }
        else if (auto*tmp = parseTypeName(t); tmp != nullptr) {
            node = new AstCompositeLit;
            node->acl.typeName = tmp;
            node->literalValue = parseLiteralValue(t);
        }
        return node;
    };
    parseLiteralValue = [&](Token&t)->AstNode* {
        AstLiteralValue*node = nullptr;
        if (t.type == OP_LBRACE) {
            node = new AstLiteralValue;
            do {
                t = next(f);
                if (t.type == OP_RBRACE) {
                    // it's necessary since both {a,b} or {a,b,} are legal form
                    break;
                }
                node->keyedElement.push_back(parseKeyedElement(t));
            } while (t.type != OP_RBRACE);
            eat(OP_RBRACE, "brace {} must match");
        }
        return node;
    };
    parseKeyedElement = [&](Token&t)->AstNode* {
        AstKeyedElement*node = nullptr;
        if (auto*tmp = parseKey(t); tmp != nullptr) {
            node = new AstKeyedElement;
            node->element = tmp;
            if (t.type == OP_COLON) {
                node->key = tmp;
                t = next(f);
                if (auto*tmp1 = parseElement(t); tmp1 != nullptr) {
                    node->element = tmp1;
                }
            }
        }
        return node;
    };
    parseKey = [&](Token&t)->AstNode* {
        AstKey*node = nullptr;
        if (auto*tmp = parseFieldName(t); tmp != nullptr) {
            node = new AstKey;
            node->ak.fieldName = tmp;
        }
        else if (auto*tmp = parseLiteralValue(t); tmp != nullptr) {
            node = new AstKey;
            node->ak.literalValue = tmp;
        }
        else if (auto*tmp = parseExpression(t); tmp != nullptr) {
            node = new AstKey;
            node->ak.expression = tmp;
        }
        return node;
    };
    parseFieldName = [&](Token&t)->AstNode* {
        AstFieldName* node = nullptr;
        if (t.type == TK_ID) {
            node = new AstFieldName;
            node->fieldName = t.lexeme;
            t = next(f);
        }
        return node;
    };
    parseElement = [&](Token&t)->AstNode* {
        AstElement*node = nullptr;
        if (auto*tmp = parseExpression(t); tmp != nullptr) {
            node = new AstElement;
            node->ae.expression = tmp;
        }
        else if (auto*tmp = parseLiteralValue(t); tmp != nullptr) {
            node = new AstElement;
            node->ae.literalValue = tmp;
        }
        return node;
    };
    parseFunctionLit = [&](Token&t)->AstNode* {
        AstFunctionLit* node = nullptr;
        if (t.type == KW_func) {
            node = new AstFunctionLit;
            t = next(f);
            node->signature = parseSignature(t);
            t = next(f);
            node->functionBody = parseBlock(t);
        }
        return node;
    };
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
    //printLex(filename);
    if (argc < 2) {
        fprintf(stderr, "specify your go source file\n");
        return 1;
    }
    const AstNode* ast = parse(argv[1]);
    fprintf(stdout, "parsing passed\n");
    return 0;
}