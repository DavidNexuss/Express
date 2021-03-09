#pragma once
#include <vector>
#include <iostream>
#include <cmath>
using namespace std;

struct Value : public vector<double>
{
    string str;

    Value(const std::string& _str) : str(_str) { }
    Value(double value) { push_back(value); }
    Value() : Value(0.0) { }

    Value(const std::initializer_list<double>& l) : vector<double>(l) { }
    Value(const vector<double>& v) : vector<double>(v) { }

    bool is_numeric() const { return size() == 1; }

    double& at(size_t i)
    {
        if (is_numeric()) return vector<double>::at(0);
        else return vector<double>::at(i);
    }

    const double& operator[](const size_t i) const
    {
        vector<double>& c = *((vector<double>*)this);
        if (is_numeric()) return c[0];
        else return c[i];
    }

    double& operator[](const size_t i)
    {
        return at(i);
    }

    bool is_string() const { return !str.empty(); }

    string as_string() const { return str; }
};

#define MASTER_OPERATOR(op,op2) \
Value& operator op2 (Value& v,const Value& other) \
{ \
    if (v.is_numeric()) v.resize(other.size(),v[0]); \
    for (size_t i = 0; i < v.size(); ++i) v[i] op2 other[i]; \
    return v; \
} \
Value operator op (const Value& v,const Value& other) \
{ \
    Value result = v; \
    result op2 other; \
    return result; \
}

MASTER_OPERATOR(+,+=)
MASTER_OPERATOR(-,-=)
MASTER_OPERATOR(/,/=)
MASTER_OPERATOR(*,*=)

//Verbatim for ^ operator
Value& operator ^= (Value& v,const Value& other)
{
    if (v.is_numeric()) v.resize(other.size(),v[0]);
    for (size_t i = 0; i < v.size(); ++i) v[i] = pow(v[i],other[i]);
    return v;
}
Value operator ^ (const Value& v,const Value& other)
{
    Value result = v;
    result ^= other;
    return result;
}

#undef MASTER_OPERATOR
std::ostream& operator<<(std::ostream& os,const Value& v)
{
    if (v.is_numeric()) os << v[0];
    else
    {
        os << "{";
        for (size_t i = 0; i < v.size(); i++)
        {
            os << v[i];
            if (i < v.size() - 1) os << ", ";
        }
        os << "}";
    }
    return os;
}

Value sum(const Value& v)
{
    Value result = v[0];
    for (size_t i = 1; i < v.size();i++)
    {
        result += v[i];
    }
    return result;
}

Value prod(const Value& v)
{
    Value result = v[0];
    for (size_t i = 1; i < v.size();i++)
    {
        result *= v[i];
    }
    return result;
}
