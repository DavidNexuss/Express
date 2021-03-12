#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <sstream>
using namespace std;


inline string double_to_string(double v)
{
    ostringstream s;
    s << v;
    return s.str();
}
struct Value : public vector<double>
{
    string str;

    Value(const std::string& _str) : str(_str) { }
    Value(double value) { push_back(value); }
    Value() : Value(0.0) { }

    Value(const std::initializer_list<double>& l) : vector<double>(l) { }
    Value(const vector<double>& v) : vector<double>(v) { }

    template <typename ... T>
    Value(const T&& ... args) : vector<double>(args...) { }

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

    double& operator[](const size_t i) { return at(i); }

    bool is_string() const { return !str.empty(); }

    string as_string() const { return str; }

    operator string() const
    { 
        if (is_string()) return str;
        string result;
        if (is_numeric()) result += double_to_string((*this)[0]);
        else
        {
            result += "(";
            for (size_t i = 0; i < size(); i++)
            {
                result += double_to_string((*this)[i]);
                if (i < size() - 1) result += ", ";
            }
            result += ")";
        }
        return result;
    }

    inline bool is_vector() const { return !is_string() && size() > 1; } 
};

#define MASTER_OPERATOR(op,op2) \
inline Value& operator op2 (Value& v,const Value& other) \
{ \
    if (v.is_numeric()) v.resize(other.size(),v[0]); \
    for (size_t i = 0; i < v.size(); ++i) v[i] op2 other[i]; \
    return v; \
} \
inline Value operator op (const Value& v,const Value& other) \
{ \
    Value result = v; \
    result op2 other; \
    return result; \
}

MASTER_OPERATOR(+,+=)
MASTER_OPERATOR(-,-=)
MASTER_OPERATOR(/,/=)
MASTER_OPERATOR(*,*=)

#undef MASTER_OPERATOR

//Verbatim for ^ operator
inline Value& operator ^= (Value& v,const Value& other)
{
    if (v.is_numeric()) v.resize(other.size(),v[0]);
    for (size_t i = 0; i < v.size(); ++i) v[i] = pow(v[i],other[i]);
    return v;
}
inline Value operator ^ (const Value& v,const Value& other)
{
    Value result = v;
    result ^= other;
    return result;
}


inline std::ostream& operator<<(std::ostream& os,const Value& v)
{
    os << string(v);
    return os;
}

inline Value sum(const Value& v)
{
    Value result = v[0];
    for (size_t i = 1; i < v.size();i++) result += v[i];

    return result;
}

inline Value prod(const Value& v)
{
    Value result = v[0];
    for (size_t i = 1; i < v.size();i++) result *= v[i];

    return result;
}
