//===---------------------------------------------------------------------------------------===//
// g5 : golang compiler and runtime in 5 named functions
// Copyright (C) 2018 racaljk<1948638989@qq.com>.
//
// This program is free software: you can redistribute it and/or modify it under the terms of the 
// GNU General Public License as published by the Free Software Foundation, either version 3 of 
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without 
// even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
// General Public License for more details. You should have received a copy of the GNU General 
// Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
//===---------------------------------------------------------------------------------------===//
#include <cstdio>
#include <exception>
#include <fstream>
#include <functional>
#include <string>
#include <vector>
#include <tuple>
#include <map>
#include <optional>
#define inrange(c,begin,end) (c>=begin && c<=end)
#define LAMBDA_FUN(X) function<X*(Token&)> parse##X;
#define REPORT_ERR(PRE,STR) \
{fprintf(stderr,"%s:%s at line %d, col %d\n",#PRE,#STR,line,column);\
exit(EXIT_FAILURE);}
using namespace std;
//===---------------------------------------------------------------------------------------===//
// global data
//===---------------------------------------------------------------------------------------===//
string keywords[] = { "break",    "default",     "func",   "interface", "select",
                     "case",     "defer",       "go",     "map",       "struct",
                     "chan",     "else",        "goto",   "package",   "switch",
                     "const",    "fallthrough", "if",     "range",     "type",
                     "continue", "for",         "import", "return",    "var" };
static int line = 1, column = 1, lastToken = 0, shouldEof = 0, nestLev = 0;
static struct goruntime {} grt;
static auto anyone = [](auto&& k, auto&&... args) ->bool { return ((args == k) || ...); };
//===---------------------------------------------------------------------------------------===//
// various declarations which contains TokenType for lexical analysis and AST node definitions 
// for syntax parsing
//===---------------------------------------------------------------------------------------===//
#pragma region GlobalDecl
enum TokenType : signed int {
    INVALID = 0, KW_break, KW_default, KW_func, KW_interface, KW_select, KW_case, KW_defer, KW_go, 
    KW_map, KW_struct, KW_chan, KW_else, KW_goto, KW_package, KW_switch, KW_const, KW_fallthrough, 
    KW_if, KW_range, KW_type, KW_continue, KW_for, KW_import, KW_return, KW_var, OP_ADD, OP_BITAND,
    OP_ADDAGN, OP_SHORTAGN, OP_AND, OP_EQ, OP_NE, OP_LPAREN, OP_RPAREN, OP_SUB, OP_BITOR, OP_SUBAGN,
    OP_ORAGN, OP_OR, OP_LT, OP_LE, OP_LBRACKET, OP_RBRACKET, OP_MUL, OP_XOR, OP_MULAGN, OP_XORAGN,
    OP_CHAN, OP_GT, OP_GE, OP_LBRACE, OP_RBRACE, OP_DIV, OP_LSHIFT, OP_DIVAGN, OP_LSFTAGN, OP_INC, 
    OP_AGN, OP_ANDAGN, OP_COMMA, OP_SEMI, OP_MOD, OP_RSHIFT, OP_MODAGN, OP_RSFTAGN, OP_DEC, OP_NOT,
    OP_VARIADIC, OP_DOT, OP_COLON, OP_ANDXOR, OP_ANDXORAGN, TK_ID, LIT_INT, LIT_FLOAT, LIT_IMG, 
    LIT_RUNE, LIT_STR, TK_EOF = -1,};
// TODO: add destructor for them
// Common
#define _S :public Stmt
#define _E :public Expr
#define _N :public Node
struct UnaryExpr;
struct Node                { virtual ~Node() = default; };
struct Expr             _N { UnaryExpr* lhs{}; TokenType op{}; Expr* rhs{}; };
struct UnaryExpr        _E { Expr*expr{}; TokenType op{}; };
struct PrimaryExpr      _E { Expr* expr{}; };
struct Stmt             _N {};
struct IdentList        _E { vector<string> identList; };
struct ExprList         _E { vector<Expr*> exprList; };
struct StmtList         _E { vector<Stmt*> stmtList; };
// Statement
struct BlockStmt        _S { StmtList* stmtList{}; };
struct GoStmt           _S { Expr* expr{}; GoStmt(Expr* expr) :expr(expr) {} };
struct ReturnStmt       _S { ExprList* exprList{}; ReturnStmt(ExprList* el) :exprList(el) {} };
struct BreakStmt        _S { string label; BreakStmt(const string&s) :label(s) {} };
struct DeferStmt        _S { Expr* expr{}; DeferStmt(Expr* expr) :expr(expr) {} };
struct ContinueStmt     _S { string label; ContinueStmt(const string&s) :label(s) {} };
struct GotoStmt         _S { string label; GotoStmt(const string&s) :label(s) {} };
struct FallthroughStmt  _S {};
struct LabeledStmt      _S { string label; Stmt* stmt{}; LabeledStmt(const string&s, Stmt*st) :label(s), stmt(st) {} };
struct IfStmt           _S { Stmt* init{}, *ifBlock{}, *elseBlock{}; Expr* cond{}; };
struct SwitchCase          { ExprList* exprList{}; StmtList* stmtList{}; };
struct SwitchStmt       _S { Stmt* init{}, *cond{}; vector<SwitchCase*> caseList{}; };
struct SelectCase          {StmtList* stmtList{};};
struct SelectStmt       _S {vector<SelectCase*> caseList;};
struct ForStmt          _S { Node* init{}, *cond{}, *post{}; BlockStmt* block{}; };
struct SRangeClause     _S {vector<string> lhs;Expr* rhs{};};
struct RangeClause      _S { ExprList* lhs{}; TokenType op; Expr* rhs{}; };
struct ExprStmt         _S { Expr* expr{}; };
struct SendStmt         _S { Expr* receiver{}, *sender{}; };
struct IncDecStmt       _S { Expr* expr{}; bool isInc{}; };
struct AssignStmt       _S { ExprList* lhs{}, *rhs{}; TokenType op{}; };
struct SAssignStmt      _S { vector<string> lhs{}; ExprList* rhs{}; };
// Expression
struct SelectorExpr     _E { Expr* operand{}; string selector; SelectorExpr(Expr*e, string s):operand(e),selector(s) {} };
struct TypeSwitchExpr   _E { Expr* operand{}; TypeSwitchExpr(Expr*e) :operand(e) {} };
struct TypeAssertExpr   _E { Expr* operand{}, *type{}; TypeAssertExpr(Expr*e, Expr *t) :operand(e), type(t) {} };
struct IndexExpr        _E { Expr* operand{}, *index{}; IndexExpr(Expr*e, Expr*i) :operand(e), index(i) {} };
struct SliceExpr        _E { Expr* operand{}, *begin{}, *end{}, *step{}; };
struct CallExpr         _E { Expr* operand{}, *type{}; ExprList* arguments{}; bool isVariadic{}; };
struct KeyedElement        { Expr*key{}, *elem{}; };
struct LitValue         _E { vector<KeyedElement*> keyedElement; };
struct BasicLit         _E { TokenType type{}; string value; BasicLit(TokenType t, const string&s) :type(t), value(s) {} };
struct CompositeLit     _E { Expr* litName{}; LitValue* litValue{}; };
struct Name             _E { string name; };
struct ArrayType        _E { Expr* len{}; Expr* elem{}; bool autoLen = false; };
struct StructType       _E { vector<tuple<Expr*, Expr*, string, bool>> fields; };
struct PtrType          _E { Expr* elem{}; PtrType(Expr*e) :elem(e) {} };
struct ParamDecl           { bool isVariadic = false, hasName = false; Expr* type{}; string name; };
struct Param               { vector<ParamDecl*> paramList; };
struct Signature           { Param* param{}, *resultParam{}; Expr* resultType{}; };
struct FuncType         _E { Signature * signature{};FuncType(Signature* s):signature(s){} };
struct InterfaceType    _E { vector<tuple<Name*, Signature*>> method; };
struct SliceType        _E { Expr* elem{}; };
struct MapType          _E { Expr* type{}, *elem{}; };
struct ChanType         _E { Expr* elem{}; };
// Declaration
struct ImportDecl          { map<string, string> imports; };
struct ConstDecl        _S { vector<IdentList*> identList; vector<Expr*> type; vector<ExprList*> exprList; };
struct TypeSpec            { string ident; Expr* type; };
struct TypeDecl         _S { vector<TypeSpec*> typeSpec; };
struct VarSpec             { IdentList* identList{}; ExprList* exprList{}; Expr* type{}; };
struct VarDecl          _S { vector<VarSpec*> varSpec; };
// Freak
struct FuncDecl:public Stmt,Expr{ string funcName;Param* receiver{};Signature* signature{};BlockStmt* funcBody{};};
struct CompilationUnit {
    string package;
    vector<ImportDecl*> importDecl;
    vector<ConstDecl*> constDecl;
    vector<TypeDecl*> typeDecl;
    vector<FuncDecl*> funcDecl;
    vector<VarDecl*> varDecl;
};
struct Token { 
    TokenType type{}; string lexeme;
    Token(TokenType t, string e) :type(t), lexeme(e) { lastToken = t; }
};
#pragma endregion
//===---------------------------------------------------------------------------------------===//
// Implementation of golang compiler and runtime within 5 explicit functions
//===---------------------------------------------------------------------------------------===//
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
    for (; anyone(c, ' ', '\r', '\t', '\n'); column++) {
        if (c == '\n') {
            line++;
            column = 1;
            if (anyone(lastToken, TK_ID, LIT_INT, LIT_FLOAT, LIT_IMG, LIT_RUNE, LIT_STR, KW_fallthrough,
                KW_continue, KW_return, KW_break, OP_INC, OP_DEC, OP_RPAREN, OP_RBRACKET, OP_RBRACE)) {
                consumePeek(c);
                return Token(OP_SEMI, ";");
            }
        }
        consumePeek(c);
    }
    if (f.eof()) {
        if (shouldEof) return Token(TK_EOF, "");
        shouldEof = 1;
        return Token(OP_SEMI, ";");
    }

    string lexeme;
    // identifier
    if (inrange(c, 'a', 'z') || inrange(c, 'A', 'Z') || c == '_') {
        while (inrange(c, 'a', 'z') || inrange(c, 'A', 'Z') || inrange(c,'0','9') || c == '_') {
            lexeme += consumePeek(c);
        }

        for (int i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++)
            if (keywords[i] == lexeme)  return Token(static_cast<TokenType>(i + 1), lexeme);
        return Token(TK_ID, lexeme);
    }

    // decimal 
    if (c >= '0'&&c <= '9' || c == '.') {
        if (c == '0') {
            lexeme += consumePeek(c);
            if (c == 'x' || c == 'X') {
                do {
                    lexeme += consumePeek(c);
                } while (inrange(c, '0', '9') || inrange(c, 'a', 'f') || inrange(c, 'A', 'F'));
                return Token(LIT_INT, lexeme);
            } else if (inrange(c, '0', '9') || anyone(c, '.', 'e', 'E', 'i')) {
                while (inrange(c, '0', '9') || anyone(c, '.', 'e', 'E', 'i')) {
                    if (inrange(c, '0', '7')) {
                        lexeme += consumePeek(c);
                    } else {
                        goto shall_float;
                    }
                }
                return Token(LIT_INT, lexeme);
            }
            goto may_float;
        } else {  // 1-9 or . or just a single 0
        may_float:
            TokenType type = LIT_INT;
            if (c == '.') {
                lexeme += consumePeek(c);
                if (c == '.') {
                    lexeme += consumePeek(c);
                    if (c == '.') {
                        lexeme += consumePeek(c);
                        return Token(OP_VARIADIC, lexeme);
                    } else REPORT_ERR("lex error", "expect variadic notation(...)");
                } else if (inrange(c, '0', '9')) {
                    type = LIT_FLOAT;
                } else {
                    return Token(OP_DOT, lexeme);
                }
                goto shall_float;
            } else if (inrange(c, '1', '9')) {
                lexeme += consumePeek(c);
            shall_float:  // skip char consuming and appending since we did that before jumping here;
                bool hasDot = false, hasExponent = false;
                while (inrange(c, '0', '9') || anyone(c, '.', 'e', 'E', 'i')) {
                    if (inrange(c, '0', '9')) {
                        lexeme += consumePeek(c);
                    } else if (c == '.' && !hasDot) {
                        lexeme += consumePeek(c);
                        type = LIT_FLOAT;
                    } else if ((c == 'e' && !hasExponent) || (c == 'E' && !hasExponent)) {
                        hasExponent = true;
                        type = LIT_FLOAT;
                        lexeme += consumePeek(c);
                        if (c == '+' || c == '-') lexeme += consumePeek(c);
                    } else {
                        f.get();
                        column++;
                        lexeme += c;
                        return Token(LIT_IMG, lexeme);
                    }
                }
                return Token(type, lexeme);
            } else return Token(type, lexeme);
        }
    }
    // literal
    if (c == '\'') {
        lexeme += consumePeek(c);
        if (c == '\\') {
            lexeme += consumePeek(c);
            if (anyone(c, 'U', 'u', 'X', 'x'))
                do lexeme += consumePeek(c); 
                while (inrange(c, '0', '9') || inrange(c, 'A', 'F') || inrange(c, 'a', 'f'));
            else if (inrange(c, '0', '7'))
                do lexeme += consumePeek(c); while (inrange(c, '0', '7'));
            else if (anyone(c, 'a', 'b', 'f', 'n', 'r', 't', 'v', '\\', '\'', '"'))
                lexeme += consumePeek(c);
            else REPORT_ERR("lex error", "illegal rune");
        } else lexeme += consumePeek(c);

        if (c != '\'') {
            REPORT_ERR("lex error", "illegal rune at least in current implementation of g8");
        }
        lexeme += consumePeek(c);
        return Token(LIT_RUNE, lexeme);
    }

    // string literal
    if (c == '`') {
        do {
            lexeme += consumePeek(c);
            if (c == '\n') line++;
        } while (f.good() && c != '`');
        if (c != '`') {
            REPORT_ERR("lex error", "raw string literal does not have a closed symbol \"`\"");
        }
        lexeme += consumePeek(c);
        return Token(LIT_STR, lexeme);
    } else if (c == '"') {
        do {
            lexeme += consumePeek(c);
            if (c == '\\') {
                lexeme += consumePeek(c);
                lexeme += consumePeek(c);
            }
        } while (f.good() && (c != '\n' && c != '\r' && c != '"'));
        if (c != '"') {
            REPORT_ERR("lex error", "string literal does not have a closed symbol");
        }
        lexeme += consumePeek(c);
        return Token(LIT_STR, lexeme);
    }

    auto match = [&](initializer_list<tuple<pair<char, TokenType>,
        initializer_list<pair<string_view, TokenType>>,
        pair<string_view, TokenType>>> big) ->Token {
        for (const auto&[v1, v2, v3] : big) {
            if (c == v1.first) {
                lexeme += consumePeek(c);
                for (const auto &[v2str, v2type] : v2) {
                    if (c == v2str[1]) {
                        lexeme += consumePeek(c);
                        if (const auto&[v3str, v3type] = v3; v3type != INVALID) {
                            if (c == v3str[2]) {
                                lexeme += consumePeek(c);
                                return Token(v3type, lexeme);
                            }
                        }
                        return Token(v2type, lexeme);
                    }
                }
                return Token(v1.second, lexeme);
            }
        }

        return Token(INVALID, "");
    };

    // operators
    if(c=='/') { // special case for /  /= // /*...*/
        char pending = consumePeek(c);
        if (c == '=') {
            lexeme += pending;
            lexeme += consumePeek(c);
            return Token(OP_DIVAGN, lexeme);
        } else if (c == '/') {
            do consumePeek(c); while (f.good() && (c != '\n' && c != '\r'));
            goto skip_comment_and_find_next;
        } else if (c == '*') {
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
        return Token(OP_DIV, lexeme);
    }
    auto result = match({
        {{ '+',OP_ADD },    {{"+=",OP_ADDAGN} ,{"++",OP_INC}},                  {}},
        {{'&',OP_BITAND},   {{"&=",OP_ANDAGN},{"&&",OP_AND},{"&^",OP_ANDXOR}},  {"&^=",OP_ANDXORAGN}},
        {{'=',OP_AGN},      {{"==",OP_EQ}},                                     {}},
        {{'!',OP_NOT},      {{"!=",OP_NE}},                                     {}},
        {{'(',OP_LPAREN},   {},                                                 {}},
        {{')',OP_RPAREN},   {},                                                 {}},
        {{'-',OP_SUB},      {{"-=",OP_SUBAGN},{"--",OP_DEC}},                   {}},
        {{'|',OP_BITOR},    {{"|=",OP_ORAGN},{"||",OP_OR}},                     {}},
        {{'<',OP_LT},       {{"<=",OP_LE},{"<-",OP_CHAN},{"<<",OP_LSHIFT}},     {"<<=",OP_LSFTAGN}},
        {{'[',OP_LBRACKET}, {},                                                 {}},
        {{']',OP_RBRACKET}, {},                                                 {}},
        {{'*',OP_MUL},      {{"*=",OP_MULAGN}},                                 {}},
        {{'^',OP_XOR},      {{"^=",OP_XORAGN}},                                 {}},
        {{'>',OP_GT},       {{">=",OP_GE},{">>",OP_RSHIFT}},                    {">>=",OP_RSFTAGN}},
        {{'{',OP_LBRACE},   {},                                                 {}},
        {{'}',OP_RBRACE},   {},                                                 {}},
        {{':',OP_COLON},    {{":=",OP_SHORTAGN}},                               {}},
        {{',',OP_COMMA},    {},                                                 {}},
        {{';',OP_SEMI},     {},                                                 {}},
        {{'%',OP_MOD},      {{"%=",OP_MODAGN}},                                 {}},
    });

    if (result.type != INVALID) { return result; }
    else REPORT_ERR("lex error", "illegal token in source file");
}

const auto parse(const string & filename) {
    fstream f(filename, ios::binary | ios::in);
    auto t = next(f);

    auto eat = [&](TokenType tk, const string&msg) {
        if (t.type != tk) throw runtime_error(msg);
        t = next(f);
    };
    auto eatOptionalSemi = [&]() { if (t.type == OP_SEMI) t = next(f); };

    LAMBDA_FUN(LitValue);LAMBDA_FUN(Expr); LAMBDA_FUN(UnaryExpr);
    LAMBDA_FUN(PrimaryExpr); LAMBDA_FUN(Stmt); LAMBDA_FUN(IfStmt);
    function<Expr*(Token&)> parseType;

#pragma region Common
    auto parseName = [&](bool couldFullName, Token&t) {
        Name * node{};
        if (t.type == TK_ID) {
            node = new Name;
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
    auto parseIdentList = [&](Token&t) {
        IdentList* node{};
        if (t.type == TK_ID) {
            node = new  IdentList;
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
    auto parseExprList = [&](Token&t) {
        ExprList* node{};
        if (auto* tmp = parseExpr(t); tmp != nullptr) {
            node = new  ExprList;
            node->exprList.emplace_back(tmp);
            while (t.type == OP_COMMA) {
                t = next(f);
                node->exprList.emplace_back(parseExpr(t));
            }
        }
        return node;
    };
    auto parseStmtList = [&](Token&t) {
        StmtList * node{};
        Stmt* tmp = nullptr;
        while ((tmp = parseStmt(t))) {
            if (node == nullptr) node = new StmtList;
            node->stmtList.push_back(tmp);
            eatOptionalSemi();
        }
        return node;
    };
    auto parseBlock = [&](Token&t){
        BlockStmt * node{};
        if (t.type == OP_LBRACE) {
            node = new BlockStmt;
            t = next(f);
            if (t.type != OP_RBRACE) {
                node->stmtList = parseStmtList(t);
                eat(OP_RBRACE, "expect } around code block");
            } else t = next(f);
        }
        return node;
    };
#pragma endregion
#pragma region Declaration
    auto parseImportDecl = [&](Token&t) {
        auto node = new ImportDecl;
        eat(KW_import, "it should be import declaration");
        if (t.type == OP_LPAREN) {
            t = next(f);
            do {
                string importName, alias;
                if (t.type == OP_DOT || t.type == TK_ID) {
                    alias = t.lexeme;
                    t = next(f);
                    importName = t.lexeme;
                } else {
                    importName = t.lexeme;
                }
                importName = importName.substr(1, importName.length() - 2);
                node->imports[importName] = alias;
                t = next(f);
                eatOptionalSemi();
            } while (t.type != OP_RPAREN);
            eat(OP_RPAREN, "expect )");
        } else {
            string importName, alias;
            if (t.type == OP_DOT || t.type == TK_ID) {
                alias = t.lexeme;
                t = next(f);
                importName = t.lexeme;
                t = next(f);
            } else {
                importName = t.lexeme;
                t = next(f);
            }
            importName = importName.substr(1, importName.length() - 2);
            node->imports[importName] = alias;
        }
        return node;
    };
    auto parseConstDecl = [&](Token&t){
        auto * node = new ConstDecl;
        eat(KW_const, "it should be const declaration");
        if (t.type == OP_LPAREN) {
            t = next(f);
            do {
                node->identList.push_back(parseIdentList(t));
                if (auto*tmp = parseType(t); tmp != nullptr) {
                    node->type.push_back(tmp);
                } else {
                    node->type.push_back(nullptr);
                }
                if (t.type == OP_AGN) {
                    t = next(f);
                    node->exprList.push_back(parseExprList(t));
                } else {
                    node->exprList.push_back(nullptr);
                }
                eatOptionalSemi();
            } while (t.type != OP_RPAREN);
            eat(OP_RPAREN, "eat right parenthesis");
        } else {
            node->identList.push_back(parseIdentList(t));
            if (auto*tmp = parseType(t); tmp != nullptr) {
                node->type.push_back(tmp);
            } else {
                node->type.push_back(nullptr);
            }
            if (t.type == OP_AGN) {
                t = next(f);
                node->exprList.push_back(parseExprList(t));
            } else {
                node->exprList.push_back(nullptr);
            }
            if (t.type != OP_SEMI) throw runtime_error("expect an explicit semicolon");
        }

        return node;
    };
    auto parseTypeSpec = [&](Token&t) {
        TypeSpec* node{};
        if (t.type == TK_ID) {
            node = new TypeSpec;
            node->ident = t.lexeme;
            t = next(f);
            if (t.type == OP_AGN) {
                t = next(f);
            }
            node->type = parseType(t);
        }
        return node;
    };
    auto parseTypeDecl = [&](Token&t) {
        auto * node = new TypeDecl;
        eat(KW_type, "it should be type declaration");
        if (t.type == OP_LPAREN) {
            t = next(f);
            do {
                node->typeSpec.push_back(parseTypeSpec(t));
                eatOptionalSemi();
            } while (t.type != OP_RPAREN);
            eat(OP_RPAREN, "expect )");
        } else {
            node->typeSpec.push_back(parseTypeSpec(t));
        }
        return node;
    };
    auto parseVarSpec = [&](Token&t){
        VarSpec* node{};
        if (auto*tmp = parseIdentList(t); tmp != nullptr) {
            node = new VarSpec;
            node->identList = tmp;
            if (t.type == OP_AGN) {
                t = next(f);
                node->exprList = parseExprList(t);
            } else {
                node->type = parseType(t);
                if (t.type == OP_AGN) {
                    t = next(f);
                    node->exprList = parseExprList(t);
                }
            }
        }
        return node;
    };
    auto parseVarDecl = [&](Token&t) {
        auto * node = new VarDecl;
        eat(KW_var, "it should be var declaration");
        if (t.type == OP_LPAREN) {
            do {
                node->varSpec.push_back(parseVarSpec(t));
                t = next(f);
            } while (t.type != OP_RPAREN);
            eat(OP_RPAREN, "expect )");
        } else {
            node->varSpec.push_back(parseVarSpec(t));
        }

        return node;
    };
    auto parseParamDecl = [&](Token&t) {
        ParamDecl* node{};
        if (t.type == OP_VARIADIC) {
            node = new ParamDecl;
            node->isVariadic = true;
            t = next(f);
            node->type = parseType(t);
        } else if (t.type != OP_RPAREN) {
            node = new ParamDecl;
            auto*mayIdentOrType = parseType(t);
            if (t.type != OP_COMMA && t.type != OP_RPAREN) {
                node->hasName = true;
                if (t.type == OP_VARIADIC) {
                    node->isVariadic = true;
                    t = next(f);
                }
                node->name = dynamic_cast<Name*>(mayIdentOrType)->name;
                node->type = parseType(t);
            } else {
                node->type = mayIdentOrType;
            }
        }
        return node;
    };
    auto parseParam = [&](Token&t){
        Param* node{};
        if (t.type == OP_LPAREN) {
            node = new Param;
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
                if (dynamic_cast<ParamDecl*>(node->paramList[i])->hasName) {
                    for (int k = rewriteStart; k < i; k++) {
                        string name = node->paramList[k]->name;
                        node->paramList[k]->type = node->paramList[i]->type;
                        node->paramList[k]->name = name;
                        node->paramList[k]->hasName = true; //It's not necessary
                    }
                    rewriteStart = i + 1;
                }
            }
        }
        return node;
    };
    auto parseSignature = [&](Token&t){
        Signature* node{};
        if (t.type == OP_LPAREN) {
            node = new Signature;
            node->param = parseParam(t);
            if (auto*result = parseParam(t); result != nullptr) {
                node->resultParam = result;
            } else  if (auto*result = parseType(t); result != nullptr) {
                node->resultType = result;
            }
        }
        return node;
    };
    auto parseFuncDecl = [&](bool anonymous, Token&t){
        auto * node = new FuncDecl;
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
#pragma endregion
#pragma region Type
    auto parseArrayOrSliceType = [&](Token&t) {
        Expr* node{};
        eat(OP_LBRACKET, "array/slice type requires [ to denote that");
        nestLev++;
        if (t.type != OP_RBRACKET) {
            node = new ArrayType;
            if (t.type == OP_VARIADIC) {
                dynamic_cast<ArrayType*>(node)->autoLen = true;
                t = next(f);
            }
            else dynamic_cast<ArrayType*>(node)->len = parseExpr(t);
            nestLev--;
            t = next(f);
            dynamic_cast<ArrayType*>(node)->elem = parseType(t);
        }
        else {
            node = new SliceType;
            nestLev--;
            t = next(f);
            dynamic_cast<SliceType*>(node)->elem = parseType(t);
        }
        return node;
    };
    auto parseStructType = [&](Token&t){
        auto * node = new  StructType;
        eat(KW_struct, "structure type requires struct keyword");
        eat(OP_LBRACE, "a { is required after struct");
        do {
            tuple<Expr*, Expr*, string, bool> field;// <IdentList/Name,Type,Tag,isEmbeded>
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
    auto parseInterfaceType = [&](Token&t){
        auto * node = new InterfaceType;
        eat(KW_interface, "interface type requires keyword interface");
        eat(OP_LBRACE, "{ is required after interface");
        while (t.type != OP_RBRACE) {
            if (auto* tmp = parseName(true, t); tmp != nullptr && tmp->name.find('.') == string::npos) {
                node->method.emplace_back(tmp, parseSignature(t));
            }
            else {
                node->method.emplace_back(tmp, nullptr);
            }
            eatOptionalSemi();
        }
        t = next(f);
        return node;
    };
    auto parseMapType = [&](Token&t){
        auto * node = new MapType;
        eat(KW_map, "map type requires keyword map to denote that");
        eat(OP_LBRACKET, "[ is required after map");
        node->type = parseType(t);
        eat(OP_RBRACKET, "] is required after map[Key");
        node->elem = parseType(t);
        return node;
    };
    auto parseChanType = [&](Token&t){
        ChanType* node{};
        if (t.type == KW_chan) {
            node = new ChanType;
            t = next(f);
            if (t.type == OP_CHAN) {
                t = next(f);
                node->elem = parseType(t);
            }
            else {
                node->elem = parseType(t);
            }
        }
        else if (t.type == OP_CHAN) {
            node = new ChanType;
            t = next(f);
            if (t.type == KW_chan) {
                node->elem = parseType(t);
            }
        }
        return node;
    };
    parseType = [&](Token&t)->Expr* {
        switch (t.type) {
        case OP_MUL:      {t = next(f); return new PtrType(parseType(t)); }
        case KW_func:     {t = next(f); return new FuncType(parseSignature(t)); }
        case OP_LPAREN:   {t = next(f); auto*tmp = parseType(t); t = next(f); return tmp; }
        case TK_ID:       return parseName(true, t);
        case OP_LBRACKET: return parseArrayOrSliceType(t);
        case KW_struct:   return parseStructType(t);
        case KW_interface:return parseInterfaceType(t);
        case KW_map:      return parseMapType(t);
        case KW_chan:     return parseChanType(t);
        default:          return nullptr;
        }
    };
#pragma endregion
#pragma region Statement
    auto parseSimpleStmt = [&](ExprList* lhs, Token&t)->Stmt* {
        if (t.type == KW_range) {    //special case for ForStmt
            auto*stmt = new SRangeClause;
            t = next(f);
            stmt->rhs = parseExpr(t);
            return stmt;
        }
        if (lhs == nullptr) lhs = parseExprList(t);

        switch (t.type) {
        case OP_CHAN: {
            if (lhs->exprList.size() != 1) throw runtime_error("one expr required");
            t = next(f);
            auto* stmt = new SendStmt;
            stmt->receiver = lhs->exprList[0];
            stmt->sender = parseExpr(t);
            return stmt;
        }
        case OP_INC:case OP_DEC: {
            if (lhs->exprList.size() != 1) throw runtime_error("one expr required");
            auto* stmt = new IncDecStmt;
            stmt->isInc = t.type == OP_INC;
            t = next(f);
            stmt->expr = lhs->exprList[0];
            return stmt;
        }
        case OP_SHORTAGN: {
            if (lhs->exprList.empty()) throw runtime_error("one expr required");

            vector<string> identList;
            for (auto* e : lhs->exprList) {
                string identName = dynamic_cast<Name*>(dynamic_cast<PrimaryExpr*>(e->lhs->expr)->expr)->name;
                identList.push_back(identName);
            }
            t = next(f);
            if (t.type == KW_range) {
                t = next(f);
                auto* stmt = new SRangeClause;
                stmt->lhs = move(identList);
                stmt->rhs = parseExpr(t);
                return stmt;
            }
            else {
                auto*stmt = new SAssignStmt;
                stmt->lhs = move(identList);
                stmt->rhs = parseExprList(t);
                return stmt;
            }
        }
        case OP_ADDAGN:case OP_SUBAGN:case OP_ORAGN:case OP_XORAGN:case OP_MULAGN:case OP_DIVAGN:
        case OP_MODAGN:case OP_LSFTAGN:case OP_RSFTAGN:case OP_ANDAGN:case OP_ANDXORAGN:case OP_AGN: {
            if (lhs->exprList.empty()) throw runtime_error("one expr required");
            auto op = t.type;
            t = next(f);
            if (t.type == KW_range) {
                t = next(f);
                auto* stmt = new RangeClause;
                stmt->lhs = lhs;
                stmt->op = op;
                stmt->rhs = parseExpr(t);
                return stmt;
            }
            else {
                auto* stmt = new AssignStmt;
                stmt->lhs = lhs;
                stmt->op = op;
                stmt->rhs = parseExprList(t);
                return stmt;
            }

        }
        default: {//ExprStmt
            if (lhs->exprList.size() != 1) throw runtime_error("one expr required");
            auto* stmt = new ExprStmt;
            stmt->expr = lhs->exprList[0];
            return stmt;
        }
        }
    };
    parseIfStmt = [&](Token&t)->IfStmt* {
        const int outLev = nestLev;
        nestLev = -1;
        eat(KW_if, "expect keyword if");
        auto * node = new IfStmt;
        if (t.type == OP_LBRACE) throw runtime_error("if statement requires a condition");
        auto* tmp = parseSimpleStmt(nullptr, t);
        if (t.type == OP_SEMI) {
            node->init = tmp;
            t = next(f);
            node->cond = parseExpr(t);
        }
        else node->cond = dynamic_cast<ExprStmt*>(tmp)->expr;

        node->ifBlock = parseBlock(t);
        if (t.type == KW_else) {
            t = next(f);
            if (t.type == KW_if)            node->elseBlock = parseIfStmt(t);   
            else if (t.type == OP_LBRACE)   node->elseBlock = parseBlock(t);
            else throw runtime_error("only else-if or else could place here");
        }
        nestLev = outLev;
        return node;
    };
    auto parseSwitchCase = [&](Token&t) {
        SwitchCase* node{};
        if (t.type == KW_case) {
            node = new SwitchCase;
            t = next(f);
            node->exprList = parseExprList(t);
            eat(OP_COLON, "statements in each case requires colon to separate");
            node->stmtList = parseStmtList(t);
        }
        else if (t.type == KW_default) {
            node = new SwitchCase;
            t = next(f);
            eat(OP_COLON, "expect : after default label");
            node->stmtList = parseStmtList(t);
        }
        return node;
    };
    auto parseSwitchStmt = [&](Token&t) {
        const int outLev = nestLev;
        nestLev = -1;
        eat(KW_switch, "expect keyword switch");
        auto * node = new SwitchStmt;
        if (t.type != OP_LBRACE) {
            node->init = parseSimpleStmt(nullptr, t);
            if (t.type == OP_SEMI) t = next(f);
            if (t.type != OP_LBRACE) node->cond = parseSimpleStmt(nullptr, t);
        }
        eat(OP_LBRACE, "expec { after switch header");
        do {
            if (auto*tmp = parseSwitchCase(t); tmp != nullptr) node->caseList.push_back(tmp);
        } while (t.type != OP_RBRACE);
        t = next(f);
        nestLev = outLev;
        return node;
    };
    auto parseSelectCase = [&](Token&t){
        SelectCase* node{};
        if (t.type == KW_case) {
            node = new SelectCase;
            t = next(f);
            auto*tmp = parseSimpleStmt(nullptr, t);
            eat(OP_COLON, "statements in each case requires colon to separate");
            node->stmtList = parseStmtList(t);
        }
        else if (t.type == KW_default) {
            node = new SelectCase;
            t = next(f);
            eat(OP_COLON, "expect : after default label");
            node->stmtList = parseStmtList(t);
        }
        return node;
    };
    auto parseSelectStmt = [&](Token&t) {
        const int outLev = nestLev;
        nestLev = -1;
        eat(KW_select, "expect keyword select");
        eat(OP_LBRACE, "expect { after select keyword");
        auto* node = new SelectStmt;
        do {
            if (auto*tmp = parseSelectCase(t); tmp != nullptr) node->caseList.push_back(tmp);
        } while (t.type != OP_RBRACE);
        t = next(f);
        nestLev = outLev;
        return node;
    };
    auto parseForStmt = [&](Token&t){
        const int outLev = nestLev;
        nestLev = -1;
        eat(KW_for, "expect keyword for");
        auto* node = new ForStmt;
        if (t.type != OP_LBRACE) {
            if (t.type != OP_SEMI) {
                auto*tmp = parseSimpleStmt(nullptr, t);
                switch (t.type) {
                case OP_LBRACE:
                    node->cond = tmp;
                    if (anyone(typeid(*tmp), typeid(SRangeClause), typeid(RangeClause))) nestLev = outLev;
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
            else {  // for ;cond;post{}
                t = next(f);
                node->init = nullptr;
                node->cond = parseExpr(t);
                eat(OP_SEMI, "for syntax are as follows: [init];[cond];[post]{...}");
                if (t.type != OP_LBRACE) node->post = parseSimpleStmt(nullptr, t);
            }
        }
        node->block = parseBlock(t);
        nestLev = outLev;
        return node;
    };
    parseStmt = [&](Token&t)->Stmt* {
        switch (t.type) {
        case KW_type:  	    return parseTypeDecl(t);
        case KW_const:      return parseConstDecl(t);
        case KW_var:        return parseVarDecl(t);
        case KW_fallthrough:t = next(f);  return new FallthroughStmt();
        case KW_go:         t = next(f);  return new GoStmt(parseExpr(t));
        case KW_return:     t = next(f);  return new ReturnStmt(parseExprList(t));
        case KW_break:      t = next(f);  return new BreakStmt(t.type == TK_ID ? t.lexeme : "");
        case KW_continue:   t = next(f);  return new ContinueStmt(t.type == TK_ID ? t.lexeme : "");
        case KW_goto:       t = next(f);  return new GotoStmt(t.lexeme);
        case KW_defer:      t = next(f);  return new DeferStmt(parseExpr(t));
        case KW_if:         return parseIfStmt(t);
        case KW_switch:     return parseSwitchStmt(t);
        case KW_select:     return parseSelectStmt(t);
        case KW_for:        return parseForStmt(t);
        case OP_LBRACE:     return parseBlock(t);
        case OP_SEMI:       return nullptr;
        case OP_ADD:case OP_SUB:case OP_NOT:case OP_XOR:case OP_MUL:case OP_CHAN:
        case LIT_STR:case LIT_INT:case LIT_IMG:case LIT_FLOAT:case LIT_RUNE:
        case KW_func:case KW_struct:case KW_map:case OP_LBRACKET:case TK_ID: case OP_LPAREN:{
            auto* exprList = parseExprList(t);
            if (t.type == OP_COLON) { 
                //it shall a labeled statement(not part of simple stmt so we handle it here)
                t = next(f);
                return new LabeledStmt(
                    dynamic_cast<Name*>(dynamic_cast<PrimaryExpr*>(exprList->exprList[0]->lhs->expr)->expr)->name,
                    parseStmt(t));
            } else return parseSimpleStmt(exprList, t);
        }
        }
        return nullptr;
    };
#pragma endregion
#pragma region Expression
    parseExpr = [&](Token&t)->Expr* {
        Expr* node{};
        if (auto*tmp = parseUnaryExpr(t); tmp != nullptr) {
            node = new  Expr;
            node->lhs = tmp;
            if (anyone(t.type, OP_OR, OP_AND, OP_EQ, OP_NE, OP_LT, OP_LE, OP_XOR, OP_GT, OP_GE, OP_ADD,
                OP_SUB, OP_BITOR, OP_XOR, OP_ANDXOR, OP_MUL, OP_DIV, OP_MOD, OP_LSHIFT, OP_RSHIFT, OP_BITAND)) {
                node->op = t.type;
                t = next(f);
                node->rhs = parseExpr(t);
            }
        }
        return node;
    };
    parseUnaryExpr = [&](Token&t)->UnaryExpr* {
        UnaryExpr* node{};
        if (anyone(t.type, OP_ADD, OP_SUB, OP_NOT, OP_XOR, OP_MUL, OP_BITAND, OP_CHAN)) {
            node = new UnaryExpr;
            node->op = t.type;
            t = next(f);
            node->expr = parseUnaryExpr(t);
        } else if (anyone(t.type, TK_ID, LIT_INT, LIT_FLOAT, LIT_IMG, LIT_RUNE, LIT_STR,
            KW_struct, KW_map, OP_LBRACKET, KW_chan, KW_interface, KW_func, OP_LPAREN)) {
            node = new UnaryExpr;
            node->expr = parsePrimaryExpr(t);
        }
        return node;
    };
    auto parseOperand = [&](Token&t)->Expr* {
        if (t.type == TK_ID) {
            return parseName(false, t);
        } else if (t.type == KW_func) {
            return parseFuncDecl(true, t);
        } else if (t.type == OP_LPAREN) {
            t = next(f); 
            nestLev++; 
            auto* e = parseExpr(t); 
            nestLev--; 
            eat(OP_RPAREN, "expect )"); 
            return e;
        } else if (anyone(t.type, LIT_INT, LIT_FLOAT, LIT_IMG, LIT_RUNE, LIT_STR)) {
            auto*tmp = new BasicLit(t.type, t.lexeme); t = next(f); return tmp;
        } else if (anyone(t.type, KW_struct, KW_map, OP_LBRACKET, KW_chan, KW_interface)) {
            return parseType(t);
        } else return nullptr;

    };
    parsePrimaryExpr = [&](Token&t)->PrimaryExpr* {
        PrimaryExpr*node{};
        if (auto*tmp = parseOperand(t); tmp != nullptr) {
            node = new PrimaryExpr;
            // eliminate left-recursion by infinite loop; these code referenced golang official impl
            // since this work requires somewhat familiarity of golang syntax
            while (true) {
                if (t.type == OP_DOT) {
                    t = next(f);
                    if (t.type == TK_ID) {
                        tmp = new SelectorExpr(tmp, t.lexeme);
                        t = next(f);
                    } else if (t.type == OP_LPAREN) {
                        t = next(f);
                        if (t.type == KW_type) {
                            tmp = new TypeSwitchExpr(tmp);
                            t = next(f);
                        } else {
                            tmp = new TypeAssertExpr(tmp, parseType(t));
                        }
                        eat(OP_RPAREN, "expect )");
                    }
                } else if (t.type == OP_LBRACKET) {
                    nestLev++;
                    t = next(f);
                    Expr* start = nullptr;//Ignore start if next token is :(syntax of operand[:xxx])
                    if (t.type != OP_COLON) {
                        start = parseExpr(t);
                        if (t.type == OP_RBRACKET) {
                            tmp = new IndexExpr(tmp, start);
                            t = next(f);
                            nestLev--;
                            continue;
                        }
                    }
                    auto* e = new SliceExpr;
                    e->operand = tmp;
                    e->begin = start;
                    eat(OP_COLON, "expect : in slice expression");
                    e->end = parseExpr(t);//may nullptr
                    if (t.type == OP_COLON) {
                        t = next(f);
                        e->step = parseExpr(t);
                        eat(OP_RBRACKET, "expect ] at the end of slice expression");
                    } else if (t.type == OP_RBRACKET) {
                        t = next(f);
                    }
                    tmp = e;
                    nestLev--;
                } else if (t.type == OP_LPAREN) {
                    t = next(f);
                    auto* e = new CallExpr;
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
                }else if (t.type == OP_LBRACE) {
                    // only operand has literal value, otherwise, treats it as a block
                    if (anyone(typeid(*tmp), typeid(ArrayType), typeid(SliceType), typeid(StructType), typeid(MapType))
                        || ((anyone(typeid(*tmp), typeid(Name), typeid(SelectorExpr))) && nestLev >= 0)) {
                        // it's somewhat curious since official implementation treats literal type and literal value as separate parts
                        auto* e = new CompositeLit;
                        e->litName = tmp;
                        e->litValue = parseLitValue(t);
                        tmp = e;
                    } else break;
                } else break;
            }
            node->expr = tmp;
        }
        return node;
    };
    auto parseKey = [&](Token&t)->Expr* {
        if (t.type == TK_ID)                                    return parseName(false, t);
        else if (auto*tmp = parseLitValue(t); tmp != nullptr)   return tmp;
        else if (auto*tmp = parseExpr(t); tmp != nullptr)       return tmp;
        return nullptr;
    };
    auto parseKeyedElement = [&](Token&t){
        KeyedElement*node{};
        if (auto*tmp = parseKey(t); tmp != nullptr) {
            node = new KeyedElement;
            node->elem = tmp;
            if (t.type == OP_COLON) {
                node->key = tmp;
                t = next(f);
                if (auto*tmp1 = parseExpr(t); tmp1 != nullptr)          node->elem = tmp1;
                else if (auto*tmp1 = parseLitValue(t); tmp1 != nullptr) node->elem = tmp1;
                else node->elem = nullptr;
            }
        }
        return node;
    };
    parseLitValue = [&](Token&t)->LitValue* {
        LitValue*node{};
        if (t.type == OP_LBRACE) {
            nestLev++;
            node = new LitValue;
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
#pragma endregion
    // parsing startup
    auto * node = new CompilationUnit;
    eat(KW_package, "a go source file must start with package declaration");
    node->package = t.lexeme;
    eat(TK_ID, "name required at the package declaration");
    eat(OP_SEMI, "expect ; at the end of package declaration");
    while (t.type != TK_EOF) {
        switch (t.type) {
        case KW_import: node->importDecl.push_back(parseImportDecl(t));     break;
        case KW_const:  node->constDecl.push_back(parseConstDecl(t));       break;
        case KW_type:   node->typeDecl.push_back(parseTypeDecl(t));         break;
        case KW_var:    node->varDecl.push_back(parseVarDecl(t));           break;
        case KW_func:   node->funcDecl.push_back(parseFuncDecl(false, t));  break;
        case OP_SEMI:   t = next(f);                                        break;
        default:        REPORT_ERR("syntax error","unknown top level declaration"); 
        }
    }
    return node;
}

void emitStub() {}

void runtimeStub() {}

//===---------------------------------------------------------------------------------------===//
// debug auxiliary functions, they are not part of 5 functions
//===---------------------------------------------------------------------------------------===//
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
    //printLex(argv[1]);
    const CompilationUnit* ast = parse(argv[1]);
    fprintf(stdout, "parsing passed\n");
    return 0;
}