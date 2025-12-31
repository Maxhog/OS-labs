#ifndef VECTOR_H
#define VECTOR_H

#ifdef VECTOR_EXPORTS
#define VECTOR_API __attribute__((visibility("default")))
#else
#define VECTOR_API
#endif

class Number;

class Vector {
private:
    Number* x;
    Number* y;

public:
    Vector();
    Vector(const Number& xVal, const Number& yVal);
    Vector(const Vector& other);
    ~Vector();

    Number calculateRadius() const;
    Number calculateAngle() const;

    Number getX() const;
    Number getY() const;
    void setX(const Number& xVal);
    void setY(const Number& yVal);

    void print() const;
    void printPolar() const;
};

extern "C" {
    VECTOR_API Vector* AddVectors(const Vector* v1, const Vector* v2);
    VECTOR_API double CalculateVectorLength(const Vector* vec);
}

#endif // VECTOR_H
