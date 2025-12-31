#ifndef VECTOR_H
#define VECTOR_H

#ifdef VECTOR_EXPORTS
#define VECTOR_API __declspec(dllexport)
#else
#define VECTOR_API __declspec(dllimport)
#endif

class Number;

class VECTOR_API Vector {
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

extern VECTOR_API Vector ZERO_VECTOR;
extern VECTOR_API Vector ONE_VECTOR;

extern "C" VECTOR_API Vector* AddVectors(const Vector* v1, const Vector* v2);
extern "C" VECTOR_API double CalculateVectorLength(const Vector* vec);

#endif // VECTOR_H