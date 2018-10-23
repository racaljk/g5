//===---------------------------------------------------------------------------------------===//
// g5 : golang compiler and runtime in 5 named functions
// Copyright (C) 2018 racaljk<1948638989@qq.com>.
//
// This program is free software: you can redistribute it and/or modify it under the terms of the 
// GNU General Public License as published by the Free Software Foundation, either version 3 of 
// the License, or (at your option) any later version.
//===---------------------------------------------------------------------------------------===//
#include <exception>
#include <iostream>
#include <fstream>
#include <functional>
#include <string>
#include <vector>
#include <tuple>
#include <map>
#define inrange(c,begin,end) (c>=begin && c<=end)
#define LAMBDA_FUN(X) function<X*(Token&)> parse##X;
#define G_ERROR(PRE,STR) \
{cerr<<PRE<<": "<<STR<<" at line "<<line<<", col"<<column<<"\n";\
exit(EXIT_FAILURE);}
#define G_ASSERT(EXPR,PRE,MSG) {if((EXPR)) G_ERROR(PRE,MSG);}
using namespace std;
//===---------------------------------------------------------------------------------------===//
// global data
//===---------------------------------------------------------------------------------------===//
string keywords[] = { "break","default","func","interface","select","case","defer","go","map",
    "struct","chan","else","goto","package","switch","const","fallthrough","if","range","type",
    "continue","for","import","return","var" };
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
#define CTOR1(NAME,FD1)         NAME(decltype(FD1) FD1):FD1(FD1){}
#define CTOR2(NAME,FD1,FD2)     NAME(decltype(FD1) FD1, decltype(FD2) FD2):FD1(FD1),FD2(FD2){}
#define CTOR3(NAME,FD1,FD2,FD3) NAME(decltype(FD1) FD1, decltype(FD2) FD2, decltype(FD3) FD3)\
                                :FD1(FD1),FD2(FD2),FD3(FD3){}
struct Node                { virtual ~Node() = default; };
struct Expr             _N {};
struct Stmt             _N {};
struct BasicExpr        _E { Expr*lhs{}, *rhs{}; TokenType op{}; };
struct IdentList        _E { vector<string> identList; };
struct ExprList         _E { vector<Expr*> exprList; };
struct StmtList         _E { vector<Stmt*> stmtList; };
// Statement
struct BlockStmt        _S { StmtList* stmtList{}; };
struct GoStmt           _S { Expr* expr{}; CTOR1(GoStmt, expr) };
struct ReturnStmt       _S { ExprList* exprList{}; CTOR1(ReturnStmt,exprList) };
struct BreakStmt        _S { string label; CTOR1(BreakStmt, label) };
struct DeferStmt        _S { Expr* expr{}; CTOR1(DeferStmt, expr) };
struct ContinueStmt     _S { string label; CTOR1(ContinueStmt, label) };
struct GotoStmt         _S { string label; CTOR1(GotoStmt, label) };
struct FallthroughStmt  _S {};
struct LabeledStmt      _S { string label; Stmt* stmt{}; CTOR2(LabeledStmt,label,stmt)};
struct IfStmt           _S { Stmt* init{}, *ifBlock{}, *elseBlock{}; Expr* cond{}; };
struct SwitchCase          { ExprList* exprList{}; StmtList* stmtList{}; };
struct SwitchStmt       _S { Stmt* init{}, *cond{}; vector<SwitchCase*> caseList{}; };
struct SelectCase          {StmtList* stmtList{};};
struct SelectStmt       _S {vector<SelectCase*> caseList;};
struct ForStmt          _S { Node* init{}, *cond{}, *post{}; BlockStmt* block{}; };
struct SRangeClause     _S { vector<string> lhs; Expr* rhs{}; CTOR2(SRangeClause, lhs, rhs) };
struct RangeClause      _S { ExprList* lhs{}; TokenType op; Expr* rhs{}; CTOR3(RangeClause,lhs,op,rhs)};
struct ExprStmt         _S { Expr* expr{}; CTOR1(ExprStmt,expr) };
struct SendStmt         _S { Expr* receiver{}, *sender{}; CTOR2(SendStmt, receiver, sender) };
struct IncDecStmt       _S { Expr* expr{}; bool isInc{}; CTOR2(IncDecStmt, expr, isInc) };
struct AssignStmt       _S { ExprList* lhs{}, *rhs{}; TokenType op{}; CTOR3(AssignStmt,lhs,op,rhs) };
struct SAssignStmt      _S { vector<string> lhs{}; ExprList* rhs{}; CTOR2(SAssignStmt,lhs,rhs) };
// Expression
struct SelectorExpr     _E { Expr* operand{}; string selector; CTOR2(SelectorExpr, operand, selector) };
struct TypeSwitchExpr   _E { Expr* operand{}; CTOR1(TypeSwitchExpr, operand) };
struct IndexExpr        _E { Expr* operand{}, *index{}; CTOR2(IndexExpr, operand,index) };
struct TypeAssertExpr   _E { Expr* operand{}, *type{}; CTOR2(TypeAssertExpr, operand, type) };
struct SliceExpr        _E { Expr* operand{}, *begin{}, *end{}, *step{}; };
struct CallExpr         _E { Expr* operand{}, *type{}; ExprList* arguments{}; bool isVariadic{}; };
struct LitValue         _E { vector<tuple<Expr*,Expr*>> keyedElement; };
struct BasicLit         _E { TokenType type{}; string value; CTOR2(BasicLit, type, value) };
struct CompositeLit     _E { Expr* litName{}; LitValue* litValue{}; CTOR2(CompositeLit,litName,litValue) };
struct Name             _E { string name; };
struct ArrayType        _E { Expr* len{}; Expr* elem{}; bool autoLen = false; };
struct StructType       _E { vector<tuple<Expr*, Expr*, string, bool>> fields; };
struct PtrType          _E { Expr* elem{}; CTOR1(PtrType, elem) };
struct ParamDecl           { bool isVariadic = false, hasName = false; Expr* type{}; string name; };
struct Param               { vector<ParamDecl*> paramList; };
struct Signature           { Param* param{}, *resultParam{}; Expr* resultType{}; };
struct FuncType         _E { Signature * signature{}; CTOR1(FuncType,signature) };
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
                    } else G_ERROR("lex error", "expect variadic notation(...)");
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
            else G_ERROR("lex error", "illegal rune");
        } else lexeme += consumePeek(c);

        G_ASSERT(c != '\'', "lexer error", "illegal rune");
        lexeme += consumePeek(c);
        return Token(LIT_RUNE, lexeme);
    }
    // string literal
    if (c == '`') {
        do {
            lexeme += consumePeek(c);
            if (c == '\n') line++;
        } while (f.good() && c != '`');
        G_ASSERT(c != '`', "lexer error", "raw string literal does not have a closed symbol \"`\"");
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
        G_ASSERT(c != '"', "lexer error", "string literal does not have a closed symbol");
        lexeme += consumePeek(c);
        return Token(LIT_STR, lexeme);
    }

    auto match = [&](initializer_list<tuple<pair<char, TokenType>,initializer_list<pair<string_view, TokenType>>,
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
    else G_ERROR("lex error", "illegal token in source file");
}

const auto parse(const string & filename) {
    fstream f(filename, ios::binary | ios::in);
    auto t = next(f);

    auto eat = [&](TokenType tk) {
        G_ASSERT(t.type != tk, "syntax ", "expect " + keywords[tk - 1] + "but got " + keywords[t.type - 1]);
        t = next(f);
    };
    // Simulate EBNF behaviors, see g5/docs/ebnf.md for their explanation if you don't know
    // They are merely used in the case of simple parsing tasks, while keeping traditional control
    // flows for those complicated tasks since callback is not enough clear to read for human logic
    auto option = [&](TokenType specific,auto then) {
        if (t.type == specific) { t = next(f); then(); }};
    auto repetition = [&](TokenType endToken, auto work) {
        do { work(); } while (t.type != endToken); t = next(f); };
    auto alternation = [&](TokenType specific, auto then, auto otherwise) {
        if (t.type == specific) { t = next(f); then(); } else { otherwise(); }}; 

    LAMBDA_FUN(LitValue);LAMBDA_FUN(Stmt); LAMBDA_FUN(IfStmt);
    function<Expr*(Token&)> parseType, parseUnaryExpr, parsePrimaryExpr,parseExpr;

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
        option(TK_ID, [&] {
            node = new  IdentList;
            node->identList.emplace_back(t.lexeme);
            while (t.type == OP_COMMA) {
                t = next(f);
                node->identList.emplace_back(t.lexeme);
                t = next(f);
            }
        });
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
            option(OP_SEMI,[]{});
        }
        return node;
    };
    auto parseBlock = [&](Token&t){
        BlockStmt * node{};
        option(OP_LBRACE, [&] {
            node = new BlockStmt;
            alternation(OP_RBRACE, [&] {}, [&] {node->stmtList = parseStmtList(t); eat(OP_RBRACE); });});
        return node;
    };
#pragma endregion
#pragma region Declaration
    auto parseImportDecl = [&](Token&t) {
        auto node = new ImportDecl;
        eat(KW_import);
        alternation(OP_LPAREN, [&] { repetition(OP_RPAREN,[&] {
            string importName, alias;
            if (anyone(t.type, OP_DOT, TK_ID)) {
                alias = t.lexeme;
                t = next(f);
                importName = t.lexeme;
            } else importName = t.lexeme;
            importName = importName.substr(1, importName.length() - 2);
            node->imports[importName] = alias;
            t = next(f);
            option(OP_SEMI, [] {});
        });}, [&] {
            string importName, alias;
            if (anyone(t.type, OP_DOT, TK_ID)) {
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
        });
        return node;
    };
    auto parseConstDecl = [&](Token&t){
        auto * node = new ConstDecl;
        eat(KW_const);
        alternation(OP_LPAREN, [&] {repetition(OP_RPAREN, [&] {
            node->identList.push_back(parseIdentList(t));
            if (auto*tmp = parseType(t); tmp != nullptr)
                node->type.push_back(tmp);
            else
                node->type.push_back(nullptr);
            alternation(OP_AGN, [&] {node->exprList.push_back(parseExprList(t)); }, [&] {node->exprList.push_back(nullptr); });
            option(OP_SEMI, [] {});
        });}, [&] {
            node->identList.push_back(parseIdentList(t));
            if (auto*tmp = parseType(t); tmp != nullptr)    node->type.push_back(tmp);
            else                                           node->type.push_back(nullptr);
            alternation(OP_AGN, [&] {node->exprList.push_back(parseExprList(t)); }, [&] {node->exprList.push_back(nullptr); });
            if (t.type != OP_SEMI) G_ERROR("syntax error", "expect an explicit semicolon");
        });
        return node;
    };
    auto parseTypeSpec = [&](Token&t) {
        TypeSpec* node{};
        if (t.type == TK_ID) {
            node = new TypeSpec;
            node->ident = t.lexeme;
            t = next(f);
            option(OP_AGN, [] {});
            node->type = parseType(t);
        }
        return node;
    };
    auto parseTypeDecl = [&](Token&t) {
        auto * node = new TypeDecl;
        eat(KW_type);
        alternation(OP_LPAREN, [&] {
            repetition(OP_RPAREN, [&] {node->typeSpec.push_back(parseTypeSpec(t));option(OP_SEMI, [] {}); });
        }, [&] {node->typeSpec.push_back(parseTypeSpec(t)); });
        return node;
    };
    auto parseVarSpec = [&](Token&t){
        VarSpec* node{};
        if (auto*tmp = parseIdentList(t); tmp != nullptr) {
            node = new VarSpec;
            node->identList = tmp;
            alternation(OP_AGN, [&] {node->exprList = parseExprList(t); },
                [&] {node->type = parseType(t); option(OP_AGN, [&] {node->exprList = parseExprList(t); }); });
        }
        return node;
    };
    auto parseVarDecl = [&](Token&t) {
        auto * node = new VarDecl;
        eat(KW_var);
        alternation(OP_LPAREN, [&] {
            repetition(OP_RPAREN, [&] {node->varSpec.push_back(parseVarSpec(t)); option(OP_SEMI, [] {}); });
        }, [&] {node->varSpec.push_back(parseVarSpec(t)); });
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
                option(OP_VARIADIC, [&] {node->isVariadic = true; });
                node->name = dynamic_cast<Name*>(mayIdentOrType)->name;
                node->type = parseType(t);
            } else node->type = mayIdentOrType;
        }
        return node;
    };
    auto parseParam = [&](Token&t){
        Param* node{};
        option(OP_LPAREN, [&] {
            node = new Param;
            repetition(OP_RPAREN, [&] {
                if (auto * tmp = parseParamDecl(t); tmp != nullptr) { node->paramList.push_back(tmp); }
                option(OP_COMMA, [] {}); });
            for (int i = 0, rewriteStart = 0; i < node->paramList.size(); i++) {
                if (dynamic_cast<ParamDecl*>(node->paramList[i])->hasName) {
                    for (int k = rewriteStart; k < i; k++) {
                        string name = dynamic_cast<Name*>(node->paramList[k]->type)->name;
                        node->paramList[k]->type = node->paramList[i]->type;
                        node->paramList[k]->name = name;
                        node->paramList[k]->hasName = true; 
                    }
                    rewriteStart = i + 1;
                }
            }
        });
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
        eat(KW_func);
        if (!anonymous) {
            if (t.type == OP_LPAREN) node->receiver = parseParam(t);
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
        eat(OP_LBRACKET);
        nestLev++;
        alternation(OP_RBRACKET, [&] {
            node = new SliceType;
            nestLev--;
            dynamic_cast<SliceType*>(node)->elem = parseType(t);
        }, [&] {
            node = new ArrayType;
            alternation(OP_VARIADIC, [&] {dynamic_cast<ArrayType*>(node)->autoLen = true; },
                [&] {dynamic_cast<ArrayType*>(node)->len = parseExpr(t); });
            nestLev--;
            t = next(f);
            dynamic_cast<ArrayType*>(node)->elem = parseType(t);
        });
        return node;
    };
    auto parseStructType = [&](Token&t){
        auto * node = new  StructType;
        eat(KW_struct); option(OP_SEMI, [] {}); eat(OP_LBRACE);
        repetition(OP_RBRACE, [&] {
            tuple<Expr*, Expr*, string, bool> field;// <IdentList/Name,Type,Tag,isEmbeded>
            if (auto * tmp = parseIdentList(t); tmp != nullptr) {
                get<0>(field) = tmp;
                get<1>(field) = parseType(t);
                get<3>(field) = false;
            } else {
                option(OP_MUL, [&] {get<3>(field) = true;});
                get<0>(field) = parseName(true, t);
            }
            if (t.type == LIT_STR) get<2>(field) = t.lexeme;
            node->fields.push_back(field);
            option(OP_SEMI, [] {});
        });
        option(OP_SEMI,[]{});
        return node;
    };
    auto parseInterfaceType = [&](Token&t){
        auto * node = new InterfaceType;
        eat(KW_interface);eat(OP_LBRACE);
        while (t.type != OP_RBRACE) {
            if (auto* tmp = parseName(true, t); tmp != nullptr && tmp->name.find('.') == string::npos)
                node->method.emplace_back(tmp, parseSignature(t));
            else node->method.emplace_back(tmp, nullptr);
            option(OP_SEMI,[]{});
        }
        t = next(f);
        return node;
    };
    auto parseMapType = [&](Token&t){
        auto * node = new MapType;
        eat(KW_map);eat(OP_LBRACKET);
        node->type = parseType(t);
        eat(OP_RBRACKET);
        node->elem = parseType(t);
        return node;
    };
    auto parseChanType = [&](Token&t){
        ChanType* node{};
        option(KW_chan, [&] {
            node = new ChanType;
            alternation(OP_CHAN, [&] {node->elem = parseType(t); }, [&] {node->elem = parseType(t); }); });
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
            t = next(f);
            return new SRangeClause{ vector<string>(),parseExpr(t) };
        }
        if (lhs == nullptr) lhs = parseExprList(t);
        switch (t.type) {
        case OP_CHAN:           {t = next(f); return new SendStmt{ lhs->exprList[0],parseExpr(t) }; }
        case OP_INC:case OP_DEC:{auto tmp = t.type; t = next(f); return new IncDecStmt{lhs->exprList[0],tmp==OP_INC}; }
        case OP_SHORTAGN: {
            vector<string> identList;
            for (auto* e : lhs->exprList) {
                string identName = dynamic_cast<Name*>(dynamic_cast<BasicExpr*>(e)->lhs)->name;
                identList.push_back(identName);
            }
            t = next(f);
            Stmt* stmt{};
            alternation(KW_range,[&] {stmt = new SRangeClause{ move(identList), parseExpr(t) }; },
                [&] {stmt = new SAssignStmt{ move(identList) ,parseExprList(t) }; });
            return stmt;
        }
        case OP_ADDAGN:case OP_SUBAGN:case OP_ORAGN:case OP_XORAGN:case OP_MULAGN:case OP_DIVAGN:
        case OP_MODAGN:case OP_LSFTAGN:case OP_RSFTAGN:case OP_ANDAGN:case OP_ANDXORAGN:case OP_AGN: {
            if (lhs->exprList.empty()) throw runtime_error("one expr required");
            auto op = t.type;
            t = next(f);
            Stmt* stmt{};
            alternation(KW_range,[&] {stmt = new RangeClause{ lhs,op,parseExpr(t) }; },
                [&] {stmt = new AssignStmt{ lhs,op,parseExprList(t)}; });
            return stmt;
        }
        default: {return new ExprStmt{ lhs->exprList[0] }; }//ExprStmt
        }
    };
    parseIfStmt = [&](Token&t)->IfStmt* {
        const int outLev = nestLev;
        nestLev = -1;
        auto * node = new IfStmt;
        if (t.type == OP_LBRACE) throw runtime_error("if statement requires a condition");
        auto* tmp = parseSimpleStmt(nullptr, t);
        alternation(OP_SEMI, 
            [&] {node->init = tmp; node->cond = parseExpr(t); },
            [&] {node->cond = dynamic_cast<ExprStmt*>(tmp)->expr; });
        nestLev = outLev;
        
        node->ifBlock = parseBlock(t);
        option(KW_else, [&] {
            if (t.type == KW_if) {t = next(f); node->elseBlock = parseIfStmt(t);}
            else if (t.type == OP_LBRACE)   node->elseBlock = parseBlock(t);
            else G_ERROR("syntax error", "only else-if or else could place here");
        });
        return node;
    };
    auto parseSwitchCase = [&](Token&t) {
        SwitchCase* node{};
        if (t.type == KW_case) {
            node = new SwitchCase;
            t = next(f);
            node->exprList = parseExprList(t);
            eat(OP_COLON);
            node->stmtList = parseStmtList(t);
        } else if (t.type == KW_default) {
            node = new SwitchCase;
            t = next(f);
            eat(OP_COLON);
            node->stmtList = parseStmtList(t);
        }
        return node;
    };
    auto parseSwitchStmt = [&](Token&t) {
        const int outLev = nestLev;
        nestLev = -1;
        auto * node = new SwitchStmt;
        if (t.type != OP_LBRACE) {
            node->init = parseSimpleStmt(nullptr, t);
            option(OP_SEMI, [] {});
            if (t.type != OP_LBRACE) node->cond = parseSimpleStmt(nullptr, t);
        }
        nestLev = outLev;

        eat(OP_LBRACE);
        repetition(OP_RBRACE, 
            [&] {if (auto*tmp = parseSwitchCase(t); tmp != nullptr) node->caseList.push_back(tmp); });
        return node;
    };
    auto parseSelectCase = [&](Token&t){
        SelectCase* node{};
        if (t.type == KW_case) {
            node = new SelectCase;
            t = next(f);
            auto*tmp = parseSimpleStmt(nullptr, t);
            eat(OP_COLON);
            node->stmtList = parseStmtList(t);
        } else if (t.type == KW_default) {
            node = new SelectCase;
            t = next(f);
            eat(OP_COLON);
            node->stmtList = parseStmtList(t);
        }
        return node;
    };
    auto parseSelectStmt = [&](Token&t) {
        eat(OP_LBRACE);
        auto* node = new SelectStmt;
        repetition(OP_RBRACE,[&] {if (auto*tmp = parseSelectCase(t); tmp != nullptr) node->caseList.push_back(tmp); });
        return node;
    };
    auto parseForStmt = [&](Token&t){
        const int outLev = nestLev;
        nestLev = -1;
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
                    eat(OP_SEMI);
                    node->cond = parseExpr(t);
                    eat(OP_SEMI);
                    if (t.type != OP_LBRACE)node->post = parseSimpleStmt(nullptr, t);
                    break;
                default:G_ERROR("syntax error", "expect {/;/range/:=/=");
                }
            } else {  // for ;cond;post{}
                t = next(f);
                node->cond = parseExpr(t);
                eat(OP_SEMI);
                if (t.type != OP_LBRACE) node->post = parseSimpleStmt(nullptr, t);
            }
        }
        nestLev = outLev;
        node->block = parseBlock(t);
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
        case KW_if:         t = next(f);  return parseIfStmt(t);
        case KW_switch:     t = next(f);  return parseSwitchStmt(t);
        case KW_select:     t = next(f);  return parseSelectStmt(t);
        case KW_for:        t = next(f);  return parseForStmt(t);
        case OP_LBRACE:     return parseBlock(t);
        case OP_SEMI:       return nullptr;
        case OP_ADD:case OP_SUB:case OP_NOT:case OP_XOR:case OP_MUL:case OP_CHAN:
        case LIT_STR:case LIT_INT:case LIT_IMG:case LIT_FLOAT:case LIT_RUNE:
        case KW_func:case KW_struct:case KW_map:case OP_LBRACKET:case TK_ID: case OP_LPAREN:{
            // It shall a labeled statement(not part of simple stmt so we handle it here)
            auto* exprList = parseExprList(t);
            Stmt*result{};
            alternation(OP_COLON, [&] { result = new LabeledStmt(dynamic_cast<Name*>(
                dynamic_cast<BasicExpr*>(exprList->exprList[0])->lhs)->name,parseStmt(t));
            }, [&] {result = parseSimpleStmt(exprList, t); });
            return result;
        }
        }
        return nullptr;
    };
#pragma endregion
#pragma region Expression
    parseExpr = [&](Token&t)->Expr* {
        BasicExpr* node{};
        if (auto*tmp = parseUnaryExpr(t); tmp != nullptr) {
            node = new  BasicExpr;
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
    parseUnaryExpr = [&](Token&t)->Expr* {
        if (anyone(t.type, OP_ADD, OP_SUB, OP_NOT, OP_XOR, OP_MUL, OP_BITAND, OP_CHAN)) {
            auto* node = new BasicExpr;
            node->op = t.type;
            t = next(f);
            node->lhs = parseUnaryExpr(t);
        } else if (anyone(t.type, TK_ID, LIT_INT, LIT_FLOAT, LIT_IMG, LIT_RUNE, LIT_STR,
            KW_struct, KW_map, OP_LBRACKET, KW_chan, KW_interface, KW_func, OP_LPAREN)) {
            return parsePrimaryExpr(t);
        } else return nullptr;
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
            eat(OP_RPAREN); 
            return e;
        } else if (anyone(t.type, LIT_INT, LIT_FLOAT, LIT_IMG, LIT_RUNE, LIT_STR)) {
            auto*tmp = new BasicLit(t.type, t.lexeme); t = next(f); return tmp;
        } else if (anyone(t.type, KW_struct, KW_map, OP_LBRACKET, KW_chan, KW_interface)) {
            return parseType(t);
        } else return nullptr;
    };
    parsePrimaryExpr = [&](Token&t)->Expr* {
        if (auto*tmp = parseOperand(t); tmp != nullptr) {
            while (true) {
                if (t.type == OP_DOT) {
                    t = next(f);
                    if (t.type == TK_ID) {
                        tmp = new SelectorExpr(tmp, t.lexeme);
                        t = next(f);
                    } else if (t.type == OP_LPAREN) {
                        t = next(f);
                        alternation(KW_type, [&] { tmp = new TypeSwitchExpr(tmp); },
                            [&] {tmp = new TypeAssertExpr(tmp, parseType(t)); });
                        eat(OP_RPAREN);
                    } else G_ERROR("syntax error", "expec identifier or (");
                } else if (t.type == OP_LBRACKET) {
                    nestLev++;
                    t = next(f);
                    Expr* start{};//Ignore start if next token is :(syntax of operand[:xxx])
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
                    eat(OP_COLON);
                    e->end = parseExpr(t);//may nullptr
                    if (t.type == OP_COLON) {
                        t = next(f);
                        e->step = parseExpr(t);
                        eat(OP_RBRACKET);
                    }
                    else if (t.type == OP_RBRACKET) t = next(f);
                    else G_ERROR("syntax error", "expec : or ]");
                    tmp = e;
                    nestLev--;
                } else if (t.type == OP_LPAREN) {
                    t = next(f);
                    auto* e = new CallExpr;
                    e->operand = tmp;
                    nestLev++;
                    if (auto*tmp1 = parseExprList(t); tmp1 != nullptr) e->arguments = tmp1;
                    option(OP_VARIADIC, [&] {e->isVariadic = true; });
                    nestLev--;
                    eat(OP_RPAREN);
                    tmp = e;
                }else if (t.type == OP_LBRACE) {
                    // Only operand has literal value, otherwise, treats it as a block
                    // It's somewhat curious since official implementation treats literal type and literal value as separate parts
                    if (anyone(typeid(*tmp), typeid(ArrayType), typeid(SliceType), typeid(StructType), typeid(MapType))
                        || ((anyone(typeid(*tmp), typeid(Name), typeid(SelectorExpr))) && nestLev >= 0)) {
                        tmp = new CompositeLit{ tmp,parseLitValue(t) };
                    } else break;
                } else break;
            }
            return tmp;
        }
        return nullptr;
    };
    auto parseKeyedElement = [&](Token&t){
        Expr* elem{}, *key{};
        elem = (t.type == OP_LBRACE) ? parseLitValue(t) : parseExpr(t);
        option(OP_COLON, 
            [&] {key = elem; elem = (t.type == OP_LBRACE) ? parseLitValue(t) : parseExpr(t); });
        return make_tuple(key,elem);
    };
    parseLitValue = [&](Token&t)->LitValue* {
        LitValue*node{};
        if (t.type == OP_LBRACE) {
            nestLev++;
            node = new LitValue;
            repetition(OP_RBRACE, [&] {
                t = next(f);
                if (t.type == OP_RBRACE) return; // it's necessary since both {a,b} or {a,b,} are legal form
                node->keyedElement.push_back(parseKeyedElement(t));
            });
            nestLev--;
        }
        return node;
    };
#pragma endregion
    // parsing startup
    auto * node = new CompilationUnit;
    eat(KW_package);
    node->package = t.lexeme;
    eat(TK_ID);eat(OP_SEMI);
    while (t.type != TK_EOF) {
        switch (t.type) {
        case KW_import: node->importDecl.push_back(parseImportDecl(t));     break;
        case KW_const:  node->constDecl.push_back(parseConstDecl(t));       break;
        case KW_type:   node->typeDecl.push_back(parseTypeDecl(t));         break;
        case KW_var:    node->varDecl.push_back(parseVarDecl(t));           break;
        case KW_func:   node->funcDecl.push_back(parseFuncDecl(false, t));  break;
        case OP_SEMI:   t = next(f);                                        break;
        default:        G_ERROR("syntax error","unknown top level declaration"); 
        }
    }
    return node;
}

//===---------------------------------------------------------------------------------------===//
// debug auxiliary functions, they are not part of 5 functions
//===---------------------------------------------------------------------------------------===//
void printLex(const string & filename) {
    fstream f(filename, ios::binary | ios::in);
    while (lastToken != TK_EOF) {
        auto[token, lexeme] = next(f);
        cout << "<" << token << "," << lexeme << "," << line << "," << column << ">\n";
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2 || argv[1] == nullptr) G_ERROR("fatal error", "specify your go source file\n");
    //printLex(argv[1]);
    const CompilationUnit* ast = parse(argv[1]);
    cout << "parsing passed\n";
    return 0;
}