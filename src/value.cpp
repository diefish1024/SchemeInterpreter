/**
 * @file value.cpp
 * @brief Implementation of value types and environment operations
 * 
 * This file implements all value types, their constructors, show methods,
 * and environment (association list) operations for the Scheme interpreter.
 */

#include "value.hpp"
#include "utils.hpp"
#include "RE.hpp"

// ============================================================================
// Base ValueBase Implementation
// ============================================================================

ValueBase::ValueBase(ValueType vt) : v_type(vt) {}

void ValueBase::showCdr(std::ostream &os) {
    os << " . ";
    show(os);
    os << ')';
}

// ============================================================================
// Value Smart Pointer Implementation
// ============================================================================

Value::Value(ValueBase *ptr) : ptr(ptr) {}

ValueBase* Value::operator->() const { 
    return ptr.get(); 
}

ValueBase& Value::operator*() { 
    return *ptr; 
}

ValueBase* Value::get() const { 
    return ptr.get(); 
}

void Value::show(std::ostream &os) {
    ptr->show(os);
}

// ============================================================================
// Environment (Association List) Implementation
// ============================================================================

AssocList::AssocList(const std::string &x, const Value &v, Assoc &next)
    : x(x), v(v), next(next) {}

Assoc::Assoc(AssocList *x) : ptr(x) {}

AssocList* Assoc::operator->() const { 
    return ptr.get(); 
}

AssocList& Assoc::operator*() { 
    return *ptr; 
}

AssocList* Assoc::get() const { 
    return ptr.get(); 
}

Assoc empty() {
    return Assoc(nullptr);
}

Assoc extend(const std::string &x, const Value &v, Assoc &lst) {
    return Assoc(new AssocList(x, v, lst));
}

void modify(const std::string &x, const Value &v, Assoc &lst) {
    for (auto i = lst; i.get() != nullptr; i = i->next) {
        if (x == i->x) {
            i->v = v;
            return;
        }
    }
    throw RuntimeError("undefined variable: " + x);
}

void insert(const std::string &x, const Value &v, Assoc &lst) {
    if (!lst.get()) {
        auto head = Assoc(nullptr);
        lst = extend(x, v, head);
        return;
    }
    lst->next = extend(x, v, lst->next);
}

Value find(const std::string &x, Assoc &l) {
    for (auto i = l; i.get() != nullptr; i = i->next) {
        if (x == i->x) {
            return i->v;
        }
    }
    return Value(nullptr);
}

bool bound(const std::string &name, Assoc &env) {
    for (Assoc p = env; p.get(); p = p->next) {
        if (p->x == name) return true;
    }
    return false;
}

// ============================================================================
// Simple Value Types Implementation
// ============================================================================

// Void
Void::Void() : ValueBase(V_VOID) {}

void Void::show(std::ostream &os) {
    os << "#<void>";
}

Value VoidV() {
    return Value(new Void());
}

// Integer
Integer::Integer(NumericType n) : ValueBase(V_INT), n(n) {}

void Integer::show(std::ostream &os) {
    os << n;
}

Value IntegerV(NumericType n) {
    return Value(new Integer(n));
}

Rational::Rational(NumericType num, NumericType den) : ValueBase(V_RATIONAL), numerator(num), denominator(den) {
    if (denominator == 0) {
        throw std::runtime_error("Division by zero");
    }
    if (denominator < (NumericType)0) {
        numerator = -numerator;
        denominator = -denominator;
    }
    util::normalize_rational(numerator, denominator);
}

void Rational::show(std::ostream &os) {
    if (denominator == 1) {
        os << numerator;
    } else {
        os << numerator << "/" << denominator;
    }
}

Value RationalV(NumericType num, NumericType den) {
    return Value(new Rational(num, den));
}

// Boolean
Boolean::Boolean(bool b) : ValueBase(V_BOOL), b(b) {}

void Boolean::show(std::ostream &os) {
    os << (b ? "#t" : "#f");
}

Value BooleanV(bool b) {
    return Value(new Boolean(b));
}

// Symbol
Symbol::Symbol(const std::string &s) : ValueBase(V_SYM), s(s) {}

void Symbol::show(std::ostream &os) {
    os << s;
}

Value SymbolV(const std::string &s) {
    return Value(new Symbol(s));
}

// String
String::String(const std::string &s) : ValueBase(V_STRING), s(s) {}

void String::show(std::ostream &os) {
    os << "\"" << s << "\"";
}

Value StringV(const std::string &s) {
    return Value(new String(s));
}

// ============================================================================
// Special Value Types Implementation
// ============================================================================

// Null
Null::Null() : ValueBase(V_NULL) {}

void Null::show(std::ostream &os) {
    os << "()";
}

void Null::showCdr(std::ostream &os) {
    os << ')';
}

Value NullV() {
    return Value(new Null());
}

// Terminate
Terminate::Terminate() : ValueBase(V_TERMINATE) {}

void Terminate::show(std::ostream &os) {
    os << "()";
}

Value TerminateV() {
    return Value(new Terminate());
}

// ============================================================================
// Composite Value Types Implementation
// ============================================================================

// Pair
Pair::Pair(const Value &car, const Value &cdr) 
    : ValueBase(V_PAIR), car(car), cdr(cdr) {}

void Pair::show(std::ostream &os) {
    os << '(' << car;
    cdr->showCdr(os);
}

void Pair::showCdr(std::ostream &os) {
    os << ' ' << car;
    cdr->showCdr(os);
}

Value PairV(const Value &car, const Value &cdr) {
    return Value(new Pair(car, cdr));
}

// Procedure
Procedure::Procedure(const std::vector<std::string> &xs, const Expr &e, const Assoc &env)
    : ValueBase(V_PROC), parameters(xs), e(e), env(env) {}

void Procedure::show(std::ostream &os) {
    os << "#<procedure>";
}

Value ProcedureV(const std::vector<std::string> &xs, const Expr &e, const Assoc &env) {
    return Value(new Procedure(xs, e, env));
}

// ============================================================================
// Utility Functions Implementation
// ============================================================================

std::ostream &operator<<(std::ostream &os, Value &v) {
    v->show(os);
    return os;
}
