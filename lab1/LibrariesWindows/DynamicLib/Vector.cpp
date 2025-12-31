#include "pch.h"

#define VECTOR_EXPORTS
#include "Vector.h"
#include "Number.h"
#include <cmath>
#include <iostream>

static unsigned int vectorCount = 0;

Vector::Vector() {
    x = new Number(0.0);
    y = new Number(0.0);
    vectorCount++;
}

Vector::Vector(const Number& xVal, const Number& yVal) {
    x = new Number(xVal);
    y = new Number(yVal);
    vectorCount++;
}

Vector::Vector(const Vector& other) {
    x = new Number(*other.x);
    y = new Number(*other.y);
    vectorCount++;
}

Vector::~Vector() {
    delete x;
    delete y;
    vectorCount--;
}

Number Vector::calculateRadius() const {
    Number xSquared = *x * *x;
    Number ySquared = *y * *y;
    Number sum = xSquared + ySquared;
    return Number(sqrt(sum.getValue()));
}

Number Vector::calculateAngle() const {
    return Number(atan2(y->getValue(), x->getValue()));
}

Number Vector::getX() const {
    return *x;
}

Number Vector::getY() const {
    return *y;
}

void Vector::setX(const Number& xVal) {
    *x = xVal;
}

void Vector::setY(const Number& yVal) {
    *y = yVal;
}

void Vector::print() const {
    std::cout << "(" << x->getValue() << ", " << y->getValue() << ")";
}

void Vector::printPolar() const {
    std::cout << "[r=" << calculateRadius().getValue()
        << ", angle=" << calculateAngle().getValue() << " rad]";
}

Vector ZERO_VECTOR(Number(0.0), Number(0.0));
Vector ONE_VECTOR(Number(1.0), Number(1.0));

extern "C" VECTOR_API Vector* AddVectors(const Vector* v1, const Vector* v2) {
    if (v1 == nullptr || v2 == nullptr) {
        return nullptr;
    }

    Number sumX = v1->getX() + v2->getX();
    Number sumY = v1->getY() + v2->getY();

    return new Vector(sumX, sumY);
}

extern "C" VECTOR_API double CalculateVectorLength(const Vector* vec) {
    if (vec == nullptr) {
        return 0.0;
    }
    return vec->calculateRadius().getValue();
}