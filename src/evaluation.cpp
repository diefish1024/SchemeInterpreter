/**
 * @file evaluation.cpp
 * @brief Expression evaluation implementation for the Scheme interpreter
 * @author luke36
 * 
 * This file implements evaluation methods for all expression types in the Scheme
 * interpreter. Functions are organized according to ExprType enumeration order
 * from Def.hpp for consistency and maintainability.
 */

#include "value.hpp"
#include "expr.hpp" 
#include "RE.hpp"
#include "syntax.hpp"
#include "utils.hpp"
#include <cstring>
#include <vector>
#include <map>
#include <climits>

extern std::map<std::string, ExprType> primitives;
extern std::map<std::string, ExprType> reserved_words;

Value Fixnum::eval(Assoc &e) { // evaluation of a fixnum
    return IntegerV(n);
}

Value RationalNum::eval(Assoc &e) { // evaluation of a rational number
    return RationalV(numerator, denominator);
}

Value StringExpr::eval(Assoc &e) { // evaluation of a string
    return StringV(s);
}

Value True::eval(Assoc &e) { // evaluation of #t
    return BooleanV(true);
}

Value False::eval(Assoc &e) { // evaluation of #f
    return BooleanV(false);
}

Value MakeVoid::eval(Assoc &e) { // (void)
    return VoidV();
}

Value Exit::eval(Assoc &e) { // (exit)
    return TerminateV();
}

Value Unary::eval(Assoc &e) { // evaluation of single-operator primitive
    return evalRator(rand->eval(e));
}

Value Binary::eval(Assoc &e) { // evaluation of two-operators primitive
    return evalRator(rand1->eval(e), rand2->eval(e));
}

Value Variadic::eval(Assoc &e) { // evaluation of multi-operator primitive
    std::vector<Value> eval_list;
    for (auto rand : rands) {
        eval_list.push_back(rand->eval(e));
    }
    return evalRator(eval_list);
}

Value Var::eval(Assoc &e) { // evaluation of variable
    Value matched_value = find(x, e);
    if (matched_value.get() == nullptr) {
        if (primitives.count(x)) {
                std::map<ExprType, std::pair<Expr, std::vector<std::string>>> primitive_map = {
                    {E_VOID,     {new MakeVoid(), {}}},
                    {E_EXIT,     {new Exit(), {}}},
                    {E_BOOLQ,    {new IsBoolean(new Var("parm")), {"parm"}}},
                    {E_INTQ,     {new IsFixnum(new Var("parm")), {"parm"}}},
                    {E_NULLQ,    {new IsNull(new Var("parm")), {"parm"}}},
                    {E_PAIRQ,    {new IsPair(new Var("parm")), {"parm"}}},
                    {E_PROCQ,    {new IsProcedure(new Var("parm")), {"parm"}}},
                    {E_SYMBOLQ,  {new IsSymbol(new Var("parm")), {"parm"}}},
                    {E_STRINGQ,  {new IsString(new Var("parm")), {"parm"}}},
                    {E_LISTQ,    {new IsList(new Var("parm")), {"parm"}}},
                    {E_DISPLAY,  {new Display(new Var("parm")), {"parm"}}},
                    {E_PLUS,     {new Plus(new Var("parm1"), new Var("parm2")), {"parm1","parm2"}}},
                    {E_MINUS,    {new Minus(new Var("parm1"), new Var("parm2")), {"parm1","parm2"}}},
                    {E_MUL,      {new Mult(new Var("parm1"), new Var("parm2")), {"parm1","parm2"}}},
                    {E_DIV,      {new Div(new Var("parm1"), new Var("parm2")), {"parm1","parm2"}}},
                    {E_MODULO,   {new Modulo(new Var("parm1"), new Var("parm2")), {"parm1","parm2"}}},
                    {E_EXPT,     {new Expt(new Var("parm1"), new Var("parm2")), {"parm1","parm2"}}},
                    {E_LT,       {new Less(new Var("parm1"), new Var("parm2")), {"parm1","parm2"}}},
                    {E_LE,       {new LessEq(new Var("parm1"), new Var("parm2")), {"parm1","parm2"}}},
                    {E_EQ,       {new Equal(new Var("parm1"), new Var("parm2")), {"parm1","parm2"}}},
                    {E_GE,       {new GreaterEq(new Var("parm1"), new Var("parm2")), {"parm1","parm2"}}},
                    {E_GT,       {new Greater(new Var("parm1"), new Var("parm2")), {"parm1","parm2"}}},
                    {E_EQQ,      {new IsEq(new Var("parm1"), new Var("parm2")), {"parm1","parm2"}}},
                    {E_CONS,     {new Cons(new Var("parm1"), new Var("parm2")), {"parm1","parm2"}}},
                    {E_CAR,      {new Car(new Var("parm")), {"parm"}}},
                    {E_CDR,      {new Cdr(new Var("parm")), {"parm"}}},
                    {E_LIST,     {new ListFunc({new Var("parm1"), new Var("parm2")}), {"parm1","parm2"}}},
                    {E_SETCAR,   {new SetCar(new Var("parm1"), new Var("parm2")), {"parm1","parm2"}}},
                    {E_SETCDR,   {new SetCdr(new Var("parm1"), new Var("parm2")), {"parm1","parm2"}}},
                    {E_NOT,      {new Not(new Var("parm")), {"parm"}}},
                    {E_AND,      {new AndVar({new Var("parm1"), new Var("parm2")}), {"parm1","parm2"}}},
                    {E_OR,       {new OrVar({new Var("parm1"), new Var("parm2")}), {"parm1","parm2"}}}
                };

            auto it = primitive_map.find(primitives[x]);
            if (it != primitive_map.end()) {
                return ProcedureV(it->second.second, it->second.first, e);
            }
      }
      throw RuntimeError("undefined variable: " + x);
    }
    return matched_value;
}

// helper function
auto toRational(const Value& v) -> std::pair<NumericType, NumericType> {
    if (v->v_type == V_INT) {
        NumericType n = dynamic_cast<Integer*>(v.get())->n;
        return {n, 1};
    } else if (v->v_type == V_RATIONAL) {
        Rational* r = dynamic_cast<Rational*>(v.get());
        return {r->numerator, r->denominator};
    }
    throw RuntimeError("+ is only defined for numbers");
}

Value Plus::evalRator(const Value &rand1, const Value &rand2) { // +
    auto [num1, den1] = toRational(rand1);
    auto [num2, den2] = toRational(rand2);

    NumericType res_num = (num1 * den2 + num2 * den1) / util::gcd(den1, den2);
    NumericType res_den = util::lcm(den1, den2);

    if (res_den == 1) {
        return IntegerV(res_num);
    }
    return RationalV(res_num, res_den);
}

Value Minus::evalRator(const Value &rand1, const Value &rand2) { // -
    auto [num1, den1] = toRational(rand1);
    auto [num2, den2] = toRational(rand2);

    NumericType res_num = (num1 * den2 - num2 * den1) / util::gcd(den1, den2);
    NumericType res_den = util::lcm(den1, den2);

    if (res_den == 1) {
        return IntegerV(res_num);
    }
    return RationalV(res_num, res_den);
}

Value Mult::evalRator(const Value &rand1, const Value &rand2) { // *
    auto [num1, den1] = toRational(rand1);
    auto [num2, den2] = toRational(rand2);

    NumericType res_num = num1 * num2;
    NumericType res_den = den1 * den2;
    util::normalize_rational(res_num, res_den);

    if (res_den == 1) {
        return IntegerV(res_num);
    }
    return RationalV(res_num, res_den);
}

Value Div::evalRator(const Value &rand1, const Value &rand2) { // /
    auto [num1, den1] = toRational(rand1);
    auto [num2, den2] = toRational(rand2);

    if (num2 == 0) {
        throw RuntimeError("Division by zero");
    }

    NumericType res_num = num1 * den2;
    NumericType res_den = den1 * num2;
    util::normalize_rational(res_num, res_den);

    if (res_den == 1) {
        return IntegerV(res_num);
    }
    return RationalV(res_num, res_den);
}

Value Modulo::evalRator(const Value &rand1, const Value &rand2) { // modulo
    if (rand1->v_type == V_INT && rand2->v_type == V_INT) {
        int dividend = dynamic_cast<Integer*>(rand1.get())->n;
        int divisor = dynamic_cast<Integer*>(rand2.get())->n;
        if (divisor == 0) {
            throw(RuntimeError("Division by zero"));
        }
        return IntegerV(dividend % divisor);
    }
    throw(RuntimeError("modulo is only defined for integers"));
}

Value PlusVar::evalRator(const std::vector<Value> &args) { // + with multiple args
    if (args.empty()) {
        return IntegerV(0);
    }
    NumericType res_num = 0;
    NumericType res_den = 1;
    for (const auto& arg : args) {
        auto [cur_num, cur_den] = toRational(arg);
        NumericType com_lcm = util::lcm(res_den, static_cast<NumericType>(cur_den));
        res_num = res_num * (com_lcm / res_den) + cur_num * (com_lcm / cur_den);
        res_den = com_lcm;
        util::normalize_rational(res_num, res_den);
    }
    if (res_den == 1) {
        return IntegerV(res_num);
    }
    return RationalV(res_num, res_den);
}

Value MinusVar::evalRator(const std::vector<Value> &args) { // - with multiple args
    if (args.empty()) {
        throw RuntimeError("Minus expression expects at least one argument.");
    }
    NumericType res_num;
    NumericType res_den;
    if (args.size() == 1) {
        // (- x) => -x
        auto [num, den] = toRational(args[0]);
        res_num = -num;
        res_den = den;
    } else {
        // x - y - z ...
        auto [num0, den0] = toRational(args[0]);
        res_num = num0;
        res_den = den0;
        for (size_t i = 1; i < args.size(); ++i) {
            auto [cur_num, cur_den] = toRational(args[i]);
            NumericType com_lcm = util::lcm(res_den, cur_den);
            res_num = res_num * (com_lcm / res_den) - cur_num * (com_lcm / cur_den);
            res_den = com_lcm;
            util::normalize_rational(res_num, res_den);
        }
    }
    util::normalize_rational(res_num, res_den);
    if (res_den == 1) {
        return IntegerV(res_num);
    }
    return RationalV(res_num, res_den);
}

Value MultVar::evalRator(const std::vector<Value> &args) { // * with multiple args
    if (args.empty()) {
        return IntegerV(1);
    }
    NumericType res_num = 1;
    NumericType res_den = 1;
    for (const auto& arg : args) {
        auto [cur_num, cur_den] = toRational(arg);
        res_num *= cur_num;
        res_den *= cur_den;
        util::normalize_rational(res_num, res_den);
    }
    if (res_den == 1) {
        return IntegerV(res_num);
    }
    return RationalV(res_num, res_den);
}

Value DivVar::evalRator(const std::vector<Value> &args) { // / with multiple args
    if (args.empty()) {
        throw RuntimeError("Division expression expects at least one argument.");
    }
    NumericType res_num;
    NumericType res_den;
    if (args.size() == 1) {
        // (/ x) => 1 / x
        auto [num, den] = toRational(args[0]);
        if (num == 0) {
            throw RuntimeError("Division by zero");
        }
        res_num = den;
        res_den = num;
    } else {
        // x / y / z ...
        auto [num0, den0] = toRational(args[0]);
        res_num = num0;
        res_den = den0;
        for (size_t i = 1; i < args.size(); ++i) {
            auto [cur_num, cur_den] = toRational(args[i]);
            if (cur_num == 0) {
                throw RuntimeError("Division by zero");
            }
            res_num *= cur_den;
            res_den *= cur_num;
            util::normalize_rational(res_num, res_den);
        }
    }
    util::normalize_rational(res_num, res_den);
    if (res_den == 1) {
        return IntegerV(res_num);
    }
    return RationalV(res_num, res_den);
}

Value Expt::evalRator(const Value &rand1, const Value &rand2) { // expt
    if (rand1->v_type == V_INT && rand2->v_type == V_INT) {
        int base = dynamic_cast<Integer*>(rand1.get())->n;
        int exponent = dynamic_cast<Integer*>(rand2.get())->n;
        
        if (exponent < 0) {
            throw(RuntimeError("Negative exponent not supported for integers"));
        }
        if (base == 0 && exponent == 0) {
            throw(RuntimeError("0^0 is undefined"));
        }
        
        long long result = 1;
        long long b = base;
        int exp = exponent;
        
        while (exp > 0) {
            if (exp % 2 == 1) {
                result *= b;
                if (result > INT_MAX || result < INT_MIN) {
                    throw(RuntimeError("Integer overflow in expt"));
                }
            }
            b *= b;
            if (b > INT_MAX || b < INT_MIN) {
                if (exp > 1) {
                    throw(RuntimeError("Integer overflow in expt"));
                }
            }
            exp /= 2;
        }
        
        return IntegerV((int)result);
    }
    throw(RuntimeError("Wrong typename"));
}

//A FUNCTION TO SIMPLIFY THE COMPARISON WITH INTEGER AND RATIONAL NUMBER
int compareNumericValues(const Value &v1, const Value &v2) {
    #define NumericType long long
    NumericType num1, den1;
    NumericType num2, den2;
    if (v1->v_type == V_INT) {
        num1 = dynamic_cast<Integer*>(v1.get())->n;
        den1 = 1;
    } else if (v1->v_type == V_RATIONAL) {
        Rational* r1 = dynamic_cast<Rational*>(v1.get());
        num1 = r1->numerator;
        den1 = r1->denominator;
    } else {
        throw RuntimeError("Numeric comparison expects a number");
    }
    if (v2->v_type == V_INT) {
        num2 = dynamic_cast<Integer*>(v2.get())->n;
        den2 = 1;
    } else if (v2->v_type == V_RATIONAL) {
        Rational* r2 = dynamic_cast<Rational*>(v2.get());
        num2 = r2->numerator;
        den2 = r2->denominator;
    } else {
        throw RuntimeError("Numeric comparison expects a number");
    }
    #undef NumericType
    #define NumericType long long
    NumericType left_product = num1 * den2;
    NumericType right_product = num2 * den1;
    #undef NumericType
    if (left_product < right_product) {
        return -1; // v1 < v2
    } else if (left_product > right_product) {
        return 1; // v1 > v2
    } else {
        return 0; // v1 == v2
    }
}

Value Less::evalRator(const Value &rand1, const Value &rand2) { // <
    int cmp_result = compareNumericValues(rand1, rand2);
    return BooleanV(cmp_result == -1);
}

Value LessEq::evalRator(const Value &rand1, const Value &rand2) { // <=
    int cmp_result = compareNumericValues(rand1, rand2);
    return BooleanV(cmp_result <= 0);
}

Value Equal::evalRator(const Value &rand1, const Value &rand2) { // =
    int cmp_result = compareNumericValues(rand1, rand2);
    return BooleanV(cmp_result == 0);
}

Value GreaterEq::evalRator(const Value &rand1, const Value &rand2) { // >=
    int cmp_result = compareNumericValues(rand1, rand2);
    return BooleanV(cmp_result >= 0);
}

Value Greater::evalRator(const Value &rand1, const Value &rand2) { // >
    int cmp_result = compareNumericValues(rand1, rand2);
    return BooleanV(cmp_result == 1);
}

Value LessVar::evalRator(const std::vector<Value> &args) { // < with multiple args
    if (args.size() < 2) {
        throw RuntimeError("'< ' expects at least two arguments");
    }
    for (size_t i = 0; i < args.size() - 1; ++i) {
        if (compareNumericValues(args[i], args[i+1]) >= 0) {
            return BooleanV(false);
        }
    }
    return BooleanV(true);
}

Value LessEqVar::evalRator(const std::vector<Value> &args) { // <= with multiple args
    if (args.size() < 2) {
        throw RuntimeError("'<=' expects at least two arguments");
    }
    for (size_t i = 0; i < args.size() - 1; ++i) {
        if (compareNumericValues(args[i], args[i+1]) > 0) {
            return BooleanV(false);
        }
    }
    return BooleanV(true);
}

Value EqualVar::evalRator(const std::vector<Value> &args) { // = with multiple args
    if (args.size() < 2) {
        throw RuntimeError("'= ' expects at least two arguments");
    }
    for (size_t i = 0; i < args.size() - 1; ++i) {
        if (compareNumericValues(args[i], args[i+1]) != 0) {
            return BooleanV(false);
        }
    }
    return BooleanV(true);
}

Value GreaterEqVar::evalRator(const std::vector<Value> &args) { // >= with multiple args
    if (args.size() < 2) {
        throw RuntimeError("'>=' expects at least two arguments");
    }
    for (size_t i = 0; i < args.size() - 1; ++i) {
        if (compareNumericValues(args[i], args[i+1]) < 0) {
            return BooleanV(false);
        }
    }
    return BooleanV(true);
}

Value GreaterVar::evalRator(const std::vector<Value> &args) { // > with multiple args
    if (args.size() < 2) {
        throw RuntimeError("'> ' expects at least two arguments");
    }
    for (size_t i = 0; i < args.size() - 1; ++i) {
        if (compareNumericValues(args[i], args[i+1]) <= 0) {
            return BooleanV(false);
        }
    }
    return BooleanV(true);
}

Value Cons::evalRator(const Value &rand1, const Value &rand2) { // cons
    return PairV(rand1, rand2);
}

Value ListFunc::evalRator(const std::vector<Value> &args) { // list function
    auto res = NullV();
    for (int i = args.size() - 1; i >= 0; --i) {
        res = PairV(args[i], res);
    }
    return res;
}

Value IsList::evalRator(const Value &rand) { // list?
    auto cur = rand;
    while (true) {
        if (cur->v_type == V_NULL) {
            return BooleanV(true);
        } else if (cur->v_type == V_PAIR) {
            auto p = dynamic_cast<Pair*>(cur.get());
            cur = p->cdr;
        } else {
            return BooleanV(false);
        }
        
    }
}

Value Car::evalRator(const Value &rand) { // car
    if (rand->v_type != V_PAIR) {
        throw RuntimeError("expects argument to be a pair");
    }
    auto p = dynamic_cast<Pair*>(rand.get());
    return p->car;
}

Value Cdr::evalRator(const Value &rand) { // cdr
    if (rand->v_type != V_PAIR) {
        throw RuntimeError("expects argument to be a pair");
    }
    auto p = dynamic_cast<Pair*>(rand.get());
    return p->cdr;
}

Value SetCar::evalRator(const Value &rand1, const Value &rand2) { // set-car!
    //TODO: To complete the set-car! logic
}

Value SetCdr::evalRator(const Value &rand1, const Value &rand2) { // set-cdr!
   //TODO: To complete the set-cdr! logic
}

Value IsEq::evalRator(const Value &rand1, const Value &rand2) { // eq?
    // 检查类型是否为 Integer
    if (rand1->v_type == V_INT && rand2->v_type == V_INT) {
        return BooleanV((dynamic_cast<Integer*>(rand1.get())->n) == (dynamic_cast<Integer*>(rand2.get())->n));
    }
    // 检查类型是否为 Boolean
    else if (rand1->v_type == V_BOOL && rand2->v_type == V_BOOL) {
        return BooleanV((dynamic_cast<Boolean*>(rand1.get())->b) == (dynamic_cast<Boolean*>(rand2.get())->b));
    }
    // 检查类型是否为 Symbol
    else if (rand1->v_type == V_SYM && rand2->v_type == V_SYM) {
        return BooleanV((dynamic_cast<Symbol*>(rand1.get())->s) == (dynamic_cast<Symbol*>(rand2.get())->s));
    }
    // 检查类型是否为 Null 或 Void
    else if ((rand1->v_type == V_NULL && rand2->v_type == V_NULL) ||
             (rand1->v_type == V_VOID && rand2->v_type == V_VOID)) {
        return BooleanV(true);
    } else {
        return BooleanV(rand1.get() == rand2.get());
    }
}

Value IsBoolean::evalRator(const Value &rand) { // boolean?
    return BooleanV(rand->v_type == V_BOOL);
}

Value IsFixnum::evalRator(const Value &rand) { // number?
    return BooleanV(rand->v_type == V_INT);
}

Value IsNull::evalRator(const Value &rand) { // null?
    return BooleanV(rand->v_type == V_NULL);
}

Value IsPair::evalRator(const Value &rand) { // pair?
    return BooleanV(rand->v_type == V_PAIR);
}

Value IsProcedure::evalRator(const Value &rand) { // procedure?
    return BooleanV(rand->v_type == V_PROC);
}

Value IsSymbol::evalRator(const Value &rand) { // symbol?
    return BooleanV(rand->v_type == V_SYM);
}

Value IsString::evalRator(const Value &rand) { // string?
    return BooleanV(rand->v_type == V_STRING);
}

Value Begin::eval(Assoc &e) {
    if (es.empty()) {
        return VoidV();
    }

    Value res = VoidV();
    for (const auto& expr: es) {
        res = expr->eval(e);
    }
    return res;
}

// helper function to convert Syntax to Value for quote
Value convertSyntaxToValue(const Syntax& syntax) {
    if (auto num = dynamic_cast<Number*>(syntax.get())) {
        return IntegerV(num->n);
    } else if (auto rational = dynamic_cast<RationalSyntax*>(syntax.get())) {
        return RationalV(rational->numerator, rational->denominator);
    } else if (auto str = dynamic_cast<StringSyntax*>(syntax.get())) {
        return StringV(str->s);
    } else if (auto sym = dynamic_cast<SymbolSyntax*>(syntax.get())) {
        return SymbolV(sym->s);
    } else if (auto true_syntax = dynamic_cast<TrueSyntax*>(syntax.get())) {
        return BooleanV(true);
    } else if (auto false_syntax = dynamic_cast<FalseSyntax*>(syntax.get())) {
        return BooleanV(false);
    } else if (auto list_syntax = dynamic_cast<List*>(syntax.get())) {
        int dot_index = -1;
        for (int i = 0; i < list_syntax->stxs.size(); ++i) {
            if (auto sym_elem = dynamic_cast<SymbolSyntax*>(list_syntax->stxs[i].get())) {
                if (sym_elem->s == ".") {
                    dot_index = i;
                    break;
                }
            }
        }
        if (dot_index != -1) { // (A B . C)
            // NOT (. A B C)
            if (dot_index == 0) {
                throw RuntimeError("quote: malformed dotted list (dot cannot be the first element)");
            }
            // NOT (A . B C) or (A .)
            if (list_syntax->stxs.size() - dot_index != 2) {
                throw RuntimeError("quote: malformed dotted list (dot must be followed by exactly one element)");
            }
            Value result_cdr = convertSyntaxToValue(list_syntax->stxs[dot_index + 1]);
            
            // (A B . C) ->  (A . (B . C))
            Value current_list = result_cdr;
            for (int i = dot_index - 1; i >= 0; --i) {
                Value element = convertSyntaxToValue(list_syntax->stxs[i]);
                current_list = PairV(element, current_list);
            }
            return current_list;
        } else {
            // (A B C)
            Value result = NullV();
            for (int i = list_syntax->stxs.size() - 1; i >= 0; --i) {
                Value element = convertSyntaxToValue(list_syntax->stxs[i]);
                result = PairV(element, result);
            }
            return result;
        }
    }

    throw RuntimeError("Unknown syntax type in quote");
}

Value Quote::eval(Assoc& e) {
    return convertSyntaxToValue(s);
}

Value AndVar::eval(Assoc &e) { // and with short-circuit evaluation
    if (rands.empty()) {
        return BooleanV(true);
    }
    Value last = BooleanV(true);
    for (const auto& rand : rands) {
        Value cur = rand->eval(e);
        if (cur->v_type == V_BOOL) {
            auto b_val = static_cast<Boolean*>(cur.get());
            if (b_val->b == false) {
                return BooleanV(false);
            }
        }
        last = cur;
    }
    return last;
}

Value OrVar::eval(Assoc &e) { // or with short-circuit evaluation
    if (rands.empty()) {
        return BooleanV(false);
    }
    Value last = BooleanV(true);
    for (const auto& rand : rands) {
        Value cur = rand->eval(e);
        if (!(cur->v_type == V_BOOL && dynamic_cast<Boolean*>(cur.get())->b == false)) {
            return cur;
        }
    }
    return BooleanV(false);
}

Value Not::evalRator(const Value &rand) { // not
    if (rand->v_type == V_BOOL) {
        auto b_val = static_cast<Boolean*>(rand.get());
        if (b_val->b == false) {
            return BooleanV(true);
        }
    }
    return BooleanV(false);
}

Value If::eval(Assoc &e) {
    Value cond_val = cond->eval(e);
    bool is_false = false;
    if (cond_val->v_type == V_BOOL) {
        auto b_val = static_cast<Boolean*>(cond_val.get());
        if (!b_val->b) {
            is_false = true;
        }
    }
    if (!is_false) {
        return conseq->eval(e);
    } else {
        return alter->eval(e);
    }
}

Value Cond::eval(Assoc &env) {
    // TODO

}

Value Lambda::eval(Assoc &env) { 
    return ProcedureV(x, e, env);
}

Value Apply::eval(Assoc &e) {
    auto rator_val = rator->eval(e);
    if (rator_val->v_type != V_PROC) {throw RuntimeError("Attempt to apply a non-procedure");}

    Procedure* clos_ptr = dynamic_cast<Procedure*>(rator_val.get());
    
    std::vector<Value> args;
    for (const auto& expr : rand) {
        args.push_back(expr->eval(e));
    }
    if (args.size() != clos_ptr->parameters.size()) {throw RuntimeError("Wrong number of arguments");}

    Assoc param_env = clos_ptr->env;
    for (size_t i = 0; i < args.size(); ++i) {
        param_env = extend(clos_ptr->parameters[i], args[i], param_env);
    }

    return clos_ptr->e->eval(param_env);
}

Value Define::eval(Assoc &env) {
    Assoc rec_env = env;
    rec_env = extend(var, Value(nullptr), rec_env);
    modify(var, e->eval(rec_env), rec_env);
    env = rec_env;
    return VoidV();
}

Value Let::eval(Assoc &env) {
    Assoc let_env = env;
    for (const auto& binding : bind) {
        let_env = extend(binding.first, binding.second->eval(env), let_env);
    }
    return body->eval(let_env);
}

Value Letrec::eval(Assoc &env) {
    Assoc rec_env = env;
    for (const auto& binding : bind) {
        rec_env = extend(binding.first, Value(nullptr), rec_env);
    }

    for (const auto& binding : bind) {
        auto val = binding.second->eval(rec_env);
        modify(binding.first, val, rec_env);
    }

    return body->eval(rec_env);
}

Value Set::eval(Assoc &env) {
    modify(var, e->eval(env), env);
    return VoidV();
}

Value Display::evalRator(const Value &rand) { // display function
    if (rand->v_type == V_STRING) {
        String* str_ptr = dynamic_cast<String*>(rand.get());
        std::cout << str_ptr->s;
    } else {
        rand->show(std::cout);
    }
    puts("");
    return VoidV();
}
