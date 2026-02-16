#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cctype>
#include <map>
#include <cstdlib>
#include <sstream>
#include <algorithm>
#include <stack>
#include <stdexcept>
#include <cstring>
#include <sys/stat.h>
#include <ctime>
#include <chrono>
#include <thread>
#include <memory>
#include <random>
#include <numeric>

#define WL_VERSION "Wei- Aurora"
#define WL_RELEASE_DATE "2026-02-15"
int WL_YN_INTERSTELLAR = 1;  

void error(const std::string& file, int line, int col, const std::string& msg) {
    std::cerr << "\033[1;31m执行错误 " << file << " 时遇到问题\033[0m" << std::endl;
    std::cerr << "\033[1;31m" << file << ":" << line << ":" << col
              << ": \033[1;35m错误:\033[0m\033[1;31m " << msg << "\033[0m" << std::endl;
    exit(1);
}

void warning(const std::string& file, int line, int col, const std::string& msg) {
    std::cerr << "\033[1;33m" << file << ":" << line << ":" << col
              << ": \033[1;36m警告:\033[0m\033[1;33m " << msg << "\033[0m" << std::endl;
}

enum class TokenType {
    CREATE_INT, CREATE_DOUBLE, CREATE_OMNI, CREATE_STRING, CREATE_ARR,
    OUTPUT_REDIRECT, INPUT_REDIRECT,
    STRING, NUMBER, IDENTIFIER, ASSIGN, CREATE, FUNCTION,
    LPAREN, RPAREN, LBRACE, RBRACE, LBRACKET, RBRACKET,
    SEMICOLON, PLUS, MINUS, GT, LT, GTE, LTE, EQ, NEQ,
    IF, WHILE, OUTLB, OUTPUT_CONNECT, LOOP, FOR,
    PLUS_EQUALS, MINUS_EQUALS, STAR_EQUALS, SLASH_EQUALS,
    PLUS_PLUS, MINUS_MINUS, EOF_TOKEN,
    MULTIPLY, DIVIDE, COMMA, FINISH, QUESTION, COLON, DOT,
    UTILIZE, FALID, VALID, VOID_TYPE, EXCLAMATION,
    ACCEPT, PIPE, DTIME_TIC, DTIME_TOC, DTIME_FUNC,
    RANDOM,       
    CREATE_SORT,     
    CREATE_SUM,       
    CREATE_SQRT,       
    TILDE,      
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int col;
    Token(TokenType t, const std::string& l, int ln, int cl)
        : type(t), lexeme(l), line(ln), col(cl) {}
};

enum class VarType {
    INT, DOUBLE, OMNI, STRING, ARRAY,
};

struct Variable {
    VarType type;
    std::string value;
    std::vector<Variable> arrayElements;
    std::vector<size_t> dims;
    std::vector<Variable> flatData;

    Variable(VarType t = VarType::DOUBLE, const std::string& v = "0")
        : type(t), value(v) {}

    bool isNumeric() const {
        return type == VarType::INT || type == VarType::DOUBLE || type == VarType::OMNI;
    }
 
    bool isArray() const {
        return type == VarType::ARRAY;
    }

    double getNumericValue() const {
        try {
            return std::stod(value);
        } catch (...) {
            return 0.0;
        }
    }

    void setDims(const std::vector<size_t>& dimensions) {
        dims = dimensions;
        size_t total = 1;
        for (size_t d : dims) total *= d;
        flatData.resize(total, Variable(VarType::OMNI, "0"));
        if (total > 0) value = flatData[0].value;
        arrayElements.clear();
    }

    size_t flattenIndex(const std::vector<size_t>& indices) const {
        if (indices.size() != dims.size()) {
            throw std::runtime_error("维度不匹配");
        }
        size_t idx = 0, mul = 1;
        for (int i = dims.size() - 1; i >= 0; i--) {
            if (indices[i] >= dims[i]) {
                throw std::runtime_error("索引越界");
            }
            idx += indices[i] * mul;
            mul *= dims[i];
        }
        return idx;
    }

    Variable& getElement(const std::vector<size_t>& indices) {
        return flatData[flattenIndex(indices)];
    }

    const Variable& getElement(const std::vector<size_t>& indices) const {
        return flatData[flattenIndex(indices)];
    }

    Variable& getArrayElement(size_t index) {
        if (!dims.empty()) {
            return getElement({index});
        }
        return arrayElements[index];
    }

    
    size_t getArraySize() const {
        if (!dims.empty()) {
            return dims[0];
        }
        return arrayElements.size();
    }

    
    std::string arrayToString() const {
        if (!dims.empty()) {
            size_t flatIdx = 0;
            return multidimensionalToString(0, flatIdx);
        }
        if (arrayElements.empty()) return "[]";
        std::string r = "[";
        for (size_t i = 0; i < arrayElements.size(); ++i) {
            if (i > 0) r += ", ";
            r += arrayElements[i].value;
        }
        r += "]";
        return r;
    }


    
    void sort() {
        if (!isArray()) {
            throw std::runtime_error("sort()只能用于数组");
        }
        if (flatData.empty()) return;
        
        for (const auto& elem : flatData) {
            if (!elem.isNumeric()) {
                throw std::runtime_error("sort()只能用于数值数组");
            }
        }
        std::sort(flatData.begin(), flatData.end(), 
                  [](const Variable& a, const Variable& b) {
                      return a.getNumericValue() < b.getNumericValue();
                  });
    }

    
    double sum() const {
        if (!isArray()) {
            throw std::runtime_error("sum()只能用于数组");
        }
        if (flatData.empty()) return 0.0;
        return std::accumulate(flatData.begin(), flatData.end(), 0.0,
            [](double acc, const Variable& v) { 
                return acc + v.getNumericValue(); 
            });
    }

    
    int find(const std::string& substr) const {
        if (type != VarType::STRING) {
            throw std::runtime_error("find()只能用于字符串");
        }
        size_t pos = value.find(substr);
        return (pos == std::string::npos) ? -1 : static_cast<int>(pos);
    }
private:
    
    std::string multidimensionalToString(size_t dimIdx, size_t& flatIdx) const {
        if (dimIdx == dims.size() - 1) {
            std::string r = "[";
            for (size_t i = 0; i < dims[dimIdx]; ++i) {
                if (i > 0) r += ", ";
                r += flatData[flatIdx++].value;
            }
            r += "]";
            return r;
        }
        std::string r = "[";
        for (size_t i = 0; i < dims[dimIdx]; ++i) {
            if (i > 0) r += ", ";
            r += multidimensionalToString(dimIdx + 1, flatIdx);
        }
        r += "]";
        return r;
    }
};


struct Function {
    std::string name;
    std::vector<Token> body;
    std::vector<std::string> paramNames;
    bool hasReturnValue;

    Function(const std::string& n = "", const std::vector<Token>& b = {}, 
             bool ret = false) 
        : name(n), body(b), hasReturnValue(ret) {}
};


struct TimerState {
    std::chrono::high_resolution_clock::time_point startTime;
    bool isRunning;
    double lastElapsedMs;

    TimerState() : isRunning(false), lastElapsedMs(0.0) {}
};


struct Scanner {
    std::string src;
    std::string filename;
    size_t pos = 0;
    int line = 1;
    int col = 1;
    bool in_output_redirect = false;
    bool in_create = false;

    Scanner(const std::string& s, const std::string& fname)
        : src(s), filename(fname) {}

    
    bool match(const std::string& s) {
        if (pos + s.length() > src.length()) return false;
        return src.substr(pos, s.length()) == s;
    }

    
    void advance() {
        if (pos < src.size()) {
            if (src[pos] == '\n') {
                line++;
                col = 1;
            } else {
                col++;
            }
            pos++;
        }
    }

    
    void skipWhitespace() {
        while (pos < src.size()) {
            char c = src[pos];
            if (c == ' ' || c == '\t' || c == '\r') {
                col++;
                pos++;
            } else if (c == '\n') {
                line++;
                col = 1;
                pos++;
            } else {
                break;
            }
        }
    }

    
    std::vector<Token> scan() {
        std::vector<Token> tokens;
        in_output_redirect = false;
        in_create = false;

        while (pos < src.size()) {
            skipWhitespace();
            if (pos >= src.size()) break;

            char c = src[pos];

            if (match("dtime_tic")) {
                tokens.emplace_back(TokenType::DTIME_TIC, "dtime_tic", line, col);
                pos += 9;
                col += 9;
                continue;
            }

            if (match("dtime_toc")) {
                tokens.emplace_back(TokenType::DTIME_TOC, "dtime_toc", line, col);
                pos += 9;
                col += 9;
                continue;
            }

            if (match("dtime()")) {
                tokens.emplace_back(TokenType::DTIME_FUNC, "dtime()", line, col);
                pos += 7;
                col += 7;
                continue;
            }

            
            if (match("random")) {
                tokens.emplace_back(TokenType::RANDOM, "random", line, col);
                pos += 6;
                col += 6;
                continue;
            }

            if (c == '!') {
                advance();
                if (match("utilize")) {
                    tokens.emplace_back(TokenType::UTILIZE, "!utilize", line, col - 1);
                    pos += 7;
                    col += 7;
                } else {
                    tokens.emplace_back(TokenType::EXCLAMATION, "!", line, col - 1);
                }
                continue;
            }

            if (c == '"') {
                advance();
                size_t start = pos;
                while (pos < src.size() && src[pos] != '"') {
                    if (src[pos] == '\n') {
                        error(filename, line, col, "字符串字面量未闭合");
                    }
                    advance();
                }
                if (pos >= src.size()) {
                    error(filename, line, col, "未找到闭合的字符串字面量");
                }
                std::string s = src.substr(start, pos - start);
                advance();
                tokens.emplace_back(TokenType::STRING, s, line, col - (pos - start) - 1);
                continue;
            }

            if (match("accept")) {
                tokens.emplace_back(TokenType::ACCEPT, "accept", line, col);
                pos += 6;
                col += 6;
                while (pos < src.size() && isspace(src[pos])) {
                    advance();
                }
                if (pos < src.size() && src[pos] == '>') {
                    advance();
                }
                continue;
            }

            if (match("create")) {
                pos += 6;
                col += 6;
                if (pos < src.size() && src[pos] == '.') {
                    advance();
                    if (match("int")) {
                        tokens.emplace_back(TokenType::CREATE_INT, "create.int", line, col - 7);
                        pos += 3;
                        col += 3;
                        in_create = true;
                        continue;
                    }
                    if (match("double")) {
                        tokens.emplace_back(TokenType::CREATE_DOUBLE, "create.double", line, col - 7);
                        pos += 6;
                        col += 6;
                        in_create = true;
                        continue;
                    }
                    if (match("omni")) {
                        tokens.emplace_back(TokenType::CREATE_OMNI, "create.omni", line, col - 7);
                        pos += 4;
                        col += 4;
                        in_create = true;
                        continue;
                    }
                    if (match("string")) {
                        tokens.emplace_back(TokenType::CREATE_STRING, "create.string", line, col - 7);
                        pos += 6;
                        col += 6;
                        in_create = true;
                        continue;
                    }
                    if (match("arr")) {
                        tokens.emplace_back(TokenType::CREATE_ARR, "create.arr", line, col - 7);
                        pos += 3;
                        col += 3;
                        in_create = true;
                        continue;

                    if (match("sort")) {
                        tokens.emplace_back(TokenType::CREATE_SORT, "create.sort", line, col - 7);
                        pos += 4;
                        col += 4;
                        in_create = true;
                        continue;
                    }
                    if (match("sum")) {
                        tokens.emplace_back(TokenType::CREATE_SUM, "create.sum", line, col - 7);
                        pos += 3;
                        col += 3;
                        in_create = true;
                        continue;
                    }
                    if (match("sqrt")) {
                        tokens.emplace_back(TokenType::CREATE_SQRT, "create.sqrt", line, col - 7);
                        pos += 4;
                        col += 4;
                        in_create = true;
                        continue;
                    }

                    }
                }
                tokens.emplace_back(TokenType::CREATE, "create", line, col - 6);
                continue;
            }

            if (match("output")) {
                tokens.emplace_back(TokenType::OUTPUT_REDIRECT, "output", line, col);
                pos += 6;
                col += 6;
                while (pos < src.size() && isspace(src[pos])) {
                    advance();
                }
                if (pos < src.size() && src[pos] == '>') {
                    advance();
                    in_output_redirect = true;
                }
                continue;
            }

            if (match("input")) {
                pos += 5;
                col += 5;
                while (pos < src.size() && isspace(src[pos])) {
                    advance();
                }
                if (pos < src.size() && src[pos] == '>') {
                    tokens.emplace_back(TokenType::INPUT_REDIRECT, "input>", line, col - 5);
                    advance();
                } else {
                    tokens.emplace_back(TokenType::IDENTIFIER, "input", line, col - 5);
                }
                continue;
            }

            if (match("function")) {
                tokens.emplace_back(TokenType::FUNCTION, "function", line, col);
                pos += 8;
                col += 8;
                continue;
            }

            if (in_output_redirect && c == '>') {
                tokens.emplace_back(TokenType::OUTPUT_CONNECT, ">", line, col);
                advance();
                continue;
            }

            if (match("outlb")) {
                tokens.emplace_back(TokenType::OUTLB, "outlb", line, col);
                pos += 5;
                col += 5;
                continue;
            }

            if (match("for")) {
                tokens.emplace_back(TokenType::FOR, "for", line, col);
                pos += 3;
                col += 3;
                continue;
            }

            if (match("while")) {
                tokens.emplace_back(TokenType::WHILE, "while", line, col);
                pos += 5;
                col += 5;
                continue;
            }

            if (match("if")) {
                tokens.emplace_back(TokenType::IF, "if", line, col);
                pos += 2;
                col += 2;
                continue;
            }

            if (match("loop")) {
                tokens.emplace_back(TokenType::LOOP, "loop", line, col);
                pos += 4;
                col += 4;
                continue;
            }

            if (match("finish")) {
                tokens.emplace_back(TokenType::FINISH, "finish", line, col);
                pos += 6;
                col += 6;
                continue;
            }

            if (isdigit(c)) {
                size_t start = pos;
                while (pos < src.size() && isdigit(src[pos])) {
                    advance();
                }
                if (pos < src.size() && src[pos] == '.') {
                    advance();
                    while (pos < src.size() && isdigit(src[pos])) {
                        advance();
                    }
                }
                std::string num = src.substr(start, pos - start);
                tokens.emplace_back(TokenType::NUMBER, num, line, col - (pos - start));
                continue;
            }

            if (isalpha(c)) {
                size_t start = pos;
                while (pos < src.size() && (isalnum(src[pos]) || src[pos] == '_')) {
                    advance();
                }
                std::string word = src.substr(start, pos - start);
                if (word == "if") {
                    tokens.emplace_back(TokenType::IF, word, line, col - (pos - start));
                    continue;
                }
                if (word == "while") {
                    tokens.emplace_back(TokenType::WHILE, word, line, col - (pos - start));
                    continue;
                }
                if (word == "for") {
                    tokens.emplace_back(TokenType::FOR, word, line, col - (pos - start));
                    continue;
                }
                if (word == "loop") {
                    tokens.emplace_back(TokenType::LOOP, word, line, col - (pos - start));
                    continue;
                }
                if (word == "outlb") {
                    tokens.emplace_back(TokenType::OUTLB, word, line, col - (pos - start));
                    continue;
                }
                if (word == "finish") {
                    tokens.emplace_back(TokenType::FINISH, word, line, col - (pos - start));
                    continue;
                }
                if (word == "function") {
                    tokens.emplace_back(TokenType::FUNCTION, word, line, col - (pos - start));
                    continue;
                }
                if (word == "accept") {
                    tokens.emplace_back(TokenType::ACCEPT, word, line, col - (pos - start));
                    continue;
                }
                if (word == "dtime_tic") {
                    tokens.emplace_back(TokenType::DTIME_TIC, word, line, col - (pos - start));
                    continue;
                }
                if (word == "dtime_toc") {
                    tokens.emplace_back(TokenType::DTIME_TOC, word, line, col - (pos - start));
                    continue;
                }
                if (word == "random") {
                    tokens.emplace_back(TokenType::RANDOM, word, line, col - (pos - start));
                    continue;
                }
                tokens.emplace_back(TokenType::IDENTIFIER, word, line, col - (pos - start));
                continue;
            }

            if (c == '.') {
                advance();
                if (match("falid")) {
                    tokens.emplace_back(TokenType::FALID, ".falid", line, col - 1);
                    pos += 5;
                    col += 5;
                } else if (match("valid")) {
                    tokens.emplace_back(TokenType::VALID, ".valid", line, col - 1);
                    pos += 5;
                    col += 5;
                } else if (match("void")) {
                    tokens.emplace_back(TokenType::VOID_TYPE, ".void", line, col - 1);
                    pos += 4;
                    col += 4;
                } else {
                    tokens.emplace_back(TokenType::DOT, ".", line, col - 1);
                }
                continue;
            }

            if (c == '|') {
                tokens.emplace_back(TokenType::PIPE, "|", line, col);
                advance();
                continue;
            }

            if (c == '=') {
                advance();
                if (pos < src.size() && src[pos] == '=') {
                    tokens.emplace_back(TokenType::EQ, "==", line, col - 1);
                    advance();
                } else {
                    tokens.emplace_back(TokenType::ASSIGN, "=", line, col);
                }
                continue;
            }

            if (c == '!') {
                advance();
                if (pos < src.size() && src[pos] == '=') {
                    tokens.emplace_back(TokenType::NEQ, "!=", line, col - 1);
                    advance();
                } else {
                    error(filename, line, col, "不支持的字符 \'" + std::string(1, c) + "\'");
                }
                continue;
            }

            if (c == '>') {
                advance();
                if (pos < src.size() && src[pos] == '=') {
                    tokens.emplace_back(TokenType::GTE, ">=", line, col - 1);
                    advance();
                } else {
                    tokens.emplace_back(TokenType::GT, ">", line, col - 1);
                }
                continue;
            }

            if (c == '<') {
                advance();
                if (pos < src.size() && src[pos] == '=') {
                    tokens.emplace_back(TokenType::LTE, "<=", line, col - 1);
                    advance();
                } else {
                    tokens.emplace_back(TokenType::LT, "<", line, col - 1);
                }
                continue;
            }

            if (c == '+') {
                advance();
                if (pos < src.size() && src[pos] == '=') {
                    tokens.emplace_back(TokenType::PLUS_EQUALS, "+=", line, col - 1);
                    advance();
                } else if (pos < src.size() && src[pos] == '+') {
                    tokens.emplace_back(TokenType::PLUS_PLUS, "++", line, col - 1);
                    advance();
                } else {
                    tokens.emplace_back(TokenType::PLUS, "+", line, col);
                }
                continue;
            }

            if (c == '-') {
                advance();
                if (pos < src.size() && src[pos] == '=') {
                    tokens.emplace_back(TokenType::MINUS_EQUALS, "-=", line, col - 1);
                    advance();
                } else if (pos < src.size() && src[pos] == '-') {
                    tokens.emplace_back(TokenType::MINUS_MINUS, "--", line, col - 1);
                    advance();
                } else {
                    tokens.emplace_back(TokenType::MINUS, "-", line, col);
                }
                continue;
            }

            if (c == '*') {
                advance();
                if (pos < src.size() && src[pos] == '=') {
                    tokens.emplace_back(TokenType::STAR_EQUALS, "*=", line, col - 1);
                    advance();
                } else {
                    tokens.emplace_back(TokenType::MULTIPLY, "*", line, col);
                }
                continue;
            }

            if (c == '/') {
                advance();
                if (pos < src.size() && src[pos] == '=') {
                    tokens.emplace_back(TokenType::SLASH_EQUALS, "/=", line, col - 1);
                    advance();
                } else {
                    tokens.emplace_back(TokenType::DIVIDE, "/", line, col);
                }
                continue;
            }

            if (c == '[') {
                tokens.emplace_back(TokenType::LBRACKET, "[", line, col);
                advance();
                continue;
            }
            if (c == ']') {
                tokens.emplace_back(TokenType::RBRACKET, "]", line, col);
                advance();
                continue;
            }

            
            if (c == '~') {
                tokens.emplace_back(TokenType::TILDE, "~", line, col);
                advance();
                continue;
            }

            switch (c) {
            case '(': tokens.emplace_back(TokenType::LPAREN, "(", line, col); break;
            case ')': tokens.emplace_back(TokenType::RPAREN, ")", line, col); break;
            case '{': tokens.emplace_back(TokenType::LBRACE, "{", line, col); break;
            case '}': tokens.emplace_back(TokenType::RBRACE, "}", line, col); break;
            case ';':
                tokens.emplace_back(TokenType::SEMICOLON, ";", line, col);
                advance();
                if (in_output_redirect) in_output_redirect = false;
                if (in_create) in_create = false;
                continue;
            case ',':
                tokens.emplace_back(TokenType::COMMA, ",", line, col);
                advance();
                continue;
            case '#':
                while (pos < src.size() && src[pos] != '\n') advance();
                continue;
            case '?':
                tokens.emplace_back(TokenType::QUESTION, "?", line, col);
                advance();
                continue;
            case ':':
                tokens.emplace_back(TokenType::COLON, ":", line, col);
                advance();
                continue;
            default:
                if (!isspace(c)) {
                    error(filename, line, col, "不支持的字符 \'" + std::string(1, c) + "\'");
                }
            }
            advance();
        }

        tokens.emplace_back(TokenType::EOF_TOKEN, "", line, col);
        return tokens;
    }
};


struct Interpreter {
    std::map<std::string, Variable> vars;
    std::map<std::string, Function> functions;
    std::string filename;
    std::stack<std::map<std::string, Variable>> callStack;
    TimerState timer;
    double lastTocTime;
    bool hasTocExecuted;
    std::mt19937 rng;  

    Interpreter(const std::string& fname) 
        : filename(fname), lastTocTime(0.0), hasTocExecuted(false),
          rng(std::random_device{}()) {}

    
    void setVar(const std::string& name, VarType type, const std::string& value) {
        vars[name] = Variable(type, value);
    }

    
    void setArrayVar(const std::string& name, const std::vector<Variable>& elements) {
        Variable arr(VarType::ARRAY);
        arr.arrayElements = elements;
        if (!elements.empty()) {
            arr.value = elements[0].value;
        }
        vars[name] = arr;
    }

    
    void setArrayElement(const std::string& name, size_t index, const Variable& value) {
        auto it = vars.find(name);
        if (it == vars.end()) {
            error(filename, 0, 0, "未声明的数组 \'" + name + "\'");
        }
        if (!it->second.isArray()) {
            error(filename, 0, 0, "变量 \'" + name + "\' 不是数组类型");
        }
        if (!it->second.dims.empty()) {
            it->second.getElement({index}) = value;
            if (index == 0) {
                it->second.value = value.value;
            }
        } else {
            if (index >= it->second.getArraySize()) {
                error(filename, 0, 0, "数组索引越界: " + name + "[" + std::to_string(index) + "]");
            }
            it->second.getArrayElement(index) = value;
            if (index == 0) {
                it->second.value = value.value;
            }
        }
    }

    
    std::string getVar(const std::string& name) {
        auto it = vars.find(name);
        if (it == vars.end()) {
            error(filename, 0, 0, "未声明的变量 \'" + name + "\'");
        }
        if (it->second.isArray()) {
            return it->second.arrayToString();
        }
        return it->second.value;
    }

    
    std::string getArrayElement(const std::string& name, size_t index) {
        auto it = vars.find(name);
        if (it == vars.end()) {
            error(filename, 0, 0, "未声明的数组 \'" + name + "\'");
        }
        if (!it->second.isArray()) {
            error(filename, 0, 0, "变量 \'" + name + "\' 不是数组类型");
        }
        if (!it->second.dims.empty()) {
            return it->second.getElement({index}).value;
        }
        if (index >= it->second.getArraySize()) {
            error(filename, 0, 0, "数组索引越界: " + name + "[" + std::to_string(index) + "]");
        }
        return it->second.getArrayElement(index).value;
    }

    
    double getNumericVar(const std::string& name) {
        auto it = vars.find(name);
        if (it == vars.end()) {
            error(filename, 0, 0, "未声明的变量 \'" + name + "\'");
        }
        if (!it->second.isNumeric()) {
            error(filename, 0, 0, "变量 \'" + name + "\' 不是数值类型");
        }
        return it->second.getNumericValue();
    }

    
    double getNumericArrayElement(const std::string& name, size_t index) {
        auto it = vars.find(name);
        if (it == vars.end()) {
            error(filename, 0, 0, "未声明的数组 \'" + name + "\'");
        }
        if (!it->second.isArray()) {
            error(filename, 0, 0, "变量 \'" + name + "\' 不是数组类型");
        }
        if (!it->second.dims.empty()) {
            return it->second.getElement({index}).getNumericValue();
        }
        if (index >= it->second.getArraySize()) {
            error(filename, 0, 0, "数组索引越界: " + name + "[" + std::to_string(index) + "]");
        }
        return it->second.getArrayElement(index).getNumericValue();
    }

    
    VarType getVarType(const std::string& name) {
        auto it = vars.find(name);
        if (it == vars.end()) {
            error(filename, 0, 0, "未声明的变量 \'" + name + "\'");
        }
        return it->second.type;
    }

    
    size_t getArraySize(const std::string& name) {
        auto it = vars.find(name);
        if (it == vars.end()) {
            error(filename, 0, 0, "未声明的数组 \'" + name + "\'");
        }
        if (!it->second.isArray()) {
            error(filename, 0, 0, "变量 \'" + name + "\' 不是数组类型");
        }
        return it->second.getArraySize();
    }

    
    bool hasVar(const std::string& name) {
        return vars.find(name) != vars.end();
    }

    
    bool isArrayVar(const std::string& name) {
        auto it = vars.find(name);
        if (it == vars.end()) return false;
        return it->second.isArray();
    }

    
    std::string getTypeName(VarType type) {
        switch (type) {
        case VarType::INT: return "int";
        case VarType::DOUBLE: return "double";
        case VarType::OMNI: return "omni";
        case VarType::STRING: return "string";
        case VarType::ARRAY: return "array";
        default: return "unknown";
        }
    }

    
    double stringToDouble(const std::string& str) {
        try {
            return std::stod(str);
        } catch (const std::exception&) {
            error(filename, 0, 0, "无法将字符串 \'" + str + "\' 转换为数字");
            return 0;
        }
    }

    
    std::string doubleToString(double num) {
        if (num == static_cast<long long>(num)) {
            return std::to_string(static_cast<long long>(num));
        }
        std::string s = std::to_string(num);
        s.erase(s.find_last_not_of('0') + 1, std::string::npos);
        if (s.back() == '.') {
            s.erase(s.find_last_not_of('.') + 1, std::string::npos);
        }
        return s;
    }

private:
    
    double parseFactor(const std::vector<Token>& tokens, size_t& index) {
        if (index >= tokens.size()) {
            error(filename, 0, 0, "表达式意外结束");
        }

        if (tokens[index].type == TokenType::MINUS) {
            index++;
            return -parseFactor(tokens, index);
        }

        if (tokens[index].type == TokenType::LPAREN) {
            index++;
            double result = parseExpression(tokens, index);
            if (index >= tokens.size() || tokens[index].type != TokenType::RPAREN) {
                error(filename, tokens[index].line, tokens[index].col, "缺少闭合括号 \')\'");
            }
            index++;
            return result;
        }

        if (tokens[index].type == TokenType::NUMBER) {
            double value = stringToDouble(tokens[index].lexeme);
            index++;
            return value;
        }

        if (tokens[index].type == TokenType::IDENTIFIER) {
            std::string varName = tokens[index].lexeme;
            index++;

            if (index < tokens.size() && tokens[index].type == TokenType::LBRACKET) {
                index++;
                double idxValue = parseExpression(tokens, index);
                if (index >= tokens.size() || tokens[index].type != TokenType::RBRACKET) {
                    error(filename, tokens[index].line, tokens[index].col, "缺少闭合方括号 \']\'");
                }
                index++;

                size_t arrayIndex = static_cast<size_t>(idxValue);
                if (idxValue < 0 || idxValue != static_cast<long long>(idxValue)) {
                    error(filename, 0, 0, "数组索引必须是正整数");
                }

                auto it = vars.find(varName);
                if (it != vars.end() && !it->second.dims.empty()) {
                    return it->second.getElement({arrayIndex}).getNumericValue();
                }
                return getNumericArrayElement(varName, arrayIndex);
            }

            return getNumericVar(varName);
        }

        if (tokens[index].type == TokenType::DTIME_FUNC) {
            index++;
            if (!hasTocExecuted) {
                error(filename, tokens[index-1].line, tokens[index-1].col, 
                      "dtime() 必须在 dtime_toc 之后使用");
            }
            return lastTocTime;
        }

        error(filename, tokens[index].line, tokens[index].col,
              "表达式中期望数字、变量或括号，但得到 \'" + tokens[index].lexeme + "\'");
        return 0.0;
    }

    
    double parseTerm(const std::vector<Token>& tokens, size_t& index) {
        double result = parseFactor(tokens, index);
        while (index < tokens.size() &&
               (tokens[index].type == TokenType::MULTIPLY ||
                tokens[index].type == TokenType::DIVIDE)) {
            std::string op = tokens[index].lexeme;
            index++;
            double right = parseFactor(tokens, index);
            result = performArithmeticOp(result, op, right);
        }
        return result;
    }

    
    double parseExpression(const std::vector<Token>& tokens, size_t& index) {
        double result = parseTerm(tokens, index);
        while (index < tokens.size() &&
               (tokens[index].type == TokenType::PLUS ||
                tokens[index].type == TokenType::MINUS)) {
            std::string op = tokens[index].lexeme;
            index++;
            double right = parseTerm(tokens, index);
            result = performArithmeticOp(result, op, right);
        }
        return result;
    }

    
    double performArithmeticOp(double left, const std::string& op, double right) {
        if (op == "+") return left + right;
        if (op == "-") return left - right;
        if (op == "*") return left * right;
        if (op == "/") {
            if (right == 0) error(filename, 0, 0, "除零错误");
            return left / right;
        }
        error(filename, 0, 0, "未知运算符 \'" + op + "\'");
        return 0;
    }

public:
    
    double evaluateExpr(const std::vector<Token>& tokens, size_t& index) {
        return parseExpression(tokens, index);
    }

    
    std::vector<Variable> parseMultiArrayInitializer(const std::vector<Token>& tokens, size_t& ip, 
                                                     std::vector<size_t>& dims) {
        std::vector<Variable> elements;

        if (tokens[ip].type != TokenType::LBRACE) {
            error(filename, tokens[ip].line, tokens[ip].col, "数组初始化缺少 \'{\'");
        }
        ip++;

        size_t currentDimSize = 0;
        std::vector<size_t> subDims;

        while (ip < tokens.size() && tokens[ip].type != TokenType::RBRACE) {
            if (tokens[ip].type == TokenType::LBRACE) {
                std::vector<size_t> innerDims;
                std::vector<Variable> subElements = parseMultiArrayInitializer(tokens, ip, innerDims);
                elements.insert(elements.end(), subElements.begin(), subElements.end());
                currentDimSize++;

                if (subDims.empty()) {
                    subDims = innerDims;
                } else {
                    if (subDims != innerDims) {
                        error(filename, tokens[ip].line, tokens[ip].col, "多维数组各维度大小不一致");
                    }
                }
            } else {
                if (tokens[ip].type == TokenType::NUMBER) {
                    elements.push_back(Variable(VarType::OMNI, tokens[ip].lexeme));
                    ip++;
                } else if (tokens[ip].type == TokenType::STRING) {
                    elements.push_back(Variable(VarType::OMNI, tokens[ip].lexeme));
                    ip++;
                } else if (tokens[ip].type == TokenType::IDENTIFIER) {
                    elements.push_back(Variable(VarType::OMNI, getVar(tokens[ip].lexeme)));
                    ip++;
                } else {
                    error(filename, tokens[ip].line, tokens[ip].col, "数组元素必须是数字、字符串或标识符");
                }
                currentDimSize++;
            }

            if (ip < tokens.size() && tokens[ip].type == TokenType::COMMA) {
                ip++;
            }
        }

        if (ip >= tokens.size() || tokens[ip].type != TokenType::RBRACE) {
            error(filename, tokens[ip].line, tokens[ip].col, "数组初始化缺少 \'}\'");
        }
        ip++;

        dims.clear();
        dims.push_back(currentDimSize);
        dims.insert(dims.end(), subDims.begin(), subDims.end());

        return elements;
    }

    
    void loadLibrary(const std::string& libName) {
        std::string libFilename = libName;
        if (libFilename.find('.') == std::string::npos) {
            libFilename += ".lib";
        }
        std::ifstream libFile(libFilename);
        if (!libFile.is_open()) {
            error(filename, 0, 0, "无法打开库文件 \'" + libFilename + "\'");
            return;
        }
        std::string libSrc((std::istreambuf_iterator<char>(libFile)),
                          std::istreambuf_iterator<char>());
        libFile.close();
        Scanner libScanner(libSrc, libFilename);
        auto libTokens = libScanner.scan();
        execute(libTokens);
    }

    
    std::string evaluateTernary(const std::vector<Token>& tokens, size_t& index) {
        if (tokens[index].type != TokenType::IDENTIFIER) {
            error(filename, tokens[index].line, tokens[index].col,
                  "三元运算符条件必须是标识符");
        }
        std::string condVar = tokens[index].lexeme;
        double condValue = getNumericVar(condVar);
        index++;
        if (index >= tokens.size() || tokens[index].type != TokenType::QUESTION) {
            error(filename, tokens[index].line, tokens[index].col,
                  "三元表达式缺少问号");
        }
        index++;
        std::string trueValue;
        if (tokens[index].type == TokenType::STRING) {
            trueValue = tokens[index].lexeme;
            index++;
        } else if (tokens[index].type == TokenType::NUMBER) {
            trueValue = tokens[index].lexeme;
            index++;
        } else if (tokens[index].type == TokenType::IDENTIFIER) {
            trueValue = getVar(tokens[index].lexeme);
            index++;
        } else {
            error(filename, tokens[index].line, tokens[index].col,
                  "三元表达式真分支格式错误");
        }
        if (index >= tokens.size() || tokens[index].type != TokenType::COLON) {
            error(filename, tokens[index].line, tokens[index].col,
                  "三元表达式缺少冒号");
        }
        index++;
        std::string falseValue;
        if (tokens[index].type == TokenType::STRING) {
            falseValue = tokens[index].lexeme;
            index++;
        } else if (tokens[index].type == TokenType::NUMBER) {
            falseValue = tokens[index].lexeme;
            index++;
        } else if (tokens[index].type == TokenType::IDENTIFIER) {
            falseValue = getVar(tokens[index].lexeme);
            index++;
        } else {
            error(filename, tokens[index].line, tokens[index].col,
                  "三元表达式假分支格式错误");
        }
        return condValue != 0 ? trueValue : falseValue;
    }

    
    std::vector<Variable> parseArrayInitializer(const std::vector<Token>& tokens, size_t& ip) {
        std::vector<Variable> elements;

        if (tokens[ip].type != TokenType::LBRACE) {
            error(filename, tokens[ip].line, tokens[ip].col, "数组初始化缺少 \'{\'");
        }
        ip++;

        while (ip < tokens.size() && tokens[ip].type != TokenType::RBRACE) {
            if (tokens[ip].type == TokenType::NUMBER) {
                elements.push_back(Variable(VarType::OMNI, tokens[ip].lexeme));
                ip++;
            } else if (tokens[ip].type == TokenType::STRING) {
                elements.push_back(Variable(VarType::OMNI, tokens[ip].lexeme));
                ip++;
            } else if (tokens[ip].type == TokenType::IDENTIFIER) {
                elements.push_back(Variable(VarType::OMNI, getVar(tokens[ip].lexeme)));
                ip++;
            } else {
                error(filename, tokens[ip].line, tokens[ip].col, "数组元素必须是数字、字符串或标识符");
            }

            if (ip < tokens.size() && tokens[ip].type == TokenType::COMMA) {
                ip++;
            }
        }

        if (ip >= tokens.size() || tokens[ip].type != TokenType::RBRACE) {
            error(filename, tokens[ip].line, tokens[ip].col, "数组初始化缺少 \'}\'");
        }
        ip++;

        return elements;
    }

    
    bool parseVariableDeclaration(const std::vector<Token>& tokens, size_t& ip) {
        TokenType declType = tokens[ip].type;
        if (declType != TokenType::CREATE_INT &&
            declType != TokenType::CREATE_DOUBLE &&
            declType != TokenType::CREATE_OMNI &&
            declType != TokenType::CREATE_STRING &&
            declType != TokenType::CREATE_ARR) {
            return false;
        }

        
        if (ip + 2 < tokens.size() &&
            tokens[ip+1].type == TokenType::IDENTIFIER &&
            tokens[ip+2].type == TokenType::SEMICOLON) {

            if (declType == TokenType::CREATE_ARR) {
                
                return false;
            }

            std::string name = tokens[ip+1].lexeme;
            VarType type = VarType::DOUBLE;
            if (declType == TokenType::CREATE_INT) type = VarType::INT;
            else if (declType == TokenType::CREATE_DOUBLE) type = VarType::DOUBLE;
            else if (declType == TokenType::CREATE_OMNI) type = VarType::OMNI;
            else if (declType == TokenType::CREATE_STRING) type = VarType::STRING;

            std::string initVal = (type == VarType::STRING) ? "" : "0";
            setVar(name, type, initVal);
            ip += 3;
            return true;
        }

        
        if (declType == TokenType::CREATE_ARR && ip + 5 < tokens.size() &&
            tokens[ip+1].type == TokenType::IDENTIFIER &&
            tokens[ip+2].type == TokenType::ASSIGN &&
            tokens[ip+3].type == TokenType::LBRACE) {

            size_t tempIp = ip;
            std::string name = tokens[tempIp+1].lexeme;
            tempIp += 3;

            std::vector<size_t> dims;
            std::vector<Variable> elements = parseMultiArrayInitializer(tokens, tempIp, dims);

            if (tempIp < tokens.size() && tokens[tempIp].type == TokenType::SEMICOLON) {
                if (dims.empty()) {
                    setArrayVar(name, elements);
                } else {
                    size_t total = 1;
                    for (size_t d : dims) total *= d;
                    if (elements.size() != total) {
                        error(filename, tokens[ip].line, tokens[ip].col, 
                              "数组初始化元素数量不匹配，期望 " + std::to_string(total) + 
                              " 个，实际 " + std::to_string(elements.size()) + " 个");
                    }
                    Variable arr(VarType::ARRAY);
                    arr.setDims(dims);
                    for (size_t i = 0; i < elements.size(); i++) {
                        arr.flatData[i] = elements[i];
                    }
                    vars[name] = arr;
                }
                ip = tempIp + 1;
                return true;
            }
        }

        
        if (declType == TokenType::CREATE_ARR && ip + 4 < tokens.size() &&
            tokens[ip+1].type == TokenType::IDENTIFIER) {

            size_t tempIp = ip + 2;
            std::string name = tokens[ip+1].lexeme;
            std::vector<size_t> dims;

            while (tempIp < tokens.size() && tokens[tempIp].type == TokenType::LBRACKET) {
                tempIp++;
                if (tempIp < tokens.size() && tokens[tempIp].type == TokenType::NUMBER) {
                    size_t size = static_cast<size_t>(stringToDouble(tokens[tempIp].lexeme));
                    dims.push_back(size);
                    tempIp++;
                    if (tempIp < tokens.size() && tokens[tempIp].type == TokenType::RBRACKET) {
                        tempIp++;
                    } else {
                        break;
                    }
                } else {
                    break;
                }
            }

            if (!dims.empty() && tempIp < tokens.size() && tokens[tempIp].type == TokenType::SEMICOLON) {
                Variable arr(VarType::ARRAY);
                arr.setDims(dims);
                vars[name] = arr;
                ip = tempIp + 1;
                return true;
            }
        }

        
        if (ip + 4 < tokens.size() &&
            tokens[ip+1].type == TokenType::IDENTIFIER &&
            tokens[ip+2].type == TokenType::ASSIGN &&
            (tokens[ip+3].type == TokenType::NUMBER ||
             tokens[ip+3].type == TokenType::STRING ||
             tokens[ip+3].type == TokenType::IDENTIFIER) &&
            tokens[ip+4].type == TokenType::SEMICOLON) {

            std::string name = tokens[ip+1].lexeme;
            VarType type = VarType::DOUBLE;
            if (declType == TokenType::CREATE_INT) type = VarType::INT;
            else if (declType == TokenType::CREATE_DOUBLE) type = VarType::DOUBLE;
            else if (declType == TokenType::CREATE_OMNI) type = VarType::OMNI;
            else if (declType == TokenType::CREATE_STRING) type = VarType::STRING;

            const Token& rightToken = tokens[ip+3];

            if (rightToken.type == TokenType::NUMBER) {
                setVar(name, type, rightToken.lexeme);
            } else if (rightToken.type == TokenType::STRING) {
                setVar(name, type, rightToken.lexeme);
            } else if (rightToken.type == TokenType::IDENTIFIER) {
                
                if (!hasVar(rightToken.lexeme)) {
                    error(filename, rightToken.line, rightToken.col,
                          "赋值时右侧变量 \'" + rightToken.lexeme + "\' 未声明");
                }
                Variable rightVar = vars[rightToken.lexeme];
                setVar(name, type, rightVar.value);
            }
            ip += 5;
            return true;
        }

        
        if (ip + 4 < tokens.size() &&
            tokens[ip+1].type == TokenType::IDENTIFIER &&
            tokens[ip+2].type == TokenType::ASSIGN) {
            size_t exprStart = ip + 3;
            size_t exprEnd = exprStart;
            while (exprEnd < tokens.size() && tokens[exprEnd].type != TokenType::SEMICOLON) {
                exprEnd++;
            }
            if (exprEnd < tokens.size() && tokens[exprEnd].type == TokenType::SEMICOLON) {
                std::vector<Token> exprTokens(tokens.begin() + exprStart,
                                             tokens.begin() + exprEnd);
                VarType type = VarType::DOUBLE;
                if (declType == TokenType::CREATE_INT) type = VarType::INT;
                else if (declType == TokenType::CREATE_DOUBLE) type = VarType::DOUBLE;
                else if (declType == TokenType::CREATE_OMNI) type = VarType::OMNI;
                else if (declType == TokenType::CREATE_STRING) type = VarType::STRING;

                if (type == VarType::STRING) {
                    error(filename, tokens[ip].line, tokens[ip].col,
                          "字符串类型不支持表达式赋值");
                } else {
                    size_t idx = 0;
                    double result = evaluateExpr(exprTokens, idx);
                    setVar(tokens[ip+1].lexeme, type, doubleToString(result));
                }
                ip = exprEnd + 1;
                return true;
            }
        }
        return false;
    }

    
    bool parseAssignment(const std::vector<Token>& tokens, size_t& ip) {
        if (parseVariableDeclaration(tokens, ip)) {
            return true;
        }

        
        if (tokens[ip].type == TokenType::IDENTIFIER) {
            size_t startIp = ip;
            std::string arrayName = tokens[ip].lexeme;
            ip++;

            std::vector<size_t> indices;
            bool isArrayAccess = false;

            while (ip < tokens.size() && tokens[ip].type == TokenType::LBRACKET) {
                isArrayAccess = true;
                ip++;

                size_t exprEnd = ip;
                while (exprEnd < tokens.size() && tokens[exprEnd].type != TokenType::RBRACKET) {
                    exprEnd++;
                }
                if (exprEnd >= tokens.size() || tokens[exprEnd].type != TokenType::RBRACKET) {
                    ip = startIp;
                    break;
                }

                std::vector<Token> indexExpr(tokens.begin() + ip, tokens.begin() + exprEnd);
                size_t idx = 0;
                double indexValue = evaluateExpr(indexExpr, idx);

                if (indexValue < 0 || indexValue != static_cast<long long>(indexValue)) {
                    error(filename, tokens[ip].line, tokens[ip].col, "数组索引必须是正整数");
                }

                indices.push_back(static_cast<size_t>(indexValue));
                ip = exprEnd + 1;
            }

            if (isArrayAccess) {
                if (ip < tokens.size() && tokens[ip].type == TokenType::ASSIGN) {
                    ip++;

                    size_t valueStart = ip;
                    size_t valueEnd = valueStart;
                    while (valueEnd < tokens.size() && tokens[valueEnd].type != TokenType::SEMICOLON) {
                        valueEnd++;
                    }

                    if (valueEnd < tokens.size() && tokens[valueEnd].type == TokenType::SEMICOLON) {
                        std::vector<Token> valueTokens(tokens.begin() + valueStart, tokens.begin() + valueEnd);

                        auto it = vars.find(arrayName);
                        if (it == vars.end()) {
                            error(filename, tokens[startIp].line, tokens[startIp].col, "未声明的数组");
                        }

                        Variable val(VarType::OMNI);
                        if (valueTokens.size() == 1) {
                            if (valueTokens[0].type == TokenType::NUMBER) {
                                val.value = valueTokens[0].lexeme;
                            } else if (valueTokens[0].type == TokenType::STRING) {
                                val.value = valueTokens[0].lexeme;
                            } else if (valueTokens[0].type == TokenType::IDENTIFIER) {
                                if (!hasVar(valueTokens[0].lexeme)) {
                                    error(filename, valueTokens[0].line, valueTokens[0].col,
                                          "赋值时右侧变量 \'" + valueTokens[0].lexeme + "\' 未声明");
                                }
                                val.value = getVar(valueTokens[0].lexeme);
                            }
                        } else {
                            size_t exprIdx = 0;
                            double result = evaluateExpr(valueTokens, exprIdx);
                            val.value = doubleToString(result);
                        }

                        if (!it->second.dims.empty()) {
                            if (indices.size() != it->second.dims.size()) {
                                error(filename, tokens[startIp].line, tokens[startIp].col, "维度不匹配");
                            }
                            it->second.getElement(indices) = val;
                            if (indices.size() == 1 && indices[0] == 0) {
                                it->second.value = val.value;
                            }
                        } else {
                            setArrayElement(arrayName, indices[0], val);
                        }

                        ip = valueEnd + 1;
                        return true;
                    }
                }
                ip = startIp;
            }
        }

        
        
        if (tokens[ip].type == TokenType::IDENTIFIER && ip + 3 < tokens.size() &&
            tokens[ip+1].type == TokenType::ASSIGN &&
            tokens[ip+3].type == TokenType::SEMICOLON) {

            std::string leftName = tokens[ip].lexeme;
            const Token& rightToken = tokens[ip+2];

            
            if (!hasVar(leftName)) {
                error(filename, tokens[ip].line, tokens[ip].col,
                      "变量 \'" + leftName + "\' 未声明，不能赋值");
            }

            
            if (rightToken.type == TokenType::NUMBER) {
                std::string val = rightToken.lexeme;
                VarType type = getVarType(leftName);
                setVar(leftName, type, val);
                ip += 4;
                return true;
            }

            
            if (rightToken.type == TokenType::STRING) {
                std::string val = rightToken.lexeme;
                VarType type = getVarType(leftName);
                setVar(leftName, type, val);
                ip += 4;
                return true;
            }

            
            if (rightToken.type == TokenType::IDENTIFIER) {
                std::string rightName = rightToken.lexeme;
                if (!hasVar(rightName)) {
                    error(filename, rightToken.line, rightToken.col,
                          "赋值时右侧变量 \'" + rightName + "\' 未声明");
                }
                Variable rightVar = vars[rightName];
                VarType leftType = getVarType(leftName);
                setVar(leftName, leftType, rightVar.value);
                ip += 4;
                return true;
            }
        }

        
        if (tokens[ip].type == TokenType::IDENTIFIER && ip + 2 < tokens.size() &&
            tokens[ip+1].type == TokenType::ASSIGN) {
            size_t exprStart = ip + 2;
            size_t exprEnd = exprStart;
            while (exprEnd < tokens.size() && tokens[exprEnd].type != TokenType::SEMICOLON) {
                exprEnd++;
            }
            if (exprEnd < tokens.size() && tokens[exprEnd].type == TokenType::SEMICOLON) {
                std::vector<Token> exprTokens(tokens.begin() + exprStart,
                                             tokens.begin() + exprEnd);
                std::string name = tokens[ip].lexeme;
                if (!hasVar(name)) {
                    error(filename, tokens[ip].line, tokens[ip].col,
                          "变量 \'" + name + "\' 未声明，不能赋值表达式");
                }
                VarType type = getVarType(name);
                if (type == VarType::STRING) {
                    error(filename, tokens[ip].line, tokens[ip].col,
                          "字符串类型不支持表达式赋值");
                } else {
                    size_t idx = 0;
                    double result = evaluateExpr(exprTokens, idx);
                    setVar(name, type, doubleToString(result));
                }
                ip = exprEnd + 1;
                return true;
            }
        }

        
        if (tokens[ip].type == TokenType::IDENTIFIER && ip + 3 < tokens.size() &&
            (tokens[ip+1].type == TokenType::PLUS_EQUALS ||
             tokens[ip+1].type == TokenType::MINUS_EQUALS ||
             tokens[ip+1].type == TokenType::STAR_EQUALS ||
             tokens[ip+1].type == TokenType::SLASH_EQUALS) &&
            (tokens[ip+2].type == TokenType::NUMBER ||
             tokens[ip+2].type == TokenType::IDENTIFIER) &&
            tokens[ip+3].type == TokenType::SEMICOLON) {
            std::string varName = tokens[ip].lexeme;
            if (!hasVar(varName)) {
                error(filename, tokens[ip].line, tokens[ip].col,
                      "变量 \'" + varName + "\' 未声明");
            }
            std::string op = tokens[ip+1].lexeme;
            op = op.substr(0, 1);
            double rightVal = 0;
            if (tokens[ip+2].type == TokenType::NUMBER) {
                rightVal = stringToDouble(tokens[ip+2].lexeme);
            } else {
                if (!hasVar(tokens[ip+2].lexeme)) {
                    error(filename, tokens[ip+2].line, tokens[ip+2].col,
                          "变量 \'" + tokens[ip+2].lexeme + "\' 未声明");
                }
                rightVal = getNumericVar(tokens[ip+2].lexeme);
            }
            double currentVal = getNumericVar(varName);
            double result = performArithmeticOp(currentVal, op, rightVal);
            VarType type = getVarType(varName);
            setVar(varName, type, doubleToString(result));
            ip += 4;
            return true;
        }

        
        if (tokens[ip].type == TokenType::IDENTIFIER && ip + 2 < tokens.size() &&
            (tokens[ip+1].type == TokenType::PLUS_PLUS ||
             tokens[ip+1].type == TokenType::MINUS_MINUS) &&
            tokens[ip+2].type == TokenType::SEMICOLON) {
            std::string varName = tokens[ip].lexeme;
            if (!hasVar(varName)) {
                error(filename, tokens[ip].line, tokens[ip].col,
                      "变量 \'" + varName + "\' 未声明");
            }
            double currentVal = getNumericVar(varName);
            double result = currentVal;
            if (tokens[ip+1].type == TokenType::PLUS_PLUS) {
                result = currentVal + 1;
            } else {
                result = currentVal - 1;
            }
            VarType type = getVarType(varName);
            setVar(varName, type, doubleToString(result));
            ip += 3;
            return true;
        }
        return false;
    }

    
    std::vector<std::string> parseFunctionArguments(const std::vector<Token>& tokens, size_t& ip) {
        std::vector<std::string> args;

        if (tokens[ip].type != TokenType::LPAREN) {
            error(filename, tokens[ip].line, tokens[ip].col, "函数调用缺少 \'(\'");
        }
        ip++;

        if (tokens[ip].type == TokenType::RPAREN) {
            ip++;
            return args;
        }

        while (ip < tokens.size()) {
            if (tokens[ip].type == TokenType::NUMBER) {
                args.push_back(tokens[ip].lexeme);
                ip++;
            } else if (tokens[ip].type == TokenType::STRING) {
                args.push_back(tokens[ip].lexeme);
                ip++;
            } else if (tokens[ip].type == TokenType::IDENTIFIER) {
                if (!hasVar(tokens[ip].lexeme)) {
                    error(filename, tokens[ip].line, tokens[ip].col,
                          "函数参数变量 \'" + tokens[ip].lexeme + "\' 未声明");
                }
                args.push_back(getVar(tokens[ip].lexeme));
                ip++;
            } else {
                error(filename, tokens[ip].line, tokens[ip].col, "函数参数必须是数字、字符串或变量");
            }

            if (tokens[ip].type == TokenType::COMMA) {
                ip++;
            } else if (tokens[ip].type == TokenType::RPAREN) {
                ip++;
                break;
            } else {
                error(filename, tokens[ip].line, tokens[ip].col, "函数参数列表缺少 \',\' 或 \')\'");
            }
        }

        return args;
    }

    
    void callFunction(const std::string& funcName, const std::vector<std::string>& args, size_t& ip) {
        auto it = functions.find(funcName);
        if (it == functions.end()) {
            error(filename, 0, 0, "未定义的函数 \'" + funcName + "\'");
        }

        Function& func = it->second;

        callStack.push(vars);
        vars.clear();

        std::string allArgs;
        for (size_t i = 0; i < args.size(); ++i) {
            if (i > 0) allArgs += ",";
            allArgs += args[i];
        }

        if (!allArgs.empty()) {
            vars["_args"] = Variable(VarType::STRING, allArgs);
        }

        execute(func.body);

        vars = callStack.top();
        callStack.pop();
    }

    
    void execute(const std::vector<Token>& tokens) {
        size_t ip = 0;
        while (ip < tokens.size()) {
            const Token& t = tokens[ip];

            if (t.type == TokenType::DTIME_TIC) {
                timer.startTime = std::chrono::high_resolution_clock::now();
                timer.isRunning = true;
                hasTocExecuted = false;
                ip++;
                continue;
            }

            if (t.type == TokenType::DTIME_TOC) {
                if (!timer.isRunning) {
                    error(filename, t.line, t.col, "dtime_toc 必须在 dtime_tic 之后使用");
                }

                auto endTime = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - timer.startTime);
                lastTocTime = duration.count() / 1000.0;
                timer.isRunning = false;
                hasTocExecuted = true;

                ip++;

                if (ip < tokens.size() && tokens[ip].type == TokenType::LPAREN) {
                    ip++;

                    if (ip < tokens.size() && tokens[ip].type == TokenType::IDENTIFIER) {
                        std::string varName = tokens[ip].lexeme;
                        ip++;

                        if (ip < tokens.size() && tokens[ip].type == TokenType::RPAREN) {
                            ip++;

                            if (ip < tokens.size() && tokens[ip].type == TokenType::SEMICOLON) {
                                ip++;
                                setVar(varName, VarType::DOUBLE, doubleToString(lastTocTime));
                                continue;
                            }
                        }
                    }

                    ip = ip - 1;
                }

                if (ip < tokens.size() && tokens[ip].type == TokenType::LPAREN) {
                    ip++;
                    if (ip < tokens.size() && tokens[ip].type == TokenType::RPAREN) {
                        ip++;
                        if (ip < tokens.size() && tokens[ip].type == TokenType::SEMICOLON) {
                            ip++;
                        }
                    }
                }
                continue;
            }

            
            if (t.type == TokenType::RANDOM) {
                size_t startIp = ip;
                ip++;  

                double minVal = -1e9;
                double maxVal = 1e9;
                bool isIntRange = false;

                if (ip < tokens.size() && tokens[ip].type == TokenType::DOT) {
                    ip++;
                    isIntRange = true;

                    if (ip < tokens.size() && tokens[ip].type == TokenType::NUMBER) {
                        minVal = stringToDouble(tokens[ip].lexeme);
                        ip++;
                    } else {
                        error(filename, t.line, t.col, "random 范围语法错误: 缺少最小值");
                    }

                    if (ip < tokens.size() && tokens[ip].type == TokenType::TILDE) {
                        ip++;
                    } else {
                        error(filename, t.line, t.col, "random 范围语法错误: 缺少 ~");
                    }

                    if (ip < tokens.size() && tokens[ip].type == TokenType::NUMBER) {
                        maxVal = stringToDouble(tokens[ip].lexeme);
                        ip++;
                    } else {
                        error(filename, t.line, t.col, "random 范围语法错误: 缺少最大值");
                    }

                    if (minVal > maxVal) {
                        error(filename, t.line, t.col, "random 范围错误: 最小值不能大于最大值");
                    }
                }

                if (ip < tokens.size() && tokens[ip].type == TokenType::LPAREN) {
                    ip++;
                } else {
                    error(filename, t.line, t.col, "random 语法错误: 缺少 \'(\'");
                }

                std::string varName;
                if (ip < tokens.size() && tokens[ip].type == TokenType::IDENTIFIER) {
                    varName = tokens[ip].lexeme;
                    ip++;
                } else {
                    error(filename, t.line, t.col, "random 语法错误: 括号内必须指定变量名");
                }

                if (ip < tokens.size() && tokens[ip].type == TokenType::RPAREN) {
                    ip++;
                } else {
                    error(filename, t.line, t.col, "random 语法错误: 缺少 \')\'");
                }

                if (ip < tokens.size() && tokens[ip].type == TokenType::SEMICOLON) {
                    ip++;
                } else {
                    error(filename, t.line, t.col, "random 语句必须加分号");
                }

                
                if (!hasVar(varName)) {
                    error(filename, tokens[ip-4].line, tokens[ip-4].col,  
                          "变量 \'" + varName + "\' 未声明，random 目标必须已声明");
                }

                double randomValue;
                if (isIntRange) {
                    int imin = static_cast<int>(minVal);
                    int imax = static_cast<int>(maxVal);
                    std::uniform_int_distribution<int> dist(imin, imax);
                    randomValue = dist(rng);
                } else {
                    std::uniform_real_distribution<double> dist(minVal, maxVal);
                    randomValue = dist(rng);
                }

                VarType type = getVarType(varName);
                setVar(varName, type, doubleToString(randomValue));

                continue;
            }

            
            if (t.type == TokenType::CREATE && ip + 5 < tokens.size() &&
                tokens[ip+1].type == TokenType::IDENTIFIER &&
                tokens[ip+2].type == TokenType::LPAREN &&
                tokens[ip+3].type == TokenType::FUNCTION &&
                tokens[ip+4].type == TokenType::RPAREN &&
                (tokens[ip+5].type == TokenType::FALID || 
                 tokens[ip+5].type == TokenType::VALID)) {

                std::string funcName = tokens[ip+1].lexeme;
                bool hasReturn = (tokens[ip+5].type == TokenType::VALID);

                size_t bodyStart = ip + 6;
                if (bodyStart >= tokens.size() || tokens[bodyStart].type != TokenType::LBRACE) {
                    error(filename, t.line, t.col, "函数定义缺少 \'{\'");
                }

                size_t depth = 1;
                size_t bodyEnd = bodyStart + 1;
                while (bodyEnd < tokens.size() && depth > 0) {
                    if (tokens[bodyEnd].type == TokenType::LBRACE) depth++;
                    else if (tokens[bodyEnd].type == TokenType::RBRACE) depth--;
                    bodyEnd++;
                }
                if (depth != 0) {
                    error(filename, t.line, t.col, "函数定义缺少闭合的 \'}\'");
                }

                std::vector<Token> body(tokens.begin() + bodyStart + 1, 
                                       tokens.begin() + bodyEnd - 1);

                functions[funcName] = Function(funcName, body, hasReturn);

                ip = bodyEnd;
                continue;
            }

            
            if (t.type == TokenType::IDENTIFIER && ip + 1 < tokens.size() &&
                tokens[ip+1].type == TokenType::LPAREN) {

                std::string funcName = t.lexeme;
                size_t startIp = ip;

                ip += 2;

                std::vector<std::string> args;

                int parenDepth = 1;
                size_t paramStart = ip;

                while (ip < tokens.size() && parenDepth > 0) {
                    if (tokens[ip].type == TokenType::LPAREN) parenDepth++;
                    else if (tokens[ip].type == TokenType::RPAREN) parenDepth--;
                    ip++;
                }

                if (parenDepth != 0) {
                    ip = startIp;
                } else {
                    size_t paramEnd = ip - 1;

                    if (paramEnd > paramStart) {
                        size_t paramIdx = paramStart;
                        while (paramIdx < paramEnd && tokens[paramIdx].type == TokenType::COMMA) {
                            paramIdx++;
                        }

                        if (paramIdx < paramEnd) {
                            if (tokens[paramIdx].type == TokenType::IDENTIFIER) {
                                if (!hasVar(tokens[paramIdx].lexeme)) {
                                    error(filename, tokens[paramIdx].line, tokens[paramIdx].col,
                                          "函数参数变量 \'" + tokens[paramIdx].lexeme + "\' 未声明");
                                }
                                args.push_back(getVar(tokens[paramIdx].lexeme));
                            } else if (tokens[paramIdx].type == TokenType::NUMBER) {
                                args.push_back(tokens[paramIdx].lexeme);
                            } else if (tokens[paramIdx].type == TokenType::STRING) {
                                args.push_back(tokens[paramIdx].lexeme);
                            }
                        }
                    }
                }

                if (ip < tokens.size() && tokens[ip].type == TokenType::SEMICOLON) {
                    callFunction(funcName, args, ip);
                    ip++;
                    continue;
                } else {
                    ip = startIp;
                }
            }

            
            if (t.type == TokenType::OUTPUT_REDIRECT) {
                std::vector<Token> outputTokens;
                outputTokens.push_back(tokens[ip]);
                ip++;
                while (ip < tokens.size() && tokens[ip].type != TokenType::SEMICOLON) {
                    outputTokens.push_back(tokens[ip]);
                    ip++;
                }
                if (ip >= tokens.size() || tokens[ip].type != TokenType::SEMICOLON) {
                    error(filename, t.line, t.col, "输出语句缺少分号");
                }
                ip++;
                parseOutputStatement(outputTokens);
                continue;
            }

            
            if (t.type == TokenType::OUTLB && ip + 1 < tokens.size() &&
                tokens[ip+1].type == TokenType::SEMICOLON) {
                std::cout << std::endl;
                ip += 2;
                continue;
            }

            
            if (t.type == TokenType::INPUT_REDIRECT && ip + 2 < tokens.size() &&
                tokens[ip+1].type == TokenType::IDENTIFIER &&
                tokens[ip+2].type == TokenType::SEMICOLON) {
                std::string varName = tokens[ip+1].lexeme;
                if (!hasVar(varName)) {
                    error(filename, tokens[ip+1].line, tokens[ip+1].col,
                          "变量 \'" + varName + "\' 未声明，不能接收输入");
                }
                std::string input;
                std::getline(std::cin, input);
                VarType type = getVarType(varName);
                setVar(varName, type, input);
                ip += 3;
                continue;
            }

            
            if (parseAssignment(tokens, ip)) {
                continue;
            }

            
            if (t.type == TokenType::IF && ip + 5 < tokens.size() &&
                tokens[ip+1].type == TokenType::LPAREN &&
                tokens[ip+2].type == TokenType::IDENTIFIER &&
                (tokens[ip+3].type == TokenType::GT || tokens[ip+3].type == TokenType::LT ||
                 tokens[ip+3].type == TokenType::GTE || tokens[ip+3].type == TokenType::LTE ||
                 tokens[ip+3].type == TokenType::EQ || tokens[ip+3].type == TokenType::NEQ) &&
                (tokens[ip+4].type == TokenType::NUMBER || tokens[ip+4].type == TokenType::IDENTIFIER) &&
                tokens[ip+5].type == TokenType::RPAREN) {

                std::string varName = tokens[ip+2].lexeme;
                std::string op = tokens[ip+3].lexeme;
                std::string limitStr = tokens[ip+4].lexeme;
                int limit = 0;
                if (tokens[ip+4].type == TokenType::NUMBER) {
                    limit = stringToDouble(limitStr);
                } else {
                    if (!hasVar(limitStr)) {
                        error(filename, tokens[ip+4].line, tokens[ip+4].col,
                              "变量 \'" + limitStr + "\' 未声明");
                    }
                    limit = getNumericVar(limitStr);
                }
                if (!hasVar(varName)) {
                    error(filename, tokens[ip+2].line, tokens[ip+2].col,
                          "变量 \'" + varName + "\' 未声明");
                }
                double cur = getNumericVar(varName);
                bool cond = false;
                if (op == ">") cond = (cur > limit);
                else if (op == "<") cond = (cur < limit);
                else if (op == ">=") cond = (cur >= limit);
                else if (op == "<=") cond = (cur <= limit);
                else if (op == "==") cond = (cur == limit);
                else if (op == "!=") cond = (cur != limit);

                size_t braceStart = ip + 6;
                if (braceStart >= tokens.size() || tokens[braceStart].type != TokenType::LBRACE) {
                    error(filename, t.line, t.col, "if 语句缺少 {");
                }
                size_t braceEnd = braceStart + 1;
                int depth = 1;
                while (braceEnd < tokens.size() && depth > 0) {
                    if (tokens[braceEnd].type == TokenType::LBRACE) depth++;
                    else if (tokens[braceEnd].type == TokenType::RBRACE) depth--;
                    braceEnd++;
                }
                if (depth != 0) error(filename, t.line, t.col, "if 语句缺少闭合的 }");

                if (cond) {
                    std::vector<Token> body(tokens.begin() + braceStart + 1, tokens.begin() + braceEnd - 1);
                    Interpreter sub(filename);
                    sub.vars = vars;
                    sub.execute(body);
                    vars = sub.vars;
                }
                ip = braceEnd;
                continue;
            }

            
            if (t.type == TokenType::WHILE && ip + 5 < tokens.size() &&
                tokens[ip+1].type == TokenType::LPAREN &&
                tokens[ip+2].type == TokenType::IDENTIFIER &&
                (tokens[ip+3].type == TokenType::LT || tokens[ip+3].type == TokenType::LTE ||
                 tokens[ip+3].type == TokenType::GT || tokens[ip+3].type == TokenType::GTE ||
                 tokens[ip+3].type == TokenType::EQ || tokens[ip+3].type == TokenType::NEQ) &&
                (tokens[ip+4].type == TokenType::NUMBER || tokens[ip+4].type == TokenType::IDENTIFIER) &&
                tokens[ip+5].type == TokenType::RPAREN) {

                std::string varName = tokens[ip+2].lexeme;
                std::string op = tokens[ip+3].lexeme;
                std::string limitStr = tokens[ip+4].lexeme;
                int limit = 0;
                if (tokens[ip+4].type == TokenType::NUMBER) {
                    limit = stringToDouble(limitStr);
                } else {
                    if (!hasVar(limitStr)) {
                        error(filename, tokens[ip+4].line, tokens[ip+4].col,
                              "变量 \'" + limitStr + "\' 未声明");
                    }
                    limit = getNumericVar(limitStr);
                }
                if (!hasVar(varName)) {
                    error(filename, tokens[ip+2].line, tokens[ip+2].col,
                          "变量 \'" + varName + "\' 未声明");
                }

                size_t braceStart = ip + 6;
                if (braceStart >= tokens.size() || tokens[braceStart].type != TokenType::LBRACE) {
                    error(filename, t.line, t.col, "while 循环缺少 {");
                }
                size_t braceEnd = braceStart + 1;
                int depth = 1;
                while (braceEnd < tokens.size() && depth > 0) {
                    if (tokens[braceEnd].type == TokenType::LBRACE) depth++;
                    else if (tokens[braceEnd].type == TokenType::RBRACE) depth--;
                    braceEnd++;
                }
                if (depth != 0) error(filename, t.line, t.col, "while 循环缺少闭合的 }");

                while (true) {
                    double cur = getNumericVar(varName);
                    bool cond = false;
                    if (op == "<") cond = (cur < limit);
                    else if (op == "<=") cond = (cur <= limit);
                    else if (op == ">") cond = (cur > limit);
                    else if (op == ">=") cond = (cur >= limit);
                    else if (op == "==") cond = (cur == limit);
                    else if (op == "!=") cond = (cur != limit);
                    if (!cond) break;

                    std::vector<Token> body(tokens.begin() + braceStart + 1, tokens.begin() + braceEnd - 1);
                    Interpreter sub(filename);
                    sub.vars = vars;
                    sub.execute(body);
                    vars = sub.vars;
                }
                ip = braceEnd;
                continue;
            }

            
            if (t.type == TokenType::LOOP && ip + 3 < tokens.size() &&
                tokens[ip+1].type == TokenType::LPAREN) {

                int count = 0;
                if (tokens[ip+2].type == TokenType::NUMBER) {
                    count = stringToDouble(tokens[ip+2].lexeme);
                } else if (tokens[ip+2].type == TokenType::IDENTIFIER) {
                    if (!hasVar(tokens[ip+2].lexeme)) {
                        error(filename, tokens[ip+2].line, tokens[ip+2].col,
                              "变量 \'" + tokens[ip+2].lexeme + "\' 未声明");
                    }
                    count = getNumericVar(tokens[ip+2].lexeme);
                } else {
                    error(filename, t.line, t.col, "loop 循环次数必须是数字或标识符");
                }
                if (tokens[ip+3].type != TokenType::RPAREN) {
                    error(filename, t.line, t.col, "loop 语句括号不匹配");
                }

                size_t braceStart = ip + 4;
                if (braceStart >= tokens.size() || tokens[braceStart].type != TokenType::LBRACE) {
                    error(filename, t.line, t.col, "loop 语句缺少 {");
                }
                size_t braceEnd = braceStart + 1;
                int depth = 1;
                while (braceEnd < tokens.size() && depth > 0) {
                    if (tokens[braceEnd].type == TokenType::LBRACE) depth++;
                    else if (tokens[braceEnd].type == TokenType::RBRACE) depth--;
                    braceEnd++;
                }
                if (depth != 0) error(filename, t.line, t.col, "loop 语句缺少闭合的 }");

                std::vector<Token> body(tokens.begin() + braceStart + 1, tokens.begin() + braceEnd - 1);
                for (int i = 0; i < count; i++) {
                    Interpreter sub(filename);
                    sub.vars = vars;
                    sub.execute(body);
                    vars = sub.vars;
                }
                ip = braceEnd;
                continue;
            }

            
            if (t.type == TokenType::FOR && ip + 13 < tokens.size() &&
                tokens[ip+1].type == TokenType::LPAREN &&
                tokens[ip+2].type == TokenType::CREATE_INT &&
                tokens[ip+3].type == TokenType::IDENTIFIER &&
                tokens[ip+4].type == TokenType::ASSIGN &&
                (tokens[ip+5].type == TokenType::NUMBER || tokens[ip+5].type == TokenType::IDENTIFIER) &&
                tokens[ip+6].type == TokenType::SEMICOLON &&
                tokens[ip+7].type == TokenType::IDENTIFIER &&
                (tokens[ip+8].type == TokenType::LT || tokens[ip+8].type == TokenType::LTE ||
                 tokens[ip+8].type == TokenType::GT || tokens[ip+8].type == TokenType::GTE ||
                 tokens[ip+8].type == TokenType::EQ) &&
                (tokens[ip+9].type == TokenType::NUMBER || tokens[ip+9].type == TokenType::IDENTIFIER) &&
                tokens[ip+10].type == TokenType::SEMICOLON &&
                tokens[ip+11].type == TokenType::IDENTIFIER &&
                tokens[ip+12].type == TokenType::PLUS_PLUS &&
                tokens[ip+13].type == TokenType::RPAREN) {

                std::string initVar = tokens[ip+3].lexeme;
                std::string initValStr = tokens[ip+5].lexeme;
                int initVal = 0;
                if (tokens[ip+5].type == TokenType::NUMBER) {
                    initVal = stringToDouble(initValStr);
                } else {
                    if (!hasVar(initValStr)) {
                        error(filename, tokens[ip+5].line, tokens[ip+5].col,
                              "变量 \'" + initValStr + "\' 未声明");
                    }
                    initVal = getNumericVar(initValStr);
                }
                std::string condVar = tokens[ip+7].lexeme;
                std::string op = tokens[ip+8].lexeme;
                std::string limitStr = tokens[ip+9].lexeme;
                int limit = 0;
                if (tokens[ip+9].type == TokenType::NUMBER) {
                    limit = stringToDouble(limitStr);
                } else {
                    if (!hasVar(limitStr)) {
                        error(filename, tokens[ip+9].line, tokens[ip+9].col,
                              "变量 \'" + limitStr + "\' 未声明");
                    }
                    limit = getNumericVar(limitStr);
                }
                std::string updateVar = tokens[ip+11].lexeme;

                size_t braceStart = ip + 14;
                if (braceStart >= tokens.size() || tokens[braceStart].type != TokenType::LBRACE) {
                    error(filename, t.line, t.col, "for 循环缺少 {");
                }
                size_t braceEnd = braceStart + 1;
                int depth = 1;
                while (braceEnd < tokens.size() && depth > 0) {
                    if (tokens[braceEnd].type == TokenType::LBRACE) depth++;
                    else if (tokens[braceEnd].type == TokenType::RBRACE) depth--;
                    braceEnd++;
                }
                if (depth != 0) error(filename, t.line, t.col, "for 循环缺少闭合的 }");

                setVar(initVar, VarType::INT, std::to_string(initVal));
                while (true) {
                    if (!hasVar(condVar)) {
                        error(filename, tokens[ip+7].line, tokens[ip+7].col,
                              "变量 \'" + condVar + "\' 未声明");
                    }
                    double cur = getNumericVar(condVar);
                    bool cond = false;
                    if (op == "<") cond = (cur < limit);
                    else if (op == "<=") cond = (cur <= limit);
                    else if (op == ">") cond = (cur > limit);
                    else if (op == ">=") cond = (cur >= limit);
                    else if (op == "==") cond = (cur == limit);
                    if (!cond) break;

                    std::vector<Token> body(tokens.begin() + braceStart + 1, tokens.begin() + braceEnd - 1);
                    Interpreter sub(filename);
                    sub.vars = vars;
                    sub.execute(body);
                    vars = sub.vars;

                    if (!hasVar(updateVar)) {
                        error(filename, tokens[ip+11].line, tokens[ip+11].col,
                              "变量 \'" + updateVar + "\' 未声明");
                    }
                    double current_dbl = getNumericVar(updateVar);
                    setVar(updateVar, VarType::INT, std::to_string(static_cast<int>(current_dbl) + 1));
                }
                ip = braceEnd;
                continue;
            }

            
            if (t.type == TokenType::FINISH && ip + 4 < tokens.size() &&
                tokens[ip+1].type == TokenType::LPAREN &&
                tokens[ip+2].type == TokenType::IDENTIFIER &&
                tokens[ip+3].type == TokenType::RPAREN &&
                tokens[ip+4].type == TokenType::SEMICOLON) {

                std::string funcName = tokens[ip+2].lexeme;
                if (funcName != "main") {
                    error(filename, tokens[ip+2].line, tokens[ip+2].col,
                          "finish 语句必须写为 finish(main); 其他名称不被允许");
                }
                exit(0);
            }

            
            if (t.type == TokenType::ACCEPT && ip + 4 < tokens.size() &&
                tokens[ip+1].type == TokenType::IDENTIFIER &&
                tokens[ip+2].lexeme == "function" &&
                tokens[ip+3].type == TokenType::DOT &&
                tokens[ip+4].type == TokenType::IDENTIFIER &&
                tokens[ip+4].lexeme == "main") {

                size_t acceptStart = ip + 5;
                while (acceptStart < tokens.size() && tokens[acceptStart].type != TokenType::PIPE) {
                    acceptStart++;
                }

                if (acceptStart < tokens.size() && tokens[acceptStart].type == TokenType::PIPE) {
                    acceptStart++;
                    if (acceptStart + 1 < tokens.size() &&
                        tokens[acceptStart].type == TokenType::IDENTIFIER &&
                        tokens[acceptStart].lexeme == "set" &&
                        tokens[acceptStart+1].type == TokenType::IDENTIFIER &&
                        (acceptStart + 2 >= tokens.size() || tokens[acceptStart+2].type == TokenType::SEMICOLON)) {

                        std::string varName = tokens[acceptStart+1].lexeme;

                        
                        if (!hasVar(varName)) {
                            error(filename, tokens[acceptStart+1].line, tokens[acceptStart+1].col,
                                  "变量 \'" + varName + "\' 未声明，accept 目标必须已声明");
                        }

                        auto it = vars.find("_args");
                        if (it != vars.end()) {
                            vars[varName] = it->second;
                        } else {
                            setVar(varName, VarType::OMNI, "");
                        }

                        ip = acceptStart + 2;
                        if (ip < tokens.size() && tokens[ip].type == TokenType::SEMICOLON) {
                            ip++;
                        }
                        continue;
                    }
                }

                error(filename, t.line, t.col, "accept>function.main 语法错误");
            }

            ip++;
        }
    }

    
    void parseOutputStatement(const std::vector<Token>& tokens) {
        if (tokens.empty()) return;
        size_t i = 0;
        if (tokens[i].type != TokenType::OUTPUT_REDIRECT) {
            error(filename, tokens[i].line, tokens[i].col, "输出语句必须以output开始");
        }
        i++;
        bool newline = false;
        std::string outputContent;
        while (i < tokens.size()) {
            if (tokens[i].type == TokenType::OUTLB) {
                newline = true;
                i++;
                continue;
            }
            if (tokens[i].type == TokenType::OUTPUT_CONNECT) {
                i++;
                continue;
            }
            if (tokens[i].type == TokenType::STRING) {
                outputContent += tokens[i].lexeme;
                i++;
            } else if (tokens[i].type == TokenType::DTIME_FUNC) {
                if (!hasTocExecuted) {
                    error(filename, tokens[i].line, tokens[i].col, 
                          "dtime() 必须在 dtime_toc 之后使用");
                }
                outputContent += doubleToString(lastTocTime);
                i++;
            } else if (tokens[i].type == TokenType::IDENTIFIER) {
                if (i + 1 < tokens.size() && tokens[i+1].type == TokenType::LBRACKET) {
                    std::string arrayName = tokens[i].lexeme;
                    i += 2;

                    std::vector<size_t> indices;

                    while (i < tokens.size() && tokens[i].type != TokenType::OUTLB && 
                           tokens[i].type != TokenType::OUTPUT_CONNECT) {
                        if (tokens[i].type == TokenType::LBRACKET) {
                            i++;
                            continue;
                        }

                        if (tokens[i].type == TokenType::RBRACKET) {
                            i++;
                            continue;
                        }

                        size_t exprEnd = i;
                        while (exprEnd < tokens.size() && tokens[exprEnd].type != TokenType::RBRACKET) {
                            exprEnd++;
                        }

                        if (exprEnd < tokens.size()) {
                            std::vector<Token> indexExpr(tokens.begin() + i, tokens.begin() + exprEnd);
                            size_t idx = 0;
                            double indexValue = evaluateExpr(indexExpr, idx);
                            indices.push_back(static_cast<size_t>(indexValue));
                            i = exprEnd + 1;
                        } else {
                            break;
                        }
                    }

                    auto it = vars.find(arrayName);
                    if (it == vars.end()) {
                        error(filename, tokens[i].line, tokens[i].col, "未声明的数组");
                    }

                    if (!it->second.dims.empty()) {
                        if (indices.size() != it->second.dims.size()) {
                            error(filename, tokens[i].line, tokens[i].col, "维度不匹配");
                        }
                        outputContent += it->second.getElement(indices).value;
                    } else {
                        outputContent += getArrayElement(arrayName, indices[0]);
                    }
                    continue;
                } else {
                    if (!hasVar(tokens[i].lexeme)) {
                        error(filename, tokens[i].line, tokens[i].col,
                              "变量 \'" + tokens[i].lexeme + "\' 未声明");
                    }
                    outputContent += getVar(tokens[i].lexeme);
                    i++;
                }
            } else if (tokens[i].type == TokenType::NUMBER) {
                outputContent += tokens[i].lexeme;
                i++;
            } else {
                size_t exprEnd = i;
                while (exprEnd < tokens.size() &&
                       tokens[exprEnd].type != TokenType::OUTLB &&
                       tokens[exprEnd].type != TokenType::OUTPUT_CONNECT) {
                    exprEnd++;
                }
                if (exprEnd > i) {
                    std::vector<Token> exprTokens(tokens.begin() + i, tokens.begin() + exprEnd);
                    size_t idx = 0;
                    try {
                        double result = evaluateExpr(exprTokens, idx);
                        outputContent += doubleToString(result);
                    } catch (...) {
                        error(filename, tokens[i].line, tokens[i].col, "输出表达式错误");
                    }
                    i = exprEnd;
                } else {
                    break;
                }
            }
        }
        if (newline) {
            std::cout << outputContent << std::endl;
        } else {
            std::cout << outputContent << std::flush;
        }
    }
};


void check_update() {
    std::cout << "\033[1;34m正在检查 Wei 语言解释器更新...\033[0m" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(800));
    const char* latest_version = "0.9.5";
    const char* release_date = "2026-02-17";
    std::cout << "\033[1;32m检查完成\033[0m" << std::endl;
    std::cout << "当前安装版本: v" << WL_VERSION << " (" << WL_RELEASE_DATE << ")" << std::endl;
    if (strcmp(WL_VERSION, latest_version) == 0) {
        std::cout << "\033[1;33m您已使用最新 Aurora 版本! (" << release_date << ")\033[0m" << std::endl;
        std::cout << "\033[1;36m欢迎访问 wanw.us.ci 了解更多信息\033[0m" << std::endl;
        std::cout << "\033[1;32m  重要变更: 移除自动创建变量！左侧变量必须已声明，否则报错\033[0m" << std::endl;
        std::cout << "\033[1;32m           纯声明 create.int x; 正常工作，默认值0\033[0m" << std::endl;
        std::cout << "\033[1;32m           变量间赋值 b = a; 正常工作，类型按左侧\033[0m" << std::endl;
    } else {
        std::cout << "\033[1;31m可用新版本: v" << latest_version << " (" << release_date << ")\033[0m" << std::endl;
        std::cout << "\033[1;33m  重要变更:\033[0m" << std::endl;
        std::cout << "     彻底移除自动创建变量！所有赋值左侧必须已声明" << std::endl;
        std::cout << "     纯声明: create.int x;" << std::endl;
        std::cout << "     变量间赋值: x = y;" << std::endl;
    }
    std::cout << std::endl << "\033[1;34m更新检查完成\033[0m" << std::endl;
}


int main(int argc, char* argv[]) {
    if (argc == 2) {
        if (strcmp(argv[1], "-v") == 0) {
            std::cout << "\033[1;35m创造无限可能！\033[0m" << std::endl;
            std::cout << "维语言解释器 wl" << std::endl;
            std::cout << "版本: "<< WL_VERSION;
            if(WL_YN_INTERSTELLAR == 1){
                std::cout<<"测试版"<<std::endl;
            } else if(WL_YN_INTERSTELLAR == 0){
                std::cout<<"正式版"<<std::endl;
            }
            std::cout << "最新更新日期: " << WL_RELEASE_DATE << std::endl;
            return 0;
        } else if (strcmp(argv[1], "-u") == 0) {
            check_update();
            return 0;
        }
    }

    if (argc != 2) {
        std::cerr << "\033[1;33m用法: " << argv[0] << " <源文件.wei> | -v | -u\033[0m" << std::endl;
        std::cerr << "  " << argv[0] << " -v       : 显示版本信息" << std::endl;
        std::cerr << "  " << argv[0] << " -u       : 检查更新" << std::endl;
        std::cerr << "  " << argv[0] << " program.wei : 编译并执行源文件" << std::endl;
        return 1;
    }

    std::string filename = argv[1];

    struct stat buffer;
    if (stat(filename.c_str(), &buffer) != 0) {
        std::cerr << "\033[1;31mwl: 无法打开源文件 \'" << filename
                  << "\': 没有该文件或目录\033[0m" << std::endl;
        return 1;
    }

    std::ifstream f(filename);
    if (!f.is_open()) {
        std::cerr << "\033[1;31mwl: 无法读取文件 \'" << filename << "\': 权限不足\033[0m" << std::endl;
        return 1;
    }

    std::string src((std::istreambuf_iterator<char>(f)),
                   std::istreambuf_iterator<char>());

    Scanner scanner(src, filename);
    std::vector<Token> tokens = scanner.scan();

    bool hasUtilizeCore = false;
    for (size_t i = 0; i < tokens.size() && i < 10; ++i) {
        if (tokens[i].type == TokenType::UTILIZE) {
            if (i + 1 < tokens.size() && tokens[i+1].type == TokenType::IDENTIFIER &&
                tokens[i+1].lexeme == "core") {
                hasUtilizeCore = true;
                break;
            }
        }
    }
    if (!hasUtilizeCore) {
        error(filename, 0, 0, "源文件必须以 \'!utilize core\' 开头");
    }

    bool foundMain = false;
    size_t mainBodyStart = 0, mainBodyEnd = 0;
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (i + 8 < tokens.size() &&
            tokens[i].type == TokenType::CREATE &&
            tokens[i+1].type == TokenType::IDENTIFIER &&
            tokens[i+1].lexeme == "main" &&
            tokens[i+2].type == TokenType::LPAREN &&
            tokens[i+3].type == TokenType::FUNCTION &&
            tokens[i+4].type == TokenType::RPAREN &&
            (tokens[i+5].type == TokenType::FALID || tokens[i+5].type == TokenType::VALID) &&
            tokens[i+6].type == TokenType::LBRACE) {
            foundMain = true;
            mainBodyStart = i + 7;
            size_t depth = 1;
            mainBodyEnd = mainBodyStart;
            while (mainBodyEnd < tokens.size() && depth > 0) {
                if (tokens[mainBodyEnd].type == TokenType::LBRACE) depth++;
                else if (tokens[mainBodyEnd].type == TokenType::RBRACE) depth--;
                mainBodyEnd++;
            }
            if (depth != 0) {
                error(filename, tokens[i].line, tokens[i].col, "main 函数缺少闭合的 }");
            }
            break;
        }
    }

    if (!foundMain) {
        error(filename, 0, 0, "未检测到符合规范的主函数。请使用: create main(function).falid { ... } 或 create main(function).valid { ... }");
    }

    std::vector<Token> mainBody(tokens.begin() + mainBodyStart, tokens.begin() + mainBodyEnd - 1);

    Interpreter interp(filename);

    size_t ip = 0;
    while (ip < tokens.size()) {
        if (tokens[ip].type == TokenType::CREATE && ip + 5 < tokens.size() &&
            tokens[ip+1].type == TokenType::IDENTIFIER &&
            tokens[ip+2].type == TokenType::LPAREN &&
            tokens[ip+3].type == TokenType::FUNCTION &&
            tokens[ip+4].type == TokenType::RPAREN &&
            (tokens[ip+5].type == TokenType::FALID || tokens[ip+5].type == TokenType::VALID)) {

            std::string funcName = tokens[ip+1].lexeme;
            bool hasReturn = (tokens[ip+5].type == TokenType::VALID);

            size_t bodyStart = ip + 6;
            if (bodyStart >= tokens.size() || tokens[bodyStart].type != TokenType::LBRACE) {
                error(filename, tokens[ip].line, tokens[ip].col, "函数定义缺少 \'{\'");
            }

            size_t depth = 1;
            size_t bodyEnd = bodyStart + 1;
            while (bodyEnd < tokens.size() && depth > 0) {
                if (tokens[bodyEnd].type == TokenType::LBRACE) depth++;
                else if (tokens[bodyEnd].type == TokenType::RBRACE) depth--;
                bodyEnd++;
            }
            if (depth != 0) {
                error(filename, tokens[ip].line, tokens[ip].col, "函数定义缺少闭合的 \'}\'");
            }

            std::vector<Token> body(tokens.begin() + bodyStart + 1, 
                                   tokens.begin() + bodyEnd - 1);

            interp.functions[funcName] = Function(funcName, body, hasReturn);

            ip = bodyEnd;
        } else {
            ip++;
        }
    }

    interp.execute(mainBody);
    return 0;
}
