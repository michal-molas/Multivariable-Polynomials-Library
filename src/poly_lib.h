/** @file
  Biblioteka wielomianów rzadkich wielu zmiennych

  @authors Michał Molas
  <mm429570@students.mimuw.edu.pl>
  @copyright Uniwersytet Warszawski
  @date 2021
*/

#ifndef __POLY_LIB_H__
#define __POLY_LIB_H__

#include "poly.h"
#include <assert.h>

/**
 * Sprawdza, czy alokacja się udała, jeśli nie, opuszcza program z kodem 1
 * @param[in] p : wskaźnik na zaalokowaną pamięć
 */
#define CHECK_PTR(p)                                                           \
    do                                                                         \
    {                                                                          \
        if (p == NULL)                                                         \
        {                                                                      \
            exit(1);                                                           \
        }                                                                      \
    } while (0)

/**
 * Przekształca wielomian będący coeffem @f$p = C@f$ na wielomian w postaci
 * @f$p = Cx^0@f$
 * @param[in,out] p : wielomian @f$p@f$
 */
void PolyToMonoCoeff(Poly *p);

/**
 * Dodaje wielomian @f$q@f$ do wielomianu @f$p@f$
 * @param[in,out] p : wielomian @f$p@f$
 * @param[in] q : wielomian @f$q@f$
 */
void PolyAddTo(Poly *p, const Poly *q);

/**
 * Neguje wielomian @f$p@f$
 * @param[in,out] p : wielomian @f$p@f$
 */
void PolyNegTo(Poly *p);

/**
 * Mnoży wielomian @f$p@f$ przez współczynnik @f$c@f$
 * @param[in,out] p : wielomian @f$p@f$
 * @param[in] c : współczynnik @f$c@f$
 */
void PolyMulByCoeffTo(Poly *p, poly_coeff_t c);

/**
 * Funkcja pomocnicza do PolyDeg. Szuka stopnia wielomianu @f$p@f$ ze względu na
 * zmienną @f$x_\mathrm{var_idx}@f$, zapisuje go w @p max_exp
 * @param[in] p : wielomian @f$p@f$
 * @param[in] var_idx : indeks zmiennej
 * @param[in] curr_idx : aktualny indeks
 * @param[in,out] max_exp : aktualny maksymalny stopień
 */
void PolyDegByHelp(const Poly *p, size_t var_idx, size_t curr_idx,
                   poly_exp_t *max_exp);

/**
 * Funkcja pomocnicza do PolyDeg. Szuka największego stopnia w wielomianie
 * @f$p@f$ zapisuje go w @p max_exp, jeśli znajdzie
 * @param[in] p : wielomian @f$p@f$
 * @param[in, out] max_exp : aktualny maksymalny stopień
 * @param[in] curr_exp : aktualny stopień gałęszi wielomianu @f$p@f$
 */
void PolyDegHelp(const Poly *p, poly_exp_t *max_exp, poly_exp_t curr_exp);

/**
 * Zwraca jednomian będący iloczynem jednomianów @f$m@f$ i @f$n@f$
 * @param[in] m : jednomian @f$m@f$
 * @param[in] n : jednomian @f$n@f$
 * @return @f$m \cdot n@f$
 */
Mono MonoMul(const Mono *m, const Mono *n);

/**
 * Zwraca wielomian będący iloczynem wielomianu @f$p@f$ i liczby @f$c@f$
 * @param[in] p : wielomian @f$p@f$
 * @param[in] c : liczba @f$c@f$
 * @return @f$c\cdot p@f$
 */
Poly PolyMulByCoeff(const Poly *p, poly_coeff_t c);

/**
 * Dodaje jednomian @p n do jednomianu @p m
 * @param[in,out] m : jednomian @f$m@f$
 * @param[in] n : jednomian @f$n@f$
 */
static inline void MonoAddTo(Mono *m, const Mono *n)
{
    assert(MonoGetExp(m) == MonoGetExp(n));
    PolyAddTo(&m->p, &n->p);
}

/**
 * Sortuje jednomiany malejąco po wykładnikach w złożoności
 * @f$\mathrm{O}(n*\log(n))@f$.
 * @param[in,out] monos : tablica jednomianów
 * @param[in] size : rozmiar tablicy
 */
void MonosSort(Mono *monos, size_t size);

/**
 * Liczy @f$x^\mathrm{exp}@f$ w złożoności @f$\mathrm{O}(\log(\mathrm{exp}))@f$.
 * Algorytm opiera się na fakcie, że @f$x^{2n}=x^{n}\cdot x^{n}@f$ i
 * @f$x^{2n+1}=x\cdot x^{n}\cdot x^{n}@f$
 * @param[in] x : liczba @f$x@f$
 * @param[in] exp : wykładnik @f$\mathrm{exp}@f$
 * @return @f$x^\mathrm{exp}@f$
 */
poly_coeff_t Power(poly_coeff_t x, poly_exp_t exp);

/**
 * Tworzy wielomian będący sumą jednomianów z @p monos
 * @param[in] count : liczba jednomianów
 * @param[in,out] monos : tablica jednomianów
 * @return wielomian będący sumą jednomianów z @p monos
 */
Poly PolyFromMonos(size_t count, Mono *monos);

/**
 * Tworzy wielomian będący złożeniem wielomianu @f$p@f$ i wielomianów z tablicy
 * @p q. Oznacza to, że podstawia pod kolejne zmienne wielomianu @f$p@f$ kolejne
 * wielomiany z tablicy. Wynik nadpisuje do @f$p@f$. Przejmuje na własność
 * elementy tablicy @p q
 * @param[in,out] p : wielomian @f$p@f$
 * @param[in] k : liczba wielomianów w @p q
 * @param[in,out] q : tablica podstawianych wielomianów
 */
void PolyComposeTo(Poly *p, size_t k, Poly q[]);

#endif