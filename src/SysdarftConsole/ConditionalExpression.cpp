#include "debugger_operand.h"
#include <EncodingDecoding.h>
#include <SysdarftMain.h>
#include <algorithm>
#include <cctype>
#include <memory>
#include <regex>
#include <string>
#include <vector>

std::unique_ptr<RemoteDebugServer::ConditionalTargetExpression> RemoteDebugServer::Parser::parseExpression()
{
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

void RemoteDebugServer::Parser::skipWhitespace() {
    while (pos < input.size() && std::isspace(input[pos]))
        pos++;
}

bool RemoteDebugServer::Parser::match(const std::string &keyword)
{
    skipWhitespace();
    if (input.substr(pos, keyword.size()) == keyword) {
        pos += keyword.size();
        return true;
    }
    return false;
}

std::unique_ptr<RemoteDebugServer::ConditionalTargetExpression> RemoteDebugServer::Parser::parseValEqual()
{
    auto node = std::make_unique<ConditionalTargetExpression>();
    node->ConditionType = VAL_EQUAL;

    node->TgExp1 = parseUntilCommaOrParen();
    if (input[pos] == ',') {
        pos++;
    } // consume comma

    node->TgExp2 = parseUntilParen();

    if (input[pos] == ')')
        pos++; // consume ')'

    std::string TgExp1Prc = node->TgExp1;
    std::string TgExp2Prc = node->TgExp2;

    remove_space(TgExp1Prc);
    remove_space(TgExp2Prc);

    capitalization(TgExp1Prc);
    capitalization(TgExp2Prc);

    encode_target(node->EncodedTgExp1, TgExp1Prc);
    encode_target(node->EncodedTgExp2, TgExp2Prc);

    return node;
}

std::unique_ptr<RemoteDebugServer::ConditionalTargetExpression> RemoteDebugServer::Parser::parseLogical(const int condType)
{
    auto node = std::make_unique<ConditionalTargetExpression>();
    node->ConditionType = condType;

    node->ComplexTgExp1 = parseExpression();
    skipWhitespace();
    if (input[pos] == ',') {
        pos++;
    } // consume comma

    node->ComplexTgExp2 = parseExpression();
    skipWhitespace();
    if (input[pos] == ')') {
        pos++;
    } // consume closing ')'

    return node;
}

// Regex processing remains unchanged
std::string RemoteDebugServer::Parser::processTgExp(const std::string &exp)
{
    const std::regex angleBracketRegex("<([^>]*)>");
    if (std::smatch match; std::regex_match(exp, match, angleBracketRegex)) {
        return match.str(1); // content inside brackets
    }
    return exp;
}

std::string RemoteDebugServer::Parser::parseUntilCommaOrParen()
{
    skipWhitespace();
    std::string result;
    int angleCount = 0;
    while (pos < input.size())
    {
        const char c = input[pos];
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

std::string RemoteDebugServer::Parser::parseUntilParen()
{
    skipWhitespace();
    std::string result;
    int angleCount = 0;
    while (pos < input.size()) {
        const char c = input[pos];
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

bool RemoteDebugServer::check_one_level_of_condition(const ConditionalTargetExpression * expr)
{
    if (!expr) {
        return true;
    }

    uint64_t data1;
    uint64_t data2;
    if (!expr->EncodedTgExp1.empty()) {
        data1 = absolute_target_access(expr->EncodedTgExp1);
    } else {
        data1 = check_one_level_of_condition(expr->ComplexTgExp1.get());
    }

    if (!expr->EncodedTgExp2.empty()) {
        data2 = absolute_target_access(expr->EncodedTgExp2);
    } else {
        data2 = check_one_level_of_condition(expr->ComplexTgExp2.get());
    }

    // check condition
    if (expr->ConditionType == VAL_EQUAL) {
        return data1 == data2;
    } else if (expr->ConditionType == AND) {
        return data1 && data2;
    } else if (expr->ConditionType == OR) {
        return data1 || data2;
    } else if (expr->ConditionType == NOT) {
        return !data1;
    }

    // TODO: Report Error?
    return false;
}

bool RemoteDebugServer::is_condition_met(const std::string & condition)
{
    if (condition.empty()) {
        return true;
    }

    Parser parser(condition);
    const auto root = parser.parseExpression();
    return check_one_level_of_condition(root.get());
}

uint64_t RemoteDebugServer::absolute_target_access(const std::vector < uint8_t > & binary) const
{
    debugger_operand_type dbg_operand(CPUInstance, binary);
    return dbg_operand.get_val();
}

void RemoteDebugServer::absolute_target_store(const std::vector < uint8_t > & binary, const uint64_t data)
{
    debugger_operand_type dbg_operand(CPUInstance, binary);
    dbg_operand.set_val(data);
}
