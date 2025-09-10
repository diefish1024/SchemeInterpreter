#ifndef VALUE 
#define VALUE

/**
 * @file value.hpp
 * @brief Value system and environment definitions for the Scheme interpreter
 * 
 * This file defines the value types, environment (association list) system,
 * and all related operations for the Scheme interpreter runtime.
 */

#include "Def.hpp"
#include "expr.hpp"
#include <memory>
#include <cstring>
#include <vector>

using ll = long long;

// ============================================================================
// Base classes and smart pointer wrappers
// ============================================================================

/**
 * @brief Base class for all values in the Scheme interpreter
 */
struct ValueBase {
    ValueType v_type;
    ValueBase(ValueType);
    virtual void show(std::ostream &) = 0;
    virtual void showCdr(std::ostream &);
    virtual ~ValueBase() = default;
};

/**
 * @brief Smart pointer wrapper for ValueBase objects
 */
struct Value {
    std::shared_ptr<ValueBase> ptr;
    Value(ValueBase *);
    void show(std::ostream &);
    ValueBase* operator->() const;
    ValueBase& operator*();
    ValueBase* get() const;
};

// ============================================================================
// Environment (Association Lists)
// ============================================================================

/**
 * @brief Smart pointer wrapper for AssocList (Environment)
 */
struct Assoc {
    std::shared_ptr<AssocList> ptr;
    Assoc(AssocList *);
    AssocList* operator->() const;
    AssocList& operator*();
    AssocList* get() const;
};

/**
 * @brief Association list node for variable bindings
 */
struct AssocList {
    std::string x;      ///< Variable name
    Value v;            ///< Variable value
    Assoc next;         ///< Next binding in the chain
    AssocList(const std::string &, const Value &, Assoc &);
};

// Environment operations
Assoc empty();
Assoc extend(const std::string&, const Value &, Assoc &);
void modify(const std::string&, const Value &, Assoc &);
Value find(const std::string &, Assoc &);

// ============================================================================
// Simple Value Types
// ============================================================================

/**
 * @brief Void value (represents no meaningful return value)
 */
struct Void : ValueBase {
    Void();
    virtual void show(std::ostream &) override;
};
Value VoidV();

/**
 * @brief Integer value
 */
struct Integer : ValueBase {
    ll n;
    Integer(ll);
    virtual void show(std::ostream &) override;
};
Value IntegerV(ll);

/**
 * @brief Rational number value
 */
struct Rational : ValueBase {
    ll numerator;
    ll denominator;
    Rational(ll, ll);
    virtual void show(std::ostream &) override;
};
Value RationalV(ll, ll);

/**
 * @brief Boolean value
 */
struct Boolean : ValueBase {
    bool b;
    Boolean(bool);
    virtual void show(std::ostream &) override;
};
Value BooleanV(bool);

/**
 * @brief Symbol value
 */
struct Symbol : ValueBase {
    std::string s;
    Symbol(const std::string &);
    virtual void show(std::ostream &) override;
};
Value SymbolV(const std::string &);

/**
 * @brief String value
 */
struct String : ValueBase {
    std::string s;
    String(const std::string &);
    virtual void show(std::ostream &) override;
};
Value StringV(const std::string &);

// ============================================================================
// Special Value Types
// ============================================================================

/**
 * @brief Null value (empty list)
 */
struct Null : ValueBase {
    Null();
    virtual void show(std::ostream &) override;
    virtual void showCdr(std::ostream &) override;
};
Value NullV();

/**
 * @brief Termination signal value
 */
struct Terminate : ValueBase {
    Terminate();
    virtual void show(std::ostream &) override;
};
Value TerminateV();

// ============================================================================
// Composite Value Types
// ============================================================================

/**
 * @brief Pair value (cons cell)
 */
struct Pair : ValueBase {
    Value car;  ///< First element
    Value cdr;  ///< Second element
    Pair(const Value &, const Value &);
    virtual void show(std::ostream &) override;
    virtual void showCdr(std::ostream &) override;
};
Value PairV(const Value &, const Value &);

/**
 * @brief Procedure (function) value
 */
struct Procedure : ValueBase {
    std::vector<std::string> parameters;   ///< Parameter names
    Expr e;                                ///< Function body expression
    Assoc env;                             ///< Closure environment
    Procedure(const std::vector<std::string> &, const Expr &, const Assoc &);
    virtual void show(std::ostream &) override;
};
Value ProcedureV(const std::vector<std::string> &, const Expr &, const Assoc &);

// ============================================================================
// Utility Functions
// ============================================================================

std::ostream &operator<<(std::ostream &, Value &);

#endif // VALUE