/** @file
  Biblioteka kalkulatora wielomianów

  @authors Michał Molas
  <mm429570@students.mimuw.edu.pl>
  @copyright Uniwersytet Warszawski
  @date 2021
*/

#ifndef __CALC_H__
#define __CALC_H__

#include "poly.h"
#include "stack.h"
#include <stdbool.h>

/**
 * Wypisuje wielomian @f$p@f$ w postaci.
 * - jednomian: @f$(\mathrm{wielomian}, \mathrm{wykładnik})@f$
 * - wielomian: wartość współczynnika lub
 * @f$\mathrm{jednomian}+\mathrm{jednomian}+\dots+\mathrm{jednomian}@f$
 * - wykładnik: wartość wykładnika
 * @param[in] p : wielomian @f$p@f$
 */
void PolyPrint(const Poly *p);

/**
 * Wykonuje instrukcję ZERO, czyli dodaje na stos @p s wielomian zerowy.
 * @param[out] s : stos wielomianów
 */
void InstZero(Stack *s);

/**
 * Wykonuje instrukcję IS_COEFF, czyli sprawdza, czy wielomian z wierzchołka
 * stosu @p s jest typu coeff i wypisuje 1, gdy tak jest, a 0 wpp.
 * @param[in] s : stos wielomianów
 */
void InstIsCoeff(const Stack *s);

/**
 * Wykonuje instrukcję IS_ZERO, czyli sprawdza, czy wielomian z wierzchołka
 * stosu @p s jest tożsamościowo równy 0 i wypisuje 1, gdy tak jest, a 0 wpp.
 * @param[in] s : stos wielomianów
 */
void InstIsZero(const Stack *s);

/**
 * Wykonuje instrukcję CLONE, czyli kopiuje wielomian z wierzchołka stosu @p s i
 * dodaje go na stos.
 * @param[in,out] s : stos wielomianów
 */
void InstClone(Stack *s);

/**
 * Wykonuje instrukcję ADD, czyli bierze dwa wielomiany z wierzchołka stosu @p s
 * i dodaje na stos ich sumę.
 * @param[in,out] s : stos wielomianów
 */
void InstAdd(Stack *s);

/**
 * Wykonuje instrukcję MUL, czyli bierze dwa wielomiany z wierzchołka stosu @p s
 * i dodaje na stos ich iloczyn.
 * @param[in,out] s : stos wielomianów
 */
void InstMul(Stack *s);

/**
 * Wykonuje instrukcję NEG, czyli neguje wielomianu z wierzchołka stosu @p s.
 * @param[in,out] s : stos wielomianów
 */
void InstNeg(Stack *s);

/**
 * Wykonuje instrukcję SUB, czyli bierze dwa wielomiany z wierzchołka stosu @p s
 * i dodaje na stos ich różnicę.
 * @param[in,out] s : stos wielomianów
 */
void InstSub(Stack *s);

/**
 * Wykonuje instrukcję IS_EQ, czyli sprawdza, czy dwa wielomiany z wierzchołka
 * stosu @p s są równe i wypisuje 1, gdy tak jest, a 0 wpp.
 * @param[in] s : stos wielomianów
 */
void InstIsEq(const Stack *s);

/**
 * Wykonuje instrukcję DEG, czyli oblicza stopień wielomianu z wierzchołka stosu
 * @p s i go wypisuje.
 * @param[in] s : stos wielomianów
 */
void InstDeg(const Stack *s);

/**
 * Wykonuje instrukcję DEG_BY, czyli oblicza stopień wielomianu z wierzchołka
 * stosu @p s po zmiennej @f$x_{\mathrm{idx}}@f$ i go wypisuje.
 * @param[in] s : stos wielomianów
 * @param[in] idx : indeks zmiennej
 */
void InstDegBy(const Stack *s, size_t idx);

/**
 * Wykonuje instrukcję AT, czyli bierze wielomian z wierzchołka stosu @p s,
 * oblicza jego wartość w @f$x_0=x@f$ i wstawia uzyskany wielomian na stos.
 * @param[in,out] s : stos wielomianów
 * @param[in] x : wartość podstawianej zmiennej
 */
void InstAt(Stack *s, poly_coeff_t x);

/**
 * Wykonuje instrukcję PRINT, czyli wypisuje wielomian z wierzchołka stosu @p s.
 * @param[in] s : stos wielomianów
 */
void InstPrint(const Stack *s);

/**
 * Wykonuje instrukcję POP, czyli usuwa wielomian z wierzchołka stosu @p s.
 * @param[out] s : stos wielomianów
 */
void InstPop(Stack *s);

/**
 * Wykonuje instrukcję COMPOSE, czyli składa wielomian z wierzchołka stosu @p s
 * z @p k kolejnymi wielomianami ze stosu. Szczegóły tego działąnia są opisane w
 * dokumentacji funkcji ::PolyCompose.
 * @param[out] s : stos wielomianów
 * @param[in] k : liczba wielomianów do złożenia
 */
void InstCompose(Stack *s, size_t k);

/**
 * Wczytuje następną linię ze standardowego wejścia i wykonuje zadaną w niej
 * instrukcję lub wypisuje błąd na wyjście diagnostyczne, jeśli jest ona
 * niepoprawna
 * @param[in,out] stack: stos wielomianów
 * @param[in] index: numer sprawdzanej linii (poczynając od 1)
 * @return czy są jeszcze dane na wejściu
 */
bool ParseAndExecuteLine(Stack *stack, size_t index);

#endif