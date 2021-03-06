////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Project:  Embedded Learning Library (ELL)
//  File:     VectorOperations.h (value)
//  Authors:  Kern Handa
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <utilities/include/MemoryLayout.h>

#include <functional>

namespace ell
{
namespace value
{

    class Vector;
    class Scalar;

    Scalar Norm0(Vector);

    Scalar Norm1(Vector);

    Scalar Norm2(Vector);

    Scalar Norm2Squared(Vector);

    Scalar Aggregate(Vector, std::function<Scalar(Scalar)>);

    Vector Transform(Vector, std::function<Scalar(Scalar)>);

    Scalar Dot(Vector, Vector);

    Scalar Accumulate(Vector input, Scalar initialValue);

    /// <summary> Creates a for loop over the vector </summary>
    /// <param name="vector"> The instance of Vector that references the data over which to iterate </param>
    /// <param name="fn"> The function to be called for each coordinate where there is an active element </param>
    void For(Vector vector, std::function<void(Scalar)> fn);

    Vector operator+(Scalar s, Vector v);
    Vector operator+(Vector v, Scalar s);
    Vector operator+(Vector v1, Vector v2);

    Vector operator-(Scalar s, Vector v);
    Vector operator-(Vector v, Scalar s);
    Vector operator-(Vector v1, Vector v2);

    Vector operator*(Scalar s, Vector v);
    Vector operator*(Vector v, Scalar s);

    Vector operator/(Scalar s, Vector v);
    Vector operator/(Vector v, Scalar s);

} // namespace value
} // namespace ell
