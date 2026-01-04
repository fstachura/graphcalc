#include <string>
#include <vector>
#include <exception>
#include <memory>

const std::vector<std::pair<std::string, size_t>> FUNCTIONS = {
    {"sin", 1},
    {"cos", 1},
    {"tan", 1},
    {"asin", 1},
    {"acos", 1},
    {"atan", 1},
    {"sinh", 1},
    {"cosh", 1},
    {"tanh", 1},
    {"asinh", 1},
    {"acosh", 1},
    {"atanh", 1},
    {"exp", 1},
    {"log", 1},
    {"exp2", 1},
    {"log2", 1},
};

const std::vector<std::pair<std::string, size_t>> BUILTIN_FUNCTIONS = {
    {"mod", 2},
    {"min", 2},
    {"max", 2},
    {"floor", 1},
    {"ceil", 1},
    {"abs", 1},
    {"inversesqrt", 1},
    {"sqrt", 1},
};

const std::vector<std::string> BUILTIN_CONSTS = {
    "x", "y", "pi", "e"
};

enum class TokenType {
    Identifier,
    Number,
    Plus,
    Minus,
    Mult,
    Div,
    Power,
    Comma,
    ParenStart,
    ParenEnd,
};

std::string to_string(TokenType tok) {
    switch (tok) {
        case TokenType::Identifier:
            return "Identifier";
        case TokenType::Number:
            return "Number";
        case TokenType::Plus:
            return "Plus";
        case TokenType::Minus:
            return "Minus";
        case TokenType::Mult:
            return "Mult";
        case TokenType::Div:
            return "Div";
        case TokenType::Power:
            return "Power";
        case TokenType::Comma:
            return "Comma";
        case TokenType::ParenStart:
            return "ParenStart";
        case TokenType::ParenEnd:
            return "ParenEnd";
    }
}

struct Token {
    TokenType type;
    std::string token;
};

enum class ExpectedTokenType {
    Identifier,
    Number,
    None
};

struct TokenizerError: public std::exception {
    std::string what;
    size_t pos;

    TokenizerError(std::string what, size_t pos): what(what), pos(pos) {
    }
};

std::vector<Token> tokenize(std::string expr) {
    std::vector<Token> result {};
    std::string curr_token = "";
    ExpectedTokenType curr_token_type = ExpectedTokenType::None;

    size_t i = 0;
    while (i < expr.size()) {
        if ((expr[i] >= 'a' && expr[i] <= 'z') || (expr[i] >= 'A' && expr[i] <= 'Z') || expr[i] == '_') {
            if (curr_token_type == ExpectedTokenType::Number) {
                if (curr_token == ".")
                    throw TokenizerError("invalid number", i);

                curr_token_type = ExpectedTokenType::None;
                result.push_back(Token { .type = TokenType::Number, .token = curr_token });
                curr_token.clear();
            }

            curr_token_type = ExpectedTokenType::Identifier;
            curr_token += expr[i];
            i++;
            continue;
        } else if (curr_token_type == ExpectedTokenType::Identifier) {
            curr_token_type = ExpectedTokenType::None;
            result.push_back(Token { .type = TokenType::Identifier, .token = curr_token });
            curr_token.clear();
        }

        if ((expr[i] >= '0' && expr[i] <= '9') || expr[i] == '.') {
            if (expr[i] == '.' && curr_token.find('.') != std::string::npos) {
                throw TokenizerError("two dots in number", i);
            }

            if (curr_token_type == ExpectedTokenType::Identifier) {
                curr_token_type = ExpectedTokenType::None;
                result.push_back(Token { .type = TokenType::Identifier, .token = curr_token });
                curr_token.clear();
            }

            curr_token_type = ExpectedTokenType::Number;
            curr_token += expr[i];
            i++;
            continue;
        } else if (curr_token_type == ExpectedTokenType::Number) {
            if (curr_token == ".")
                throw TokenizerError("invalid number", i);

            curr_token_type = ExpectedTokenType::None;
            result.push_back(Token { .type = TokenType::Number, .token = curr_token });
            curr_token.clear();
        }

        if (expr[i] == '+') {
            result.push_back(Token { .type = TokenType::Plus });
        } else if (expr[i] == '-') {
            result.push_back(Token { .type = TokenType::Minus });
        } else if (expr[i] == '/') {
            result.push_back(Token { .type = TokenType::Div });
        } else if (expr[i] == '(') {
            result.push_back(Token { .type = TokenType::ParenStart });
        } else if (expr[i] == ')') {
            result.push_back(Token { .type = TokenType::ParenEnd });
        } else if (expr[i] == ',') {
            result.push_back(Token { .type = TokenType::Comma });
        } else if (expr[i] == '*') {
            if (i+1 < expr.size() && expr[i+1] == '*') {
                result.push_back(Token { .type = TokenType::Power });
                i++;
            } else {
                result.push_back(Token { .type = TokenType::Mult });
            }
        }

        i++;
    }

    if (curr_token_type == ExpectedTokenType::Identifier) {
        result.push_back(Token { .type = TokenType::Identifier, .token = curr_token });
    } else if (curr_token_type == ExpectedTokenType::Number) {
        if (curr_token == ".")
            throw TokenizerError("invalid number", i);

        result.push_back(Token { .type = TokenType::Number, .token = curr_token });
    }

    return result;
}

struct Expression {
    virtual std::string to_string() const = 0;
    virtual ~Expression() {};
};

enum class BinaryOperator {
    Plus,
    Minus,
    Mult,
    Div,
    Power,
};

struct BinaryExpression: public Expression {
    BinaryOperator op;
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;

    BinaryExpression(
        std::unique_ptr<Expression> left,
        BinaryOperator op,
        std::unique_ptr<Expression> right
    ): op(op), left(std::move(left)), right(std::move(right)) {
    }

    virtual std::string to_string() const {
        std::string a = left->to_string();
        std::string b = right->to_string();
        std::string op_str;
        if (op == BinaryOperator::Plus) {
            op_str = "+";
        } else if (op == BinaryOperator::Minus) {
            op_str = "-";
        } else if (op == BinaryOperator::Mult) {
            op_str = "*";
        } else if (op == BinaryOperator::Div) {
            op_str = "/";
        } else if (op == BinaryOperator::Power) {
            return "gc_pow(" + a + ", " + b + ")";
        }

        return "(" + a + op_str + b + ")";
    }

    virtual ~BinaryExpression() {}
};

enum class UnaryOperator {
    Minus,
};

struct UnaryExpression: public Expression {
    UnaryOperator op;
    std::unique_ptr<Expression> expr;

    UnaryExpression(
        UnaryOperator op,
        std::unique_ptr<Expression> expr
    ): op(op), expr(std::move(expr)) {
    }

    virtual std::string to_string() const {
        std::string a = expr->to_string();
        std::string op_str;
        if (op == UnaryOperator::Minus) {
            op_str = "+";
        }

        return "(" + op_str + a + ")";
    }

    virtual ~UnaryExpression() {}
};

struct FunctionCall: public Expression {
    std::string name;
    std::vector<std::unique_ptr<Expression>> exprs;

    FunctionCall(std::string name, std::vector<std::unique_ptr<Expression>> exprs):
        name(name), exprs(std::move(exprs)) {}

    virtual std::string to_string() const {
        std::string result = "(" + name + "(";
        for (int i=0; i != exprs.size(); i++) {
            result += exprs[i]->to_string();
            if (i+1 < exprs.size()) {
                result += ", ";
            }
        }

        return result + "))";
    }

    virtual ~FunctionCall() {}
};

struct Const: public Expression {
    std::string name;

    Const(std::string name): name(name) {
    }

    virtual std::string to_string() const {
        return name;
    }

    virtual ~Const() {}
};

struct Number: public Expression {
    std::string value;

    Number(std::string value): value(value) {
    }

    virtual std::string to_string() const {
        return value + (value.find(".") == std::string::npos ? "." : "") + "lf";
    }

    virtual ~Number() {}
};

struct Grouping: public Expression {
    std::unique_ptr<Expression> expr;

    Grouping(std::unique_ptr<Expression> expr): expr(std::move(expr)) {
    }

    virtual std::string to_string() const {
        return expr->to_string();
    }

    virtual ~Grouping() {}
};

// based on http://www.craftinginterpreters.com/parsing-expressions.html
// expr :: add ;
// add :: mult ( ("-" | "+") mult)*;
// mult :: pow ( ("*" | "/") pow)*;
// pow :: unary ( ("**") unary)*;
// unary :: ("-") unary | call;
// call :: primary ( "(" arguments? ")" )*;
// arguments :: expression ( "," expression )*;
// primary :: NUMBER | STRING;

struct ParserError: public std::exception {
    std::string what;
    size_t pos;

    ParserError(std::string what): what(what) {
    }

    ParserError(std::string what, size_t pos): what(what), pos(pos) {
    }
};

BinaryOperator token_to_binary_op(TokenType tok, size_t pos) {
    switch (tok) {
        case TokenType::Plus:
            return BinaryOperator::Plus;
        case TokenType::Minus:
            return BinaryOperator::Minus;
        case TokenType::Div:
            return BinaryOperator::Div;
        case TokenType::Mult:
            return BinaryOperator::Mult;
        case TokenType::Power:
            return BinaryOperator::Power;
        default:
            throw ParserError("invalid binary operator " + to_string(tok), pos);
    }
}

UnaryOperator token_to_unary_op(TokenType tok, size_t pos) {
    switch (tok) {
        case TokenType::Minus:
            return UnaryOperator::Minus;
        default:
            throw ParserError("invalid unary operator " + to_string(tok), pos);
    }
}

class Parser {
    std::vector<Token> tokens {};
    size_t pos = 0;

    Token prev() {
        return tokens[pos-1];
    }

    bool is_at_end() {
        return pos >= tokens.size();
    }

    bool check(TokenType type) {
        if (is_at_end()) {
            return false;
        } else {
            return tokens[pos].type == type;
        }
    }

    Token advance() {
        if (!is_at_end()) {
            pos += 1;
        }
        return prev();
    }

    bool match_tokens(std::vector<TokenType> types) {
        for (auto&& type: types) {
            if (check(type)) {
                advance();
                return true;
            }
        }

        return false;
    }

    std::unique_ptr<Expression> expr() {
        return add();
    }

    std::unique_ptr<Expression> add() {
        auto expr = mult();

        while (match_tokens({TokenType::Plus, TokenType::Minus})) {
            auto op = prev();
            auto right = mult();
            expr = std::make_unique<BinaryExpression>(std::move(expr), token_to_binary_op(op.type, pos), std::move(right));
        }

        return expr;
    }

    std::unique_ptr<Expression> mult() {
        auto expr = pow();

        while (match_tokens({TokenType::Mult, TokenType::Div})) {
            auto op = prev();
            auto right = pow();
            expr = std::make_unique<BinaryExpression>(std::move(expr), token_to_binary_op(op.type, pos), std::move(right));
        }

        return expr;
    }

    std::unique_ptr<Expression> pow() {
        auto expr = unary();

        while (match_tokens({TokenType::Power})) {
            auto op = prev();
            auto right = unary();
            expr = std::make_unique<BinaryExpression>(std::move(expr), token_to_binary_op(op.type, pos), std::move(right));
        }

        return expr;
    }

    std::unique_ptr<Expression> unary() {
        while (match_tokens({TokenType::Minus})) {
            auto op = prev();
            auto right = unary();
            auto expr = std::make_unique<UnaryExpression>(token_to_unary_op(op.type, pos), std::move(right));
        }

        return call();
    }

    std::unique_ptr<Expression> call() {
        auto expr = primary();

        while (true) {
            if (match_tokens({TokenType::ParenStart})) {
                Const* name = dynamic_cast<Const*>(expr.get());
                if (name == nullptr) {
                    throw ParserError("expected function name", pos);
                }

                expr = finish_call(name->name);
            } else {
                break;
            }
        }

        Const* cnst = dynamic_cast<Const*>(expr.get());
        if (cnst != nullptr) {
            bool ok = false;
            for (auto&& f: BUILTIN_CONSTS) {
                if (f == cnst->name) {
                    ok = true;
                    break;
                }
            }

            if (!ok)
                throw ParserError("unknown constant " + prev().token, pos);
        }

        return expr;
    }

    std::unique_ptr<Expression> finish_call(std::string name) {
        std::vector<std::unique_ptr<Expression>> args {};

        if (!check(TokenType::ParenEnd)) {
            args.push_back(expr());
            while (match_tokens({TokenType::Comma})) {
                args.push_back(expr());
            }
        }

        if (check(TokenType::ParenEnd)) {
            advance();
        } else {
            throw ParserError("expected paren end", pos);
        }

        for (auto&& f: FUNCTIONS) {
            if (f.first == name) {
                if (f.second == args.size()) {
                    return std::make_unique<FunctionCall>("gc_" + name, std::move(args));
                } else {
                    throw ParserError("invalid number of arguments to function " + name +
                            " expected " + std::to_string(f.second) + " received " + std::to_string(args.size()), pos);
                }
            }
        }

        for (auto&& f: BUILTIN_FUNCTIONS) {
            if (f.first == name) {
                if (f.second == args.size()) {
                    return std::make_unique<FunctionCall>(name, std::move(args));
                } else {
                    throw ParserError("invalid number of arguments to function " + name +
                            " expected " + std::to_string(f.second) + " received " + std::to_string(args.size()), pos);
                }
            }
        }

        throw ParserError("unknown function " + name, pos);
    }

    std::unique_ptr<Expression> primary() {
        if (match_tokens({TokenType::Identifier})) {
            return std::make_unique<Const>(Const(prev().token));
        } else if (match_tokens({TokenType::Number})) {
            return std::make_unique<Number>(Number(prev().token));
        } else if (match_tokens({TokenType::ParenStart})) {
            auto expr = this->expr();
            if (check(TokenType::ParenEnd)) {
                advance();
            } else {
                throw ParserError("expected paren end", pos);
            }
            return std::make_unique<Grouping>(std::move(expr));
        } else {
            throw ParserError("expected expression", pos);
        }
    }

public:
    Parser(std::vector<Token> tokens): tokens(tokens) {
    }

    std::unique_ptr<Expression> parse() {
        auto expr = this->expr();
        if (!is_at_end()) {
            throw ParserError("trailing data after expression", pos);
        }
        return expr;
    }
};
