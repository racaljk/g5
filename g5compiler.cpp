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

enum TokenType : signed int{
    KW_break, KW_default, KW_func, KW_interface, KW_select, KW_case, KW_defer,
    KW_go, KW_map, KW_struct, KW_chan, KW_else, KW_goto, KW_package, KW_switch,
    KW_const, KW_fallthrough, KW_if, KW_range, KW_type, KW_continue, KW_for,
    KW_import, KW_return, KW_var, OP_ADD, OP_BITAND, OP_ADDASSIGN, OP_BITANDASSIGN,
    OP_AND, OP_EQ, OP_NE, OP_LPAREN, OP_RPAREN, OP_SUB, OP_BITOR, OP_SUBASSIGN,
    OP_BITORASSIGN, OP_OR, OP_LT, OP_LE, OP_LBRACKET, OP_RBRACKET, OP_MUL, OP_XOR,
    OP_MULASSIGN, OP_BITXORASSIGN, OP_CHAN, OP_GT, OP_GE, OP_LBRACE, OP_RBRACE,
    OP_DIV, OP_LSHIFT, OP_DIVASSIGN, OP_LSFTASSIGN, OP_INC, OP_ASSIGN, OP_SHORTASSIGN,
    OP_COMMA, OP_SEMI, OP_MOD, OP_RSHIFT, OP_MODASSIGN, OP_RSFTASSIGN, OP_DEC,
    OP_NOT, OP_VARIADIC, OP_DOT, OP_COLON, OP_ANDXOR, OP_ANDXORASSIGN, TK_ID,
    LITERAL_INT, LITERAL_FLOAT, LITERAL_IMG, LITERAL_RUNE, LITERAL_STR, TK_EOF
};

//===----------------------------------------------------------------------===//
// global data
//===----------------------------------------------------------------------===//
static int line = 1, column = 1, lastToken = -1;
struct Token { 
    TokenType type; string lexeme; 
    Token(TokenType a, const string&b) :type(a), lexeme(b) {} 
};
static struct goruntime{
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
                || lastToken == KW_fallthrough||lastToken == KW_continue
                || lastToken == KW_return || lastToken == KW_break
                ||lastToken == OP_INC || lastToken == OP_DEC || lastToken == OP_RPAREN 
                || lastToken == OP_RBRACKET || lastToken == OP_RBRACE) {
                consumePeek(c);
                lastToken = OP_SEMI;
                return Token(OP_SEMI, ";");
            }
        }
        consumePeek(c);
    }
    if (f.eof()) {
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
            lastToken = OP_ADDASSIGN;
            return Token(OP_ADDASSIGN, lexeme);
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
            lastToken = OP_BITANDASSIGN;
            return Token(OP_BITANDASSIGN, lexeme);
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
                lastToken = OP_ANDXORASSIGN;
                return Token(OP_ANDXORASSIGN, lexeme);
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
        lastToken = OP_ASSIGN;
        return Token(OP_ASSIGN, lexeme);
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
            lastToken = OP_SUBASSIGN;
            return Token(OP_SUBASSIGN, lexeme);
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
            lastToken = OP_BITORASSIGN;
            return Token(OP_BITORASSIGN, lexeme);
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
                lastToken = OP_LSFTASSIGN;
                return Token(OP_LSFTASSIGN, lexeme);
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
            lastToken = OP_MULASSIGN;
            return Token(OP_MULASSIGN, lexeme);
        }
        lastToken = OP_MUL;
        return Token(OP_MUL, lexeme);
    case '^':  //^  ^=
        lexeme += consumePeek(c);
        if (c == '=') {
            lastToken = OP_BITXORASSIGN;
            return Token(OP_BITXORASSIGN, lexeme);
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
                lastToken = OP_RSFTASSIGN;
                return Token(OP_RSFTASSIGN, lexeme);
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
            lastToken = OP_DIVASSIGN;
            return Token(OP_DIVASSIGN, lexeme);
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
            lastToken = OP_SHORTASSIGN;
            return Token(OP_SHORTASSIGN, lexeme);
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
            lastToken = OP_MODASSIGN;
            return Token(OP_MODASSIGN, lexeme);
        }
        lastToken = OP_MOD;
        return Token(OP_MOD, lexeme);
        // case '.' has already checked
    }

    throw runtime_error("illegal token in source file");
}

void parse(const string & filename) {
    fstream f(filename, ios::binary | ios::in);

    auto expect = [&f](TokenType tk, const string& msg) {
        auto t = next(f);
        if (t.type != tk) throw runtime_error(msg);
        return t;
    };

    struct AstNode { virtual ~AstNode() {} };
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

    struct AstFunctionDecl :public AstNode {

    };

    struct AstMethodDecl : public AstNode {

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
    struct AstIdentifierList :public AstNode {
        vector<string> identifierList;
    };
    struct AstTypeName : public AstNode {
        string typeName;
    };
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
    struct  AstStructType :public AstNode {
        AstNode* 
    };
    

    function<AstNode*()> parseSourceFile;
    function<AstNode*(Token&)>parsePackageClause, parseImportDecl, parseTopLevelDecl,
        parseDeclaration, parseConstDecl, parseIdentifierList,parseType, parseTypeName,
        parseTypeLit,parseArrayType,parsStructType;

    parseSourceFile = [&]()->AstNode* {
        auto node = new AstSourceFile;
        auto t = next(f);

        node->packageClause = parsePackageClause(t);

        expect(OP_SEMI, "expect a semicolon");
        t = next(f);
        while (t.type == KW_import) {
            node->importDecl.push_back(parseImportDecl(t));
            t = next(f);
        }
        // todo: fix TK_EOF producing bug
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
        }else if (auto* tmp = parseFunctionDecl(t); tmp != nullptr) {
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
            node->ad.constDecl=tmp;
        }else  if (auto*tmp = parseTypeDecl(t); tmp != nullptr) {
            node = new AstDeclaration;
            node->ad.typeDecl = tmp;
        }else  if (auto*tmp = parseVarDecl(t); tmp != nullptr) {
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



    // Type = TypeName | TypeLit | "(" Type ")" .
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
        else  if (t.type==OP_LPAREN) {
            t = next(f);
            node = dynamic_cast<AstType*>(parseType(t));
            expect(OP_RPAREN, "the parenthesis () must match in type declaration");
        }
        return node;
    };

    // TypeName = identifier | QualifiedIdent .
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

    // TypeLit = ArrayType | StructType | PointerType | FunctionType | InterfaceType |
    // SliceType | MapType | ChannelType .
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
        if (t.type == OP_LBRACKET) {
            node = new  AstStructType;
            node->length = parseExpression(t);
            expect(OP_RBRACKET, "bracket [] must match in array type declaration");
            node->elementType = parseType(t);
        }
        return node;
    };

    parseIdentifierList = [&](Token&t)->AstNode* {
        auto* node = new AstIdentifierList;
        node->identifierList.emplace_back(expect(TK_ID, "it shall be an identifier").lexeme);
        t = next(f);
        if (t.type == OP_COMMA)
            while (t.type == OP_COMMA) {
                node->identifierList.emplace_back(expect(TK_ID, "it shall be an identifier").lexeme);
                t = next(f);
            }

        return node;
    };


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
    //printLex(filename);
    parse(filename);
    getchar();
    return 0;
}