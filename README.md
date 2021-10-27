# About program

This program implements operations on polynomials of multiple variables.
The polynomials are defined such that a polynomial are either an array of monomials or a number and monomial contains a polynomial as coefficient and an exponent.
Each level of recursion has it's own different variable. 

Second part of this program is a terminal calculator of such polynomials, described below.

## Operations

Standard operations avaliable in w module poly.h:
 - addition: PolyAdd
 - subtraction: PolySub
 - multiplication: PolyMul
 - negation: PolyNeg
 - finding a degree of a certain variable: PolyDegBy
 - finding maximal degree: PolyDeg
 - checking equality of 2 polynomials: PolyIsEq
 - calculating value of a polynomial at given first variable: PolyAt
 - creating a polynomial from an array of monomials: PolyAddMonos
 - deep copy of a polynomial: PolyClone
 - deleting a polynomial: PolyDestroy
 - composing polynomials: PolyCompose

None of these operations modify given polynomials.

In module poly_lib.h there are also other operations on polynomials used in poly.h.

All information about every function can be found in doxygen (only in Polish yet).

## How to use?

The example of usage is shown in poly_example.c.
I recommend using macros P and C from that file.
The C macro creates a polynomial that is a number.
The P macro takes pairs of arguments - polynomial (eg. C or P) and an exponent and makes a polynomial from them.
Eg. P(C(2), 3, P(C(4), 5, C(8), 9), 6) returns such polynomial: @f$ 2x_0^3 + (4x_1^5 + 8x_1^9)x_0^6 @f$.

WARNINGS: 
- The polynomial should not be created differently than with functions PolyAddMonos and PolyFromCoeff, as it is very likely to cause memory problems if done improperly. 
- The PolyAddMonos function does free the elements of a given array, but it doesn't free the array itself.

## Implementation

Monomials in a polynomial ar stored in a dynamic array, with extra space doubled if needed.
It helps for example in removing zero monomials from a polynomial without extra allocations.
Polynomials created by all functions are always simplified (meaning of simplified is explained in documentation of function PolySimplify), 
so if you would like to extend the library, make sure the created polynomials are in simplified form. 
All of the standard functions perform in complexity O(number of polynomials), except for multiplication which is quadratic.

## Polynomial calculator
Another feature of this program is a calculator of polynomials, reading from standard input, putting given polynomials on a stack.
Avaliable operations:
- ZERO
- IS_COEFF
- IS_ZERO
- IS_EQ
- ADD
- SUB
- MUL
- NEG
- AT
- DEG
- DEG_BY [variable_index]
- AT [value_of_variable]
- CLONE
- POP
- PRINT
- COMPOSE [number_of_composed_polynomials]

The names suggest what each command is doing, however details of each operations are in documentation of calc.h
In order to add a polynomial to a stack you need to write it in such form (without spaces): 
- polynomial: monomial+...+monomial OR number_coefficient
- monomial: (polynomial,exponent)

In case of a wrong input or incorrect data, the error information will appear.

The calculator is executed by main function in calc.c

## Compilation
Compile with such commands using cmake (on linux):

** release version **
mkdir release
cd release
cmake ..

** debug version **
mkdir debug
cd debug
cmake -D CMAKE_BUILD_TYPE=Debug ..

** creating executables and/or documentation **
- calculator executable (poly): make
- doxygen documentation: make doc
- tests of standard functions (poly_test): make test 

