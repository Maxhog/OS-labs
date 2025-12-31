#ifndef NUMBER_H
#define NUMBER_H

class Number {
private:
    double value;

public:
    Number();
    Number(double val);
    Number(const Number& other);

    Number operator+(const Number& other) const;
    Number operator-(const Number& other) const;
    Number operator*(const Number& other) const;
    Number operator/(const Number& other) const;

    Number& operator=(const Number& other);
    Number& operator=(double val);

    double getValue() const;
    void setValue(double val);

    Number power(double exponent) const;
};

extern Number ZERO;
extern Number ONE;

extern "C" Number* CreateNumber(double value);
extern "C" double CalculatePower(Number* num, double exponent);

#endif // NUMBER_H