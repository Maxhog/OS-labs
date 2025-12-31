#include "Number.h"
#include <stdexcept>
#include <cmath>

Number::Number() : value(0.0) {}
Number::Number(double val) : value(val) {}
Number::Number(const Number& other) : value(other.value) {}

Number Number::operator+(const Number& other) const {
    return Number(value + other.value);
}

Number Number::operator-(const Number& other) const {
    return Number(value - other.value);
}

Number Number::operator*(const Number& other) const {
    return Number(value * other.value);
}

Number Number::operator/(const Number& other) const {
    if (other.value == 0.0) {
        throw std::runtime_error("Division by zero");
    }
    return Number(value / other.value);
}

Number& Number::operator=(const Number& other) {
    if (this != &other) {
        value = other.value;
    }
    return *this;
}

Number& Number::operator=(double val) {
    value = val;
    return *this;
}

double Number::getValue() const {
    return value;
}

void Number::setValue(double val) {
    value = val;
}

Number Number::power(double exponent) const {
    return Number(pow(value, exponent));
}

Number ZERO(0.0);
Number ONE(1.0);

extern "C" Number* CreateNumber(double value) {
    return new Number(value);
}

extern "C" double CalculatePower(Number* num, double exponent) {
    return num->power(exponent).getValue();
}