/**
 * @file parser.cpp
 * @brief Parsing implementation for Scheme syntax tree to expression tree conversion
 * 
 * This file implements the parsing logic that converts syntax trees into
 * expression trees that can be evaluated.
 * primitive operations, and function applications.
 */

#include "RE.hpp"
#include "Def.hpp"
#include "syntax.hpp"
#include "value.hpp"
#include "expr.hpp"
#include "utils.hpp"
#include <map>
#include <string>
#include <iostream>

#define mp make_pair
using std::string;
using std::vector;
using std::pair;

extern std::map<std::string, ExprType> primitives;
extern std::map<std::string, ExprType> reserved_words;

/**
 * @brief Default parse method (should be overridden by subclasses)
 */
Expr Syntax::parse(Assoc &env) {
    throw RuntimeError("Unimplemented parse method");
}

Expr Number::parse(Assoc &env) {
    return Expr(new Fixnum(n));
}

Expr RationalSyntax::parse(Assoc &env) {
    return Expr(new RationalNum(numerator, denominator));
}

Expr SymbolSyntax::parse(Assoc &env) {
    if (!util::is_valid_variable_name(s)) {
        throw RuntimeError("Invalid variable name: " + s);
    }
    return Expr(new Var(s));
}

Expr StringSyntax::parse(Assoc &env) {
    return Expr(new StringExpr(s));
}

Expr TrueSyntax::parse(Assoc &env) {
    return Expr(new True());
}

Expr FalseSyntax::parse(Assoc &env) {
    return Expr(new False());
}

Expr List::parse(Assoc &env) {
    if (stxs.empty()) {
        return Expr(new Quote(Syntax(new List())));
    }

    //TODO: check if the first element is a symbol
    //If not, use Apply function to package to a closure;
    //If so, find whether it's a variable or a keyword;
    SymbolSyntax *id = dynamic_cast<SymbolSyntax*>(stxs[0].get());
    if (id == nullptr) {
        std::vector<Expr> rands;
        for (size_t i = 1; i < stxs.size(); ++i) {
            rands.push_back(stxs[i]->parse(env));
        }
        return Expr(new Apply(stxs[0]->parse(env), rands));
    } else {
    string op = id->s;
    // if (find(op, env).get() != nullptr) {
    //     //TODO: TO COMPLETE THE PARAMETER PARSER LOGIC
    // }
    if (primitives.count(op) != 0) {
        vector<Expr> parameters;
        for (size_t i = 1; i < stxs.size(); ++i) {
            parameters.push_back(stxs[i]->parse(env));
        }
        
        ExprType op_type = primitives[op];
        switch (primitives[op]) {
            case E_PLUS:
                if (parameters.size() < 2) throw RuntimeError("+ expects at least 2 arguments");
                return parameters.size() == 2 ? Expr(new Plus(parameters[0], parameters[1])) : Expr(new PlusVar(parameters));
            case E_MINUS:
                if (parameters.size() < 1) throw RuntimeError("- expects at least 1 argument");
                 return parameters.size() == 2 ? Expr(new Minus(parameters[0], parameters[1])) : Expr(new MinusVar(parameters));
            case E_MUL:
                 if (parameters.size() < 2) throw RuntimeError("* expects at least 2 arguments");
                 return parameters.size() == 2 ? Expr(new Mult(parameters[0], parameters[1])) : Expr(new MultVar(parameters));
            case E_DIV:
                 if (parameters.size() < 1) throw RuntimeError("/ expects at least 1 argument");
                 return parameters.size() == 2 ? Expr(new Div(parameters[0], parameters[1])) : Expr(new DivVar(parameters));
            case E_MODULO:
                if (parameters.size() != 2) throw RuntimeError("modulo expects exactly 2 arguments");
                return Expr(new Modulo(parameters[0], parameters[1]));
            case E_EXPT:
                if (parameters.size() != 2) throw RuntimeError("expt expects exactly 2 arguments");
                return Expr(new Expt(parameters[0], parameters[1]));
            case E_LT:
                 if (parameters.size() < 2) throw RuntimeError("< expects at least 2 arguments");
                 return parameters.size() == 2 ? Expr(new Less(parameters[0], parameters[1])) : Expr(new LessVar(parameters));
            case E_LE:
                 if (parameters.size() < 2) throw RuntimeError("<= expects at least 2 arguments");
                 return parameters.size() == 2 ? Expr(new LessEq(parameters[0], parameters[1])) : Expr(new LessEqVar(parameters));
            case E_EQ:
                 if (parameters.size() < 2) throw RuntimeError("= expects at least 2 arguments");
                 return parameters.size() == 2 ? Expr(new Equal(parameters[0], parameters[1])) : Expr(new EqualVar(parameters));
            case E_GE:
                 if (parameters.size() < 2) throw RuntimeError(">= expects at least 2 arguments");
                 return parameters.size() == 2 ? Expr(new GreaterEq(parameters[0], parameters[1])) : Expr(new GreaterEqVar(parameters));
            case E_GT:
                 if (parameters.size() < 2) throw RuntimeError("> expects at least 2 arguments");
                 return parameters.size() == 2 ? Expr(new Greater(parameters[0], parameters[1])) : Expr(new GreaterVar(parameters));
            case E_CONS:
                if (parameters.size() != 2) throw RuntimeError("cons expects exactly 2 arguments");
                return Expr(new Cons(parameters[0], parameters[1]));
            case E_CAR:
                if (parameters.size() != 1) throw RuntimeError("car expects exactly 1 argument");
                return Expr(new Car(parameters[0]));
            case E_CDR:
                if (parameters.size() != 1) throw RuntimeError("cdr expects exactly 1 argument");
                return Expr(new Cdr(parameters[0]));
            case E_LIST: // Variadic, no argument count check needed
                return Expr(new ListFunc(parameters));
            case E_SETCAR:
                if (parameters.size() != 2) throw RuntimeError("set-car! expects exactly 2 arguments");
                return Expr(new SetCar(parameters[0], parameters[1]));
            case E_SETCDR:
                if (parameters.size() != 2) throw RuntimeError("set-cdr! expects exactly 2 arguments");
                return Expr(new SetCdr(parameters[0], parameters[1]));
            case E_NOT:
                if (parameters.size() != 1) throw RuntimeError("not expects exactly 1 argument");
                return Expr(new Not(parameters[0]));
            case E_AND:
                return Expr(new AndVar(parameters));
            case E_OR:
                return Expr(new OrVar(parameters));
            case E_EQQ:
                if (parameters.size() != 2) throw RuntimeError("eq? expects exactly 2 arguments");
                return Expr(new IsEq(parameters[0], parameters[1]));
            case E_BOOLQ:
                if (parameters.size() != 1) throw RuntimeError("boolean? expects exactly 1 argument");
                return Expr(new IsBoolean(parameters[0]));
            case E_INTQ:
                if (parameters.size() != 1) throw RuntimeError("number? expects exactly 1 argument");
                return Expr(new IsFixnum(parameters[0]));
            case E_NULLQ:
                if (parameters.size() != 1) throw RuntimeError("null? expects exactly 1 argument");
                return Expr(new IsNull(parameters[0]));
            case E_PAIRQ:
                if (parameters.size() != 1) throw RuntimeError("pair? expects exactly 1 argument");
                return Expr(new IsPair(parameters[0]));
            case E_PROCQ:
                if (parameters.size() != 1) throw RuntimeError("procedure? expects exactly 1 argument");
                return Expr(new IsProcedure(parameters[0]));
            case E_SYMBOLQ:
                if (parameters.size() != 1) throw RuntimeError("symbol? expects exactly 1 argument");
                return Expr(new IsSymbol(parameters[0]));
            case E_LISTQ:
                if (parameters.size() != 1) throw RuntimeError("list? expects exactly 1 argument");
                return Expr(new IsList(parameters[0]));
            case E_STRINGQ:
                if (parameters.size() != 1) throw RuntimeError("string? expects exactly 1 argument");
                return Expr(new IsString(parameters[0]));
            case E_DISPLAY:
                if (parameters.size() != 1) throw RuntimeError("display expects exactly 1 argument");
                return Expr(new Display(parameters[0]));
            case E_VOID:
                if (parameters.size() != 0) throw RuntimeError("void expects exactly 0 arguments");
                return Expr(new MakeVoid());
            case E_EXIT:
                if (parameters.size() != 0) throw RuntimeError("exit expects exactly 0 arguments");
                return Expr(new Exit());

            default:
                throw RuntimeError("Primitive parser not yet implemented for: " + op);
        }
    }

    if (reserved_words.count(op) != 0) {
        
    	switch (reserved_words[op]) {
			case E_BEGIN: {
                // (begin expr ...)
                std::vector<Expr> body_expr;
                for (size_t i = 1; i < stxs.size(); ++i) {
                    body_expr.push_back(stxs[i]->parse(env));
                }
                return Expr(new Begin(body_expr));
            }
            case E_QUOTE: {
                // (quote expr)
                if (stxs.size() != 2) {
                    throw RuntimeError("Wrong number of arguments for quote");
                }
                return Expr(new Quote(stxs[1]));
            }
            case E_IF: {
                // (if cond conseq alter)
                if (stxs.size() != 4) {
                    throw RuntimeError("Wrong numbrt of arguments for if");
                }
                return Expr(new If(stxs[1]->parse(env), stxs[2]->parse(env), stxs[3]->parse(env)));
            }
            case E_COND: {
                // (cond (test expr ...) ... [else expr ...])
                if (stxs.size() < 2) {
                    throw RuntimeError("Wrong numbrt of arguments for cond");
                }
                std::vector<std::vector<Expr>> clauses;
                bool is_else = false;
                for (int i = 1; i < stxs.size(); ++i) {
                    auto clause_list = dynamic_cast<List*>(stxs[i].get());
                    if (clause_list == nullptr) {
                        throw RuntimeError("Cond clause must be a list");
                    }
                    if (clause_list->stxs.empty()) {
                        throw RuntimeError("Empty clause in cond expression");
                    }
                    std::vector<Expr> clause;
                    auto first = dynamic_cast<Symbol*>(clause_list->stxs[0].get());
                    if (first && first->s == "else") {
                        is_else = true;
                        for (size_t j = 1; j < clause_list->stxs.size(); ++j) {
                            clause.push_back(clause_list->stxs[j]->parse(env));
                        }
                    } else {
                        for (const auto& stx: clause_list->stxs) {
                            clause.push_back(stx->parse(env));
                        }
                    }
                    clauses.push_back(clause);
                    if (is_else && i < stxs.size() - 1) {
                        throw RuntimeError("Else clause can only appear as the last clause in a cond expression");
                    }
                }
                return Expr(new Cond(clauses));
            }
            case E_LAMBDA: {
                if (stxs.size() < 3) {
                    throw RuntimeError("lambda: too few arguments.");
                }
                auto args_list = dynamic_cast<List*>(stxs[1].get());
                if (!args_list) {
                    throw RuntimeError("lambda: parameters must be a list.");
                }
                std::vector<std::string> parms;
                for (const auto& arg : args_list->stxs) {
                    auto name = dynamic_cast<SymbolSyntax*>(arg.get());
                    if (!name) {
                        throw RuntimeError("lambda: parameters must be symbols.");
                    }
                    parms.push_back(name->s);
                }
                if (stxs.size() == 3) {
                    return Expr(new Lambda(parms, stxs[2]->parse(env)));
                } else {
                    std::vector<Expr> body_exprs;
                    for (size_t i = 2; i < stxs.size(); ++i) {
                        body_exprs.push_back(stxs[i]->parse(env));
                    }
                    Expr body_expr_seq = Expr(new Begin(body_exprs));
                    return Expr(new Lambda(parms, body_expr_seq));
                }

            }
            case E_DEFINE: {
                // (define var expr) or (define (func-name params...) body)
                if (stxs.size() != 3) {
                    throw RuntimeError("define: wrong number of arguments");
                }

                // (define (func-name params...) body)
                auto func_list = dynamic_cast<List*>(stxs[1].get());
                if (func_list != nullptr) {
                    if (func_list->stxs.empty()) {
                        throw RuntimeError("define: function name missing");
                    }
                    
                    auto func_name = dynamic_cast<SymbolSyntax*>(func_list->stxs[0].get());
                    if (func_name == nullptr) {
                        throw RuntimeError("define: function name must be a symbol");
                    }

                    std::vector<std::string> params;
                    for (size_t i = 1; i < func_list->stxs.size(); ++i) {
                        auto param = dynamic_cast<SymbolSyntax*>(func_list->stxs[i].get());
                        if (param == nullptr) {
                            throw RuntimeError("define: parameter must be a symbol");
                        }
                        params.push_back(param->s);
                    }

                    Expr lambda_expr = Expr(new Lambda(params, stxs[2]->parse(env)));
                    
                    return Expr(new Define(func_name->s, lambda_expr));
                }
                
                // (define var expr)
                auto var_name = dynamic_cast<SymbolSyntax*>(stxs[1].get());
                if (var_name == nullptr) {
                    throw RuntimeError("define: variable name must be a symbol");
                }
                
                return Expr(new Define(var_name->s, stxs[2]->parse(env)));
            }
            case E_LET:
            case E_LETREC: {
                if (stxs.size() < 3) {
                    throw RuntimeError(op + ": bad syntax, requires bindings and a body");
                }
                auto bind_list = dynamic_cast<List*>(stxs[1].get());
                if (bind_list == nullptr) {
                    throw RuntimeError(op + ": bindings must be in a list");
                }

                std::vector<std::pair<std::string, Expr>> bind;
                for (const auto& bind_pair_stx : bind_list->stxs) {
                    auto bind_pair = dynamic_cast<List*>(bind_pair_stx.get());
                    if (bind_pair == nullptr || bind_pair->stxs.size() != 2) {
                        throw RuntimeError(op + ":each binding must be a (variable expression) pair");
                    }
                    auto var_name = dynamic_cast<SymbolSyntax*>(bind_pair->stxs[0].get());
                    if (var_name == nullptr) {
                        throw RuntimeError(op + ": variable in a binding must be a symbol");
                    }
                    auto expr = bind_pair->stxs[1]->parse(env);
                    bind.push_back({var_name->s, expr});
                }

                Expr body(nullptr);
                if (stxs.size() == 3) {
                    body = stxs[2]->parse(env);
                } else {
                    std::vector<Expr> body_exprs;
                    for (size_t i = 2; i < stxs.size(); ++i) {
                        body_exprs.push_back(stxs[i]->parse(env));
                    }
                    body = Expr(new Begin(body_exprs));
                }

                if (reserved_words[op] == E_LET) {
                    return Expr(new Let(bind, body));
                } else {
                    return Expr(new Letrec(bind, body));
                }
            }
            case E_SET: {
                // (set! var expr)
                if (stxs.size() != 3) {
                    throw RuntimeError("set!: bad syntax, requires a variable and an expression");
                }
                auto var_name = dynamic_cast<SymbolSyntax*>(stxs[1].get());
                if (var_name == nullptr) {
                    throw RuntimeError("set!: variable must be a symbol");
                }
                Expr expr = stxs[2]->parse(env);
                return Expr(new Set(var_name->s, expr));
            }
        	default:
            	throw RuntimeError("Unknown reserved word: " + op);
    	}
    }

    Expr rator = Expr(new Var(op));
    std::vector<Expr> rands;
    for (size_t i = 1; i < stxs.size(); ++i) {
        rands.push_back(stxs[i]->parse(env));
    }
    return Expr(new Apply(rator, rands));
}
}
