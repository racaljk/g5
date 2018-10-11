//===----------------------------------------------------------------------===//
// Golang specification can be found here: https://golang.org/ref/spec
//
// In development, I consider raise runtime_error  since it's helpful to locate 
// where error occurred and do further debugging. 
//
// Written by racaljk@github<1948638989@qq.com>
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
#include <tuple>
#include <map>
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
struct AstNode { virtual ~AstNode() {} };
struct AstIdentifierList :public AstNode { vector<string> identifierList; };
struct AstExpressionList :public AstNode { vector<AstNode*> expressionList; };
struct AstSourceFile :public AstNode {
    AstNode* packageClause;
    vector<AstNode*> importDecl;
    vector<AstNode*> topLevelDecl;
};
struct AstPackageClause :public AstNode { string packageName; };
struct AstImportDecl :public AstNode { map<string, string> imports; };
struct AstTopLevelDecl :public AstNode {
    union {
        AstNode* decl;
        AstNode* functionDecl;
        AstNode* methodDecl;
    }atld;
};
struct AstDeclaration :public AstNode {
    union {
        AstNode* constDecl;
        AstNode* typeDecl;
        AstNode* varDecl;
    }ad;
};
struct AstConstDecl :public AstNode {
    vector<AstNode*> identifierList;
    vector<AstNode*> type;
    vector<AstNode*> expressionList;
};
struct AstType :public AstNode {
    union {
        AstNode* typeName;
        AstNode* typeLit;
    }at;
};
struct AstTypeName : public AstNode { string typeName; };
struct AstTypeLit : public AstNode {
    union {
        AstNode* arrayType;
        AstNode* structType;
        AstNode* pointerType;
        AstNode* functionType;
        AstNode* interfaceType;
        AstNode* sliceType;
        AstNode* mapType;
        AstNode* channelType;
    }atl;
};
struct AstArrayType : public AstNode {
    AstNode* length;
    AstNode* elementType;
};
struct AstStructType :public AstNode {
    union _FieldDecl {
        struct {
            AstNode* identifierList;
            AstNode* type;
        }named;
        AstNode* typeName;
    };

    vector<tuple<_FieldDecl, string>> fields;
};
struct AstPointerType : public AstNode { AstNode * baseType; };
struct AstFunctionType :public AstNode { AstNode * signature; };
struct AstSignature :public AstNode {
    AstNode* parameters;
    AstNode* result;
};
struct AstParameter :public AstNode { vector<AstNode*> parameterList; };
struct AstParameterDecl :public AstNode {
    AstNode* identifierList;
    bool isVariadic = false;
    AstNode* type;
};
struct AstResult :public AstNode {
    union {
        AstNode* parameter;
        AstNode* type;
    }ar;
};
struct AstInterfaceType :public AstNode { vector<AstNode*> methodSpec; };
struct AstMethodSpec :public AstNode {
    union {
        struct _MethodSignature {
            AstNode* methodName;
            AstNode* signature;
        }named;
        AstNode* interfaceTypeName;
    }ams;
};
struct AstMethodName :public AstNode { string methodName; };
struct AstSliceType :public AstNode { AstNode* elementType; };
struct AstMapType :public AstNode {
    AstNode* keyType;
    AstNode* elementType;
};
struct AstChannelType :public AstNode { AstNode* elementType; };
struct AstTypeDecl :public AstNode { vector<AstNode*> typeSpec; };
struct AstTypeSpec :public AstNode {
    string identifier;
    AstNode* type;
};
struct AstVarDecl :public AstNode { vector<AstNode*> varSpec; };
struct AstVarSpec :public AstNode {
    AstNode* identifierList;
    union {
        struct {
            AstNode* type;
            AstNode* expressionList;
        }named;
        AstNode* expressionList;
    }avs;
};
struct AstFunctionDecl :public AstNode {
    string funcName;
    AstNode* signature;
    AstNode* functionBody;
};
struct AstFunctionBody : public AstNode { AstNode* block; };
struct AstBlock :public AstNode { AstNode* statementList; };
struct AstStatementList : public AstNode { vector<AstNode*> statements; };
struct AstStatement : public AstNode {
    union {
        AstNode* declaration;
        AstNode* labeledStmt;
        AstNode* simpleStmt;
        AstNode* goStmt;
        AstNode* returnStmt;
        AstNode* breakStmt;
        AstNode* continueStmt;
        AstNode* gotoStmt;
        AstNode* fallthroughStmt;
        AstNode* block;
        AstNode* ifStmt;
        AstNode* switchStmt;
        AstNode* selectStmt;
        AstNode* forStmt;
        AstNode* deferStmt;
    }as;
};
struct AstLabeledStmt : public AstNode {
    string identifier;
    AstNode* statement;
};
struct AstSimpleStmt : public AstNode {
    union {
        AstNode* expressionStmt;
        AstNode* sendStmt;
        AstNode* incDecStmt;
        AstNode* assignment;
        AstNode* shortVarDecl;
    }ass;
};
struct AstGoStmt : public AstNode {
    AstNode* expression;
};
struct AstReturnStmt : public AstNode {
    AstNode* expressionList;
};
struct AstBreakStmt : public AstNode {
    string label;
};
struct AstContinueStmt : public AstNode {
    string label;
};
struct AstGotoStmt : public AstNode {
    string label;
};
struct AstFallthroughStmt : public AstNode {};
struct AstIfStmt :public AstNode {
    AstNode* condition;
    AstNode* expression;
    AstNode* block;
    union {
        AstNode* ifStmt;
        AstNode* block;
    }ais;
};
struct AstSwitchStmt :public AstNode {
    AstNode* condition;
    AstNode* conditionExpr;
    vector<AstNode*> exprCaseClause;
};
struct AstExprCaseClause : public AstNode {
    AstNode* exprSwitchCase;
    AstNode* statementList;
};
struct AstExprSwitchCase : public AstNode {
    AstNode * expressionList;
    bool isDefault;
};
struct AstSelectStmt : public AstNode {
    vector<AstNode*> commClause;
};
struct AstCommClause :public AstNode {
    AstNode* commCase;
    AstNode* statementList;
};
struct AstCommCase :public AstNode {
    union {
        AstNode* sendStmt;
        AstNode* recvStmt;
    }acc;
    bool isDefault;
};
struct AstRecvStmt :public AstNode {
    union {
        AstNode* identifierList;
        AstNode* expressionList;
    }ars;
    AstNode* recvExpr;
};
struct AstForStmt :public AstNode {
    union {
        AstNode* condition;
        AstNode* forClause;
        AstNode* rangeClause;
    }afs;
    AstNode* block;
};
struct AstForClause :public AstNode {
    AstNode* initStmt;
    AstNode* condition;
    AstNode* postStmt;
};
struct AstRangeClause :public AstNode {
    union {
        AstNode* expressionList;
        AstNode* identifierList;
    }arc;
    AstNode* expression;
};
struct AstDeferStmt :public AstNode {
    AstNode* expression;
};
struct AstExpressionStmt :public AstNode {
    AstNode* expression;
};
struct AstSendStmt : public AstNode {
    AstNode* receiver;
    AstNode* sender;
};
struct AstIncDecStmt : public AstNode {
    AstNode* expression;
    bool isInc;
};
struct AstAssignment :public AstNode {
    AstNode* lhs;
    AstNode* rhs;
    TokenType assignOp;
};
struct AstShortVarDecl : public AstNode {
    AstNode* lhs;
    AstNode* rhs;
};
struct AstMethodDecl :public AstNode {
    AstNode* receiver;
    string methodName;
    AstNode* signature;
    AstNode* functionBody;
};
struct AstExpression : public AstNode {
    union {
        struct {
            AstNode* lhs;
            TokenType binaryOp;
            AstNode* rhs;
        }named;
        AstNode* unaryExpr;
    }ae;
};
struct AstUnaryExpr :public AstNode {
    union {
        AstNode*primaryExpr;
        struct {
            AstNode* unaryExpr;
            TokenType unaryOp;
        }named;
    }aue;
};
struct AstPrimaryExpr :public AstNode {
    union {
        AstNode* operand;
        AstNode* conversion;
        AstNode* methodExpr;
        struct {
            AstNode* primaryExpr;
            AstNode* selector;
        }selector;
        struct {
            AstNode* primaryExpr;
            AstNode* index;
        }index;
        struct {
            AstNode* primaryExpr;
            AstNode* slice;
        }slice;
        struct {
            AstNode* primaryExpr;
            AstNode* typeAssertion;
        }typeAssertion;
        struct {
            AstNode* primaryExpr;
            AstNode* argument;
        }argument;
    }ape;
};
struct AstSelector :public AstNode {
    string identifier;
};
struct AstIndex :public AstNode {
    AstNode* expression;
};
struct AstSlice :public AstNode {
    AstNode*start;
    AstNode*stop;
    AstNode*step;
};
struct AstTypeAssertion :public AstNode {
    AstNode*type;
};
struct AstArgument :public AstNode {
    union {
        AstNode*expressionList;
        struct {
            AstNode*type;
            AstNode*expressionList;
        }named;    
    }aa;
    bool isVariadic;
};
struct AstOperand :public AstNode {
    union {
        AstNode*literal;
        AstNode*operandName;
        AstNode*expression;
    }ao;
};
struct AstOperandName : public AstNode {
    string operandName;
};
struct AstLiteral :public AstNode {
    union {
        AstNode*basicLit;
        AstNode*compositeLit;
        AstNode*functionLit;
    }al;
};
struct AstBasicLit : public AstNode {
    TokenType lit;
};
struct AstCompositeLit : public AstNode {
    AstNode*literalType;
    AstNode*literalValue;
};
struct AstLiteralValue : public AstNode {
    AstNode*elementList;
};
struct AstElementList : public AstNode {
    vector< AstNode*> keyedElement;
};
struct AstKeyedElement : public AstNode {
    AstNode*key;
    AstNode*element;
};
struct AstKey : public AstNode {
    union {
        AstNode* fieldName;
        AstNode* expression;
        AstNode* literalValue;
    }ak;
};
struct AstFieldName : public AstNode {
    string fieldName;
};
struct AstElement : public AstNode {
    union {
        AstNode*expression;
        AstNode*literalValue;
    }ae;
};
struct AstFunctionLit : public AstNode {
    AstNode*signature;
    AstNode*functionBody;
};
struct AstConversion : public AstNode {
    AstNode*type;
    AstNode*expression;
};
struct AstMethodExpr : public AstNode {
    AstNode*receiverType;
    string methodName;
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
// Give me 5, I'll give you a minimal but complete golang impl back.
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
            return Token(TK_EOF, "");
        }
        shouldEof = 1;
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
                    type = OP_DOT;
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

    auto expect = [&f](TokenType tk, const string& msg) {
        auto t = next(f);
        if (t.type != tk) throw runtime_error(msg);
        return t;
    };

    function<AstNode*(Token&)>parsePackageClause, parseImportDecl, parseTopLevelDecl,
        parseDeclaration, parseConstDecl, parseIdentifierList, parseType, parseTypeName,
        parseTypeLit, parseArrayType, parseStructType, parsePointerType, parseFunctionType,
        parseSignature, parseParameter, parseParameterDecl, parseResult, parseInterfaceType,
        parseMethodSpec, parseMethodName, parseSliceType, parseMapType, parseChannelType,
        parseTypeDecl, parseTypeSpec, parseVarDecl, parseVarSpec, parseFunctionDecl,
        parseFunctionBody, parseStatementList, parseStatement, parseCompositeLit,
        parseLabeledStmt, parseSimpleStmt, parseGoStmt, parseReturnStmt, parseBreakStmt,
        parseContinueStmt, parseGotoStmt, parseFallthroughStmt, parseBlock, parseIfStmt,
        parseSwitchStmt, parseSelectStmt, parseForStmt, parseDeferStmt, parseExpressionStmt,
        parseSendStmt, parseIncDecStmt, parseAssignment, parseShortVarDecl, parseExprCaseClause,
        parseExprSwitchCase, parseCommClause, parseCommCase, parseRecvStmt, parseForClause,
        parseRangeClause, parseSourceFile, parseMethodDecl, parseExpressionList, parseExpression,
        parseUnaryExpr, parsePrimaryExpr, parseSelector, parseIndex, parseSlice, parseTypeAssertion,
        parseArgument, parseOperand, parseOperandName, parseLiteral, parseBasicLit, 
        parseLiteralValue, parseElementList, parseKeyedElement, parseKey, parseElement, 
        parseFunctionLit, parseConversion, parseMethodExpr, parseFieldName;

    parseIdentifierList = [&](Token&t)->AstNode* {
        AstIdentifierList* node = nullptr;
        if (t.type = TK_ID) {
            node = new  AstIdentifierList;
            node->identifierList.emplace_back(t.lexeme);
            t = next(f);
            if (t.type == OP_COMMA)
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
            t = next(f);
            if (t.type == OP_COMMA) {
                while (t.type == OP_COMMA) {
                    t = next(f);
                    node->expressionList.emplace_back(parseExpression(t));
                    t = next(f);
                }
            }
        }
        return node;
    };
    parseSourceFile = [&](Token&t)->AstNode* {
        auto node = new AstSourceFile;
        

        node->packageClause = parsePackageClause(t);

        expect(OP_SEMI, "expect a semicolon");
        t = next(f);
        while (t.type == KW_import) {
            node->importDecl.push_back(parseImportDecl(t));
            t = next(f);
        }
        while (t.type != TK_EOF) {
            node->topLevelDecl.push_back(parseTopLevelDecl(t));
        }
        return node;
    };
    parsePackageClause = [&](Token&t)->AstNode* {
        auto node = new AstPackageClause;
        expect(KW_package, "a go source file should always start with \"package\" keyword");
        node->packageName = expect(TK_ID, "expect an identifier after package keyword").lexeme;
        grt.package = node->packageName;
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
        // TopLevelDecl  = Declaration | FunctionDecl | MethodDecl .
        if (auto* tmp = parseDeclaration(t); tmp != nullptr) {
            node = new AstTopLevelDecl;
            node->atld.decl = tmp;
        }
        else if (auto* tmp = parseFunctionDecl(t); tmp != nullptr) {
            node = new AstTopLevelDecl;
            node->atld.functionDecl = tmp;
        }
        else if (auto* tmp = parseMethodDecl(t); tmp != nullptr) {
            node = new AstTopLevelDecl;
            node->atld.methodDecl = tmp;
        }
        return node;
    };
    parseDeclaration = [&](Token&t)->AstNode* {
        AstDeclaration * node = nullptr;
        // Declaration   = ConstDecl | TypeDecl | VarDecl .
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
                do {
                    node->identifierList.push_back(parseIdentifierList(t));
                    node->type.push_back(parseType(t));
                    node->expressionList.push_back(parseExpressionList(t));
                    expect(OP_SEMI, "expect an explicit semicolon");
                } while (t.type != OP_RPAREN);
            }
            else {
                node->identifierList.push_back(parseIdentifierList(t));
                node->type.push_back(parseType(t));
                node->expressionList.push_back(parseExpressionList(t));
            }
        }
        return node;
    };
    parseType = [&](Token&t)->AstNode* {
        AstType * node = nullptr;
        // Declaration   = ConstDecl | TypeDecl | VarDecl .
        if (auto*tmp = parseTypeName(t); tmp != nullptr) {
            node = new AstType;
            node->at.typeName = tmp;
        }
        else  if (auto*tmp = parseTypeLit(t); tmp != nullptr) {
            node = new AstType;
            node->at.typeLit = tmp;
        }
        else  if (t.type == OP_LPAREN) {
            t = next(f);
            node = dynamic_cast<AstType*>(parseType(t));
            expect(OP_RPAREN, "the parenthesis () must match in type declaration");
        }
        return node;
    };
    parseTypeName = [&](Token&t)->AstNode* {
        AstTypeName * node = nullptr;
        if (t.type == TK_ID) {
            node = new AstTypeName;
            string typeName;
            typeName += t.lexeme;
            if (t.lexeme == grt.package) {
                //qualified identifier
                typeName += expect(OP_DOT, "qualified identifier required an dot as its delimiter").lexeme;
                typeName += expect(OP_DOT, "expect an identifier after dot delimiter").lexeme;
            }
            node->typeName = typeName;
        }
        return node;
    };
    parseTypeLit = [&](Token&t)->AstNode* {
        AstTypeLit * node = nullptr;
        if (auto*tmp = parseArrayType(t); tmp != nullptr) {
            node = new AstTypeLit;
            node->atl.arrayType = tmp;
        }
        else  if (auto*tmp = parseStructType(t); tmp != nullptr) {
            node = new AstTypeLit;
            node->atl.structType = tmp;
        }
        else  if (auto*tmp = parsePointerType(t); tmp != nullptr) {
            node = new AstTypeLit;
            node->atl.pointerType = tmp;
        }
        else  if (auto*tmp = parseFunctionType(t); tmp != nullptr) {
            node = new AstTypeLit;
            node->atl.functionType = tmp;
        }
        else  if (auto*tmp = parseInterfaceType(t); tmp != nullptr) {
            node = new AstTypeLit;
            node->atl.interfaceType = tmp;
        }
        else  if (auto*tmp = parseSliceType(t); tmp != nullptr) {
            node = new AstTypeLit;
            node->atl.sliceType = tmp;
        }
        else  if (auto*tmp = parseMapType(t); tmp != nullptr) {
            node = new AstTypeLit;
            node->atl.mapType = tmp;
        }
        else  if (auto*tmp = parseChannelType(t); tmp != nullptr) {
            node = new AstTypeLit;
            node->atl.channelType = tmp;
        }
        return node;
    };
    parseArrayType = [&](Token&t)->AstNode* {
        AstArrayType* node = nullptr;
        if (t.type == OP_LBRACKET) {
            node = new AstArrayType;
            node->length = parseExpression(t);
            expect(OP_RBRACKET, "bracket [] must match in array type declaration");
            node->elementType = parseType(t);
        }
        return node;
    };
    parseStructType = [&](Token&t)->AstNode* {
        AstStructType* node = nullptr;
        if (t.type == KW_struct) {
            node = new  AstStructType;
            expect(OP_LBRACE, "left brace { must exist in struct type declaration");
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
                node->fields.push_back(make_tuple(fd,tag));
            } while (t.type != OP_RBRACE);
        }
        return node;
    };
    parsePointerType = [&](Token&t)->AstNode* {
        AstPointerType* node = nullptr;
        if (t.type == OP_MUL) {
            node = new AstPointerType;
            node->baseType = parseType(t);
        }
        return node;
    };
    parseFunctionType = [&](Token&t)->AstNode* {
        AstFunctionType* node = nullptr;
        if (t.type == KW_func) {
            node = new AstFunctionType;
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
            do {
                if (auto * tmp = parseParameterDecl(t); tmp != nullptr) {
                    node->parameterList.push_back(tmp);
                }
                if (t.type == OP_COMMA) {
                    t = next(f);
                }
            } while (t.type != OP_RPAREN);
        }
        return node;
    };
    parseParameterDecl = [&](Token&t)->AstNode* {
        AstParameterDecl* node = nullptr;
        if (auto*tmp = parseIdentifierList(t); tmp != nullptr) {
            node = new AstParameterDecl;
            node->identifierList = tmp;
        }
        if (t.type == OP_VARIADIC) {
            node = new AstParameterDecl;
            node->isVariadic = true;
        }
        if (auto*tmp = parseType(t); tmp != nullptr) {
            node = new AstParameterDecl;
            node->type = tmp;
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
        if (t.type == OP_LBRACE) {
            do {
                if (auto*tmp = parseMethodSpec(t); tmp != nullptr) {
                    node = new AstInterfaceType;
                    node->methodSpec.push_back(tmp);
                    expect(OP_SEMI, "expect a semicolon after method specification");
                }
            } while (t.type != OP_RBRACE);
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
            expect(OP_LBRACKET, "bracket [] must match in map type declaration");
            node->keyType = parseType(t);
            expect(OP_RBRACKET, "bracket [] must match in map type declaration");
            node->elementType = parseType(t);
        }
        return node;
    };
    parseChannelType = [&](Token&t)->AstNode* {
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
    parseTypeDecl = [&](Token&t)->AstNode* {
        AstTypeDecl* node = nullptr;
        if (t.type == KW_type) {
            node = new AstTypeDecl;
            t = next(f);
            if (t.type == OP_LPAREN) {
                do {
                    node->typeSpec.push_back(parseTypeSpec(t));
                    expect(OP_SEMI, "expect a semicolon after each type specification");
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
            if (t.type == OP_EQ) {
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
            t = next(f);
            if (auto * tmp1 = parseType(t); tmp1 != nullptr) {
                node->avs.named.type = tmp1;
                t = next(f);
                if (t.type == OP_EQ) {
                    t = next(f);
                    node->avs.named.expressionList = parseExpressionList(t);
                }
            }
            else if (t.type == OP_EQ) {
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
            node->funcName = expect(TK_ID, "function should have a name if it's not an anonymous function").lexeme;
            t = next(f);
            node->signature = parseSignature(t);
            node->functionBody = parseFunctionBody(t);
        }
        return node;
    };
    parseFunctionBody = [&](Token&t)->AstNode* {
        AstFunctionBody * node = nullptr;
        if (auto* tmp = parseBlock(t);tmp!=nullptr) {
            node = new AstFunctionBody;
            node->block = parseBlock(t);
        }
        return node;
    };
    parseBlock = [&](Token&t)->AstNode* {
        AstBlock * node = nullptr;
        if (t.type==OP_LBRACE) {
            node = new AstBlock;
            node->statementList = parseStatementList(t);
            expect(OP_RBRACE, "block should end with right brace \"}\"");
        }
        return node;
    };
    parseStatementList = [&](Token&t)->AstNode* {
        AstStatementList * node = nullptr;
        if (auto * tmp = parseStatement(t); tmp != nullptr) {
            node = new AstStatementList;
            node->statements.push_back(tmp);
            expect(OP_SEMI, "statement should seperate by semicolon");
            while ((tmp = parseStatement(t))) {
                node->statements.push_back(tmp);
                expect(OP_SEMI, "statement should seperate by semicolon");
            }
        }
        return node;
    };
    parseStatement = [&](Token&t)->AstNode* {
        AstStatement * node = nullptr;
        if (auto*tmp = parseDeclaration(t); tmp != nullptr) {
            node = new AstStatement;
            node->as.declaration = tmp;
        }
        else  if (auto*tmp = parseLabeledStmt(t); tmp != nullptr) {
            node = new AstStatement;
            node->as.labeledStmt = tmp;
        }
        else  if (auto*tmp = parseSimpleStmt(t); tmp != nullptr) {
            node = new AstStatement;
            node->as.simpleStmt = tmp;
        }
        else  if (auto*tmp = parseGoStmt(t); tmp != nullptr) {
            node = new AstStatement;
            node->as.goStmt = tmp;
        }
        else  if (auto*tmp = parseReturnStmt(t); tmp != nullptr) {
            node = new AstStatement;
            node->as.returnStmt = tmp;
        }
        else  if (auto*tmp = parseBreakStmt(t); tmp != nullptr) {
            node = new AstStatement;
            node->as.breakStmt = tmp;
        }
        else  if (auto*tmp = parseContinueStmt(t); tmp != nullptr) {
            node = new AstStatement;
            node->as.continueStmt = tmp;
        }
        else  if (auto*tmp = parseGotoStmt(t); tmp != nullptr) {
            node = new AstStatement;
            node->as.gotoStmt = tmp;
        }
        else  if (auto*tmp = parseFallthroughStmt(t); tmp != nullptr) {
            node = new AstStatement;
            node->as.fallthroughStmt = tmp;
        }
        else  if (auto*tmp = parseBlock(t); tmp != nullptr) {
            node = new AstStatement;
            node->as.block = tmp;
        }
        else  if (auto*tmp = parseIfStmt(t); tmp != nullptr) {
            node = new AstStatement;
            node->as.ifStmt = tmp;
        }
        else  if (auto*tmp = parseSwitchStmt(t); tmp != nullptr) {
            node = new AstStatement;
            node->as.switchStmt = tmp;
        }
        else  if (auto*tmp = parseSelectStmt(t); tmp != nullptr) {
            node = new AstStatement;
            node->as.selectStmt = tmp;
        }
        else  if (auto*tmp = parseForStmt(t); tmp != nullptr) {
            node = new AstStatement;
            node->as.forStmt = tmp;
        }
        else  if (auto*tmp = parseDeferStmt(t); tmp != nullptr) {
            node = new AstStatement;
            node->as.deferStmt = tmp;
        }
        return node;
    };
    parseLabeledStmt = [&](Token&t)->AstNode* {
        AstLabeledStmt * node = nullptr;
        if(t.type==TK_ID){
            node = new AstLabeledStmt;
            node->identifier = t.lexeme;
            expect(OP_COLON,"label statement should have a colon");
            node->statement = parseStatement(t);
        }
        return node;
    };
    parseSimpleStmt = [&](Token&t)->AstNode* {
        AstSimpleStmt * node = nullptr;
        //if(auto* tmp = parseEmptyStmt(t); tmp!=nullptr){
        //    node = new AstEmptyStmt;
        //    node->ass.EmptyStmt = tmp;
       // }
        if(auto* tmp = parseExpressionStmt(t); tmp!=nullptr){
            node = new AstSimpleStmt;
            node->ass.expressionStmt = tmp;
        }else if(auto* tmp = parseSendStmt(t); tmp!=nullptr){
            node = new AstSimpleStmt;
            node->ass.sendStmt = tmp;
        }else if(auto* tmp = parseIncDecStmt(t); tmp!=nullptr){
            node = new AstSimpleStmt;
            node->ass.incDecStmt = tmp;
        }else if(auto* tmp = parseAssignment(t); tmp!=nullptr){
            node = new AstSimpleStmt;
            node->ass.assignment = tmp;
        }else if(auto* tmp = parseShortVarDecl(t); tmp!=nullptr){
            node = new AstSimpleStmt;
            node->ass.shortVarDecl = tmp;
        }
        return node;
    };
    parseGoStmt = [&](Token&t)->AstNode* {
        AstGoStmt * node = nullptr;
        if(t.type==KW_go){
            node = new AstGoStmt;
            node->expression = parseExpression(t);
        }
        return node;
    };
    parseReturnStmt = [&](Token&t)->AstNode* {
        AstReturnStmt * node = nullptr;
        if(t.type==KW_return){
            node = new AstReturnStmt;
            node->expressionList = parseExpressionList(t);
        }
        return node;
    };
    parseBreakStmt = [&](Token&t)->AstNode* {
        AstBreakStmt * node = nullptr;
        if(t.type==KW_break){
            node = new AstBreakStmt;
            t = next(f);
            if(t.type == TK_ID){
                node->label = t.lexeme;
            }else{
                node->label = nullptr;
            }
        }
        return node;
    };
    parseContinueStmt = [&](Token&t)->AstNode* {
        AstContinueStmt * node = nullptr;
        if(t.type==KW_continue){
            node = new AstContinueStmt;
            t = next(f);
            if(t.type == TK_ID){
                node->label = t.lexeme;
            }else{
                node->label = nullptr;
            }
        }
        return node;
    };
    parseGotoStmt = [&](Token&t)->AstNode* {
        AstGotoStmt* node = nullptr;
        if(t.type==KW_goto){
            node = new AstGotoStmt;
            node->label = expect(TK_ID,"goto statement must follow a label").lexeme;
        }
        return node;
    };
    parseFallthroughStmt = [&](Token&t)->AstNode* {
        AstFallthroughStmt* node = nullptr;
        if(t.type==KW_fallthrough){
            node = new AstFallthroughStmt;
        }
        return node;
    };
    parseIfStmt = [&](Token&t)->AstNode* {
        AstIfStmt* node = nullptr;
        if(t.type==KW_fallthrough){
            node = new AstIfStmt;
            if(auto* tmp = parseSimpleStmt(t);tmp!=nullptr){
                node->condition = tmp;
                expect(OP_SEMI,"expect an semicolon in condition part of if");
            }
            t=next(f);
            node->expression = parseExpression(t);
            t=next(f);
            node->block = parseBlock(t);
            t=next(f);
            if(t.type==KW_else){
                t=next(f);
                if(auto *tmp1=parseIfStmt(t);tmp1!=nullptr){
                    node->ais.ifStmt = tmp1;
                }else if(auto *tmp1=parseBlock(t);tmp1!=nullptr){
                    node->ais.block = tmp1;
                }else{
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
            if(auto*tmp = parseSendStmt(t); tmp != nullptr) {
                node->acc.sendStmt = tmp;
            }
            else if(auto*tmp = parseRecvStmt(t); tmp != nullptr) {
                node->acc.recvStmt = tmp;
            }else if (t.type == KW_default) {
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
            }else if (auto*tmp = parseForClause(t); tmp != nullptr) {
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
        AstForClause*node = new AstForClause;
        node->initStmt = parseSimpleStmt(t);
        expect(OP_SEMI, "expect semicolon in for clause");
        node->condition = parseExpression(t);
        expect(OP_SEMI, "expect semicolon in for clause");
        node->postStmt = parseSimpleStmt(t);
        return node;
    };
    parseRangeClause = [&](Token&t)->AstNode* {
        AstRangeClause*node = new AstRangeClause;
        if (auto*tmp = parseExpressionList(t); tmp != nullptr) {
            node->arc.expressionList = tmp;
            expect(OP_EQ, "expect =");
            t = next(f);
        }
        else if (auto* tmp = parseIdentifierList(t); tmp != nullptr) {
            node->arc.identifierList = tmp;
            expect(OP_SHORTAGN, "expect :=");
            t = next(f);
        }
        
        if (t.type == KW_range) {
            t = next(f);
            node->expression = parseExpression(t);
        }
        return node;
    };
    parseDeferStmt = [&](Token&t)->AstNode* {
        AstDeferStmt* node = nullptr;
        if(t.type==KW_defer){
            node = new AstDeferStmt;
            node->expression = parseExpression(t);
        }
        return node;
    };
    // simple statement
    //parseEmptyStmt | = [&](Token&t)->AstNode* {
    //    AstEmptyStmt* node = nullptr;
    //    return node;
    //};
    parseExpressionStmt = [&](Token&t)->AstNode* {
        AstExpressionStmt* node = nullptr;
        if(auto*tmp = parseExpression(t);tmp!=nullptr){
            node = new AstExpressionStmt;
            node->expression = tmp;
        }
        return node;
    };
    parseSendStmt = [&](Token&t)->AstNode* {
        AstSendStmt* node = nullptr;
        if(auto*tmp = parseExpression(t);tmp!=nullptr){
            node = new AstSendStmt;
            node->receiver = tmp;
            expect(OP_CHAN,"expect a channel symbol");
            node->sender = parseExpression(t);
        }
        return node;
    };
    parseIncDecStmt = [&](Token&t)->AstNode* {
        AstIncDecStmt* node = nullptr;
        if(auto*tmp = parseExpression(t);tmp!=nullptr){
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
    parseMethodDecl = [&](Token&t)->AstNode* {
        AstMethodDecl* node = nullptr;
        if (t.type == KW_func) {
            node = new AstMethodDecl;
            node->receiver = parseParameter(t);
            node->methodName = expect(TK_ID, "a method declaration must contain a name").lexeme;
            node->signature = parseSignature(t);
            node->functionBody = parseFunctionBody(t);
        }
        return node;
    };
    parseExpression = [&](Token&t)->AstNode* {
        AstExpression* node = nullptr;
        if (auto*tmp = parseUnaryExpr(t); tmp != nullptr) {
            node = new  AstExpression;
            node->ae.unaryExpr = tmp;
        }
        else if (auto*tmp = parseExpression(t); tmp != nullptr) {
            node = new  AstExpression;
            node->ae.named.lhs = tmp;
            t = next(f);
            if (t.type == OP_OR || t.type == OP_AND ||
                t.type == OP_EQ || t.type == OP_NE || t.type == OP_LT || t.type == OP_LE || t.type == OP_GT ||
                t.type == OP_GE || t.type == OP_ADD || t.type == OP_SUB || t.type == OP_BITOR || t.type == OP_XOR ||
                t.type == OP_MUL || t.type == OP_DIV || t.type == OP_MOD || t.type == OP_LSHIFT ||
                t.type == OP_RSHIFT || t.type == OP_BITAND || t.type == OP_XOR) {
                node->ae.named.binaryOp = t.type;
            }
            t = next(f);
            node->ae.named.rhs = parseExpression(t);
        }
        return node;
    };
    parseUnaryExpr = [&](Token&t)->AstNode* {
        AstUnaryExpr* node = nullptr;
        if (auto*tmp = parsePrimaryExpr(t); tmp != nullptr) {
            node = new AstUnaryExpr;
            node->aue.primaryExpr = tmp;
        }
        else if (t.type == OP_ADD || t.type == OP_SUB || t.type == OP_NOT || t.type == OP_XOR
            || t.type == OP_MUL || t.type == OP_BITAND || t.type == OP_CHAN) {
            node = new AstUnaryExpr;
            node->aue.named.unaryOp = t.type;
            t = next(f);
            node->aue.named.unaryExpr = parseUnaryExpr(t);
        }
        return node;
    };

    parsePrimaryExpr = [&](Token&t)->AstNode* {
        AstPrimaryExpr*node = nullptr;
        if (auto*tmp = parseOperand(t); tmp != nullptr) {
            node = new AstPrimaryExpr;
            node->ape.operand = tmp;
        }
        else if (auto*tmp = parseConversion(t); tmp != nullptr) {
            node = new AstPrimaryExpr;
            node->ape.conversion = tmp;
        }
        else if (auto*tmp = parseMethodExpr(t); tmp != nullptr){
            node = new AstPrimaryExpr;
            node->ape.methodExpr = tmp;
        }
        else if (auto*tmp = parseExpression(t); tmp != nullptr) {
            node = new AstPrimaryExpr;
            if (auto*tmp1 = parseSelector(t); tmp1 != nullptr) {
                node->ape.selector.primaryExpr = tmp;
                node->ape.selector.selector = tmp1;
            }else if(auto*tmp1 = parseIndex(t); tmp1 != nullptr) {
                node->ape.index.primaryExpr = tmp;
                node->ape.index.index = tmp1;
            }
            else if (auto*tmp1 = parseSlice(t); tmp1 != nullptr) {
                node->ape.slice.primaryExpr = tmp;
                node->ape.slice.slice = tmp1;
            }
            else if (auto*tmp1 = parseTypeAssertion(t); tmp1 != nullptr) {
                node->ape.typeAssertion.primaryExpr = tmp;
                node->ape.typeAssertion.typeAssertion = tmp1;
            }
            else if (auto*tmp1 = parseArgument(t); tmp1 != nullptr) {
                node->ape.argument.primaryExpr = tmp;
                node->ape.argument.argument = tmp1;
            }
        }
        return node;
    };
    parseSelector = [&](Token&t)->AstNode* {
        AstSelector*node = nullptr;
        if (t.type == OP_DOT) {
            node = new AstSelector;
            node->identifier = expect(TK_ID, "expect an identifier in selector").lexeme;
        }
        return node;
    };
    parseIndex = [&](Token&t)->AstNode* {
        AstIndex*node = nullptr;
        if (t.type == OP_LBRACKET) {
            node = new AstIndex;
            node->expression = parseExpression(t);
            expect(OP_RBRACKET, "bracket [] must match");
        }
        return node;
    };
    parseSlice = [&](Token&t)->AstNode* {
        AstSlice*node = nullptr;
        if (t.type == OP_LBRACKET) {
            node = new AstSlice;
            node->start = parseExpression(t);
            expect(OP_COLON, "expect colon");
            node->stop = parseExpression(t);
            t = next(f);
            if (t.type == OP_RBRACKET) {
                t = next(f);
            }
            else if (t.type == OP_COLON) {
                t = next(f);
                node->step = parseExpression(t);
                expect(OP_RBRACKET, "bracket [] must match");
            }
        }
        return node;
    };
    parseTypeAssertion = [&](Token&t)->AstNode* {
        AstTypeAssertion*node = nullptr;
        if (t.type == OP_DOT) {
            node = new AstTypeAssertion;
            expect(OP_LPAREN, "expect (");
            node->type = parseType(t);
            expect(OP_RPAREN, "expect )");
        }
        return node;
    };
    parseArgument = [&](Token&t)->AstNode* {
        AstArgument*node = nullptr;
        if (t.type == OP_LPAREN) {
            node = new AstArgument;
            if (auto*tmp = parseExpressionList(t); tmp != nullptr) {
                node->aa.expressionList = tmp;
            }
            else if (auto*tmp = parseType(t); tmp != nullptr) {
                node->aa.named.type = tmp;
                t = next(f);
                if (t.type == OP_COMMA) {
                    node->aa.named.expressionList = parseExpressionList(t);
                }
            }
            t = next(f);
            if (t.type == OP_VARIADIC) {
                node->isVariadic = true;
                t = next(f);
            }
            if (t.type == OP_COMMA) {
                t = next(f);
            }
            if (t.type != OP_RPAREN) {
                throw runtime_error("expect )");
            }
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
        else if (t.type==OP_LPAREN) {
            node = new AstOperand;
            node->ao.expression = parseExpression(t);
            expect(OP_RPAREN, "expect )");
        }
        return node;
    };
    parseOperandName = [&](Token&t)->AstNode* {
        AstOperandName*node = nullptr;
        if (t.type == TK_ID) {
            node = new AstOperandName;
            string operandName = t.lexeme;
            if (operandName == grt.package) {
                expect(OP_DOT, "expect dot");
                t = next(f);
                operandName += t.lexeme;
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
            node->lit = t.type;
        }
        return node;
    };
    parseCompositeLit = [&](Token&t)->AstNode* {
        AstCompositeLit* node = nullptr;
        if (auto*tmp = parseStructType(t); tmp != nullptr) {
            node = new AstCompositeLit;
            node->literalType = tmp;
            t = next(f);
            node->literalValue = parseLiteralValue(t);
        }
        else if (auto*tmp = parseArrayType(t); tmp != nullptr) {
            node = new AstCompositeLit;
            node->literalType = tmp;
            t = next(f);
            node->literalValue = parseLiteralValue(t);
        }
        else if (t.type == OP_LBRACKET) {
            node = new AstCompositeLit;
            expect(OP_VARIADIC, "expect variadic");
            expect(OP_RBRACKET, "expect ]");
            node->literalType = parseType(t);
            t = next(f);
            node->literalValue = parseLiteralValue(t);
        }
        else if (auto*tmp = parseSliceType(t); tmp != nullptr) {
            node = new AstCompositeLit;
            node->literalType = tmp;
            t = next(f);
            node->literalValue = parseLiteralValue(t);
        }
        else if(auto*tmp = parseMapType(t); tmp != nullptr) {
            node = new AstCompositeLit;
            node->literalType = tmp;
            t = next(f);
            node->literalValue = parseLiteralValue(t);
        }
        else if (auto*tmp = parseTypeName(t); tmp != nullptr) {
            node = new AstCompositeLit;
            node->literalType = tmp;
            t = next(f);
            node->literalValue = parseLiteralValue(t);
        }
        return node;
    };
    parseLiteralValue = [&](Token&t)->AstNode* {
        AstLiteralValue*node = nullptr;
        if (t.type == OP_LBRACE) {
            node = new AstLiteralValue;
            if (auto*tmp = parseElementList(t); tmp != nullptr) {
                node->elementList = tmp;
                t = next(f);
                if (t.type == OP_COMMA) {
                    t = next(f);
                }
            }
            if (t.type != OP_RBRACE) {
                throw runtime_error("brace {} must match");
            }
        }
        return node;
    };
    parseElementList = [&](Token&t)->AstNode* {
        AstElementList*node = nullptr;
        if (auto*tmp = parseKeyedElement(t); tmp != nullptr) {
            node = new AstElementList;
            node->keyedElement.push_back(tmp);
            t = next(f);
            while (t.type == OP_COMMA) {
                t = next(f);
                node->keyedElement.push_back(parseKeyedElement(t));
                t = next(f);
            }
        }
        return node;
    };
    parseKeyedElement = [&](Token&t)->AstNode* {
        AstKeyedElement*node = nullptr;
        if (auto*tmp = parseKey(t); tmp != nullptr) {
            node = new AstKeyedElement;
            node->key = tmp;
            expect(OP_COLON, "expect :");
            t = next(f);
        }
        if (auto*tmp = parseElement(t);tmp!=nullptr) {
            if (node == nullptr) {
                node = new AstKeyedElement;
            }
            node->element = tmp;
        }
        return node;
    };
    parseKey = [&](Token&t)->AstNode* {
        AstKey*node = nullptr;
        if (auto*tmp = parseFieldName(t); tmp != nullptr) {
            node = new AstKey;
            node->ak.fieldName = tmp;
        }
        else if (auto*tmp = parseExpression(t); tmp != nullptr) {
            node = new AstKey;
            node->ak.expression = tmp;
        }
        else if (auto*tmp = parseLiteralValue(t); tmp != nullptr) {
            node = new AstKey;
            node->ak.literalValue = tmp;
        }
        return node;
    };
    parseFieldName = [&](Token&t)->AstNode* {
        AstFieldName* node = nullptr;
        if (t.type == TK_ID) {
            node = new AstFieldName;
            node->fieldName = t.lexeme;
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
            node->functionBody = parseFunctionBody(t);
        }
        return node;
    };
    parseConversion = [&](Token&t)->AstNode* {
        AstConversion*node = nullptr;
        if (auto*tmp = parseType(t); tmp != nullptr) {
            node = new AstConversion;
            node->type = tmp;
            expect(OP_LPAREN, "expectn (");
            t = next(f);
            node->expression = parseExpression(t);
            t = next(f);
            if (t.type == OP_COMMA) {
                t = next(f);
            }
            if (t.type != OP_RPAREN) {
                throw runtime_error("expect )");
            }
        }
        return node;
    };
    parseMethodExpr = [&](Token&t)->AstNode* {
        AstMethodExpr*node = nullptr;
        if (auto*tmp = parseType(t); tmp != nullptr) {
            node = new AstMethodExpr;
            node->receiverType = tmp;
            expect(OP_DOT, "expect dot in method expression");
            t = next(f);
            node->methodName = expect(TK_ID, "expect method name").lexeme;
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
    while (f.good()) {
        auto[token, lexeme] = next(f);
        fprintf(stdout, "<%d,%s,%d,%d>\n", token, lexeme.c_str(), line, column);
    }
}

int main() {
    const string filename = "C:\\Users\\Cthulhu\\Desktop\\g5\\test\\consts.go";
    printLex(filename);
    parse(filename);
    getchar();
    return 0;
}