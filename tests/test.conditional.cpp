#include <algorithm>
#include <cctype>
#include <iostream>
#include <memory>
#include <regex>
#include <string>
#include <vector>

struct ConditionalTargetExpression {
    std::string TgExp1;
    std::string TgExp2;
    std::unique_ptr<ConditionalTargetExpression> ComplexTgExp1;
    std::unique_ptr<ConditionalTargetExpression> ComplexTgExp2;
    int ConditionType;  // Codes for different conditions
};

enum ConditionTypes {
    VAL_EQUAL,
    AND,
    OR,
    NOT,
    NONE
};

class Parser {
public:
    Parser(const std::string& input) : input(input), pos(0) {}

    std::unique_ptr<ConditionalTargetExpression> parseExpression() {
        skipWhitespace();
        if (match("valequal(")) {
            return parseValEqual();
        } else if (match("and(")) {
            return parseLogical(AND);
        } else if (match("or(")) {
            return parseLogical(OR);
        } else if (match("not(")) {
            return parseLogical(NOT);
        }
        return nullptr;
    }

private:
    std::string input;
    size_t pos;

    void skipWhitespace() {
        while (pos < input.size() && std::isspace(input[pos])) pos++;
    }

    bool match(const std::string& keyword) {
        skipWhitespace();
        if (input.substr(pos, keyword.size()) == keyword) {
            pos += keyword.size();
            return true;
        }
        return false;
    }

    std::unique_ptr<ConditionalTargetExpression> parseValEqual() {
        auto node = std::make_unique<ConditionalTargetExpression>();
        node->ConditionType = VAL_EQUAL;

        node->TgExp1 = parseUntilCommaOrParen();
        if (input[pos] == ',') { pos++; } // consume comma

        node->TgExp2 = parseUntilParen();

        if (input[pos] == ')') pos++; // consume ')'
        return node;
    }

    std::unique_ptr<ConditionalTargetExpression> parseLogical(int condType) {
        auto node = std::make_unique<ConditionalTargetExpression>();
        node->ConditionType = condType;

        node->ComplexTgExp1 = parseExpression();
        skipWhitespace();
        if (input[pos] == ',') { pos++; } // consume comma

        node->ComplexTgExp2 = parseExpression();
        skipWhitespace();
        if (input[pos] == ')') { pos++; } // consume closing ')'

        return node;
    }

    // Regex processing remains unchanged
    std::string processTgExp(const std::string& exp) {
        std::regex angleBracketRegex("<([^>]*)>");
        std::smatch match;
        if (std::regex_match(exp, match, angleBracketRegex)) {
            return match.str(1); // content inside brackets
        }
        return exp;
    }

    std::string parseUntilCommaOrParen() {
        skipWhitespace();
        std::string result;
        int angleCount = 0;
        while (pos < input.size()) {
            char c = input[pos];
            if (c == '<') {
                angleCount++;
            } else if (c == '>') {
                angleCount--;
            }
            // Stop at a comma or ')' only if we're not inside angle brackets.
            if (angleCount == 0 && (c == ',' || c == ')')) {
                break;
            }
            result += c;
            pos++;
        }
        return processTgExp(result);
    }

    std::string parseUntilParen() {
        skipWhitespace();
        std::string result;
        int angleCount = 0;
        while (pos < input.size()) {
            char c = input[pos];
            if (c == '<') {
                angleCount++;
            } else if (c == '>') {
                angleCount--;
            }
            // Stop at ')' only if we're not inside angle brackets.
            if (angleCount == 0 && c == ')') {
                break;
            }
            result += c;
            pos++;
        }
        return processTgExp(result);
    }
};

void printExpression(const ConditionalTargetExpression* expr, const std::string& prefix = "", bool isLeft = true) {
    if (!expr) return;

    std::cout << prefix << (isLeft ? "├──" : "└──");

    switch(expr->ConditionType) {
        case VAL_EQUAL:
            std::cout << "valequal(<" << expr->TgExp1 << ">, <" << expr->TgExp2 << ">)\n";
            break;
        case AND:
            std::cout << "and\n";
            break;
        case OR:
            std::cout << "or\n";
            break;
        case NOT:
            std::cout << "not\n";
            break;
        default:
            std::cout << "Unknown\n";
    }

    std::string childPrefix = prefix + (isLeft ? "│   " : "    ");

    if(expr->ComplexTgExp1 || expr->ComplexTgExp2) {
        printExpression(expr->ComplexTgExp1.get(), childPrefix, true);
        printExpression(expr->ComplexTgExp2.get(), childPrefix, false);
    }
}

int main() {
    std::string input = "and(or(valequal(<Tg,Exp1>,<Tg,Exp2>),not(valequal(<TgExp3>,<Tg,Exp4>))),valequal(<TgExp5>,<TgExp6>))";

    // Remove whitespace for parsing simplicity
    input.erase(std::remove_if(input.begin(), input.end(), ::isspace), input.end());

    Parser parser(input);
    auto root = parser.parseExpression();

    std::cout << "ASCII Tree Representation:\n";
    printExpression(root.get());

    return 0;
}
