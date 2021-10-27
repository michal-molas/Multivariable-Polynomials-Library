/** @file
  Implementacja klasy wielomianów rzadkich wielu zmiennych

  @authors Michał Molas
  <mm429570@students.mimuw.edu.pl>
  @copyright Uniwersytet Warszawski
  @date 2021
*/

#include "poly.h"
#include "calc.h"
#include "poly_lib.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

void PolyDestroy(Poly *p)
{
    if (!PolyIsCoeff(p))
    {
        for (size_t i = 0; i < p->size; i++)
            MonoDestroy(&(p->arr[i]));

        free(p->arr);
    }
}

Poly PolyClone(const Poly *p)
{
    if (PolyIsCoeff(p))
        return PolyFromCoeff(p->coeff);

    Poly cloned_p;

    cloned_p.size = p->size;
    cloned_p.max_size = p->max_size;
    cloned_p.arr = malloc(cloned_p.max_size * sizeof(Mono));
    CHECK_PTR(cloned_p.arr);

    for (size_t i = 0; i < p->size; i++)
        cloned_p.arr[i] = MonoClone(&p->arr[i]);

    return cloned_p;
}

Poly PolyAdd(const Poly *p, const Poly *q)
{
    Poly new_poly = PolyClone(p);
    PolyAddTo(&new_poly, q);
    return new_poly;
}

Poly PolyNeg(const Poly *p)
{
    Poly new_poly = PolyClone(p);
    PolyNegTo(&new_poly);
    return new_poly;
}

Poly PolySub(const Poly *p, const Poly *q)
{
    Poly new_poly = PolyNeg(q);
    PolyAddTo(&new_poly, p);
    return new_poly;
}

bool PolyIsEq(const Poly *p, const Poly *q)
{
    bool is_p_coeff = PolyIsCoeff(p);
    bool is_q_coeff = PolyIsCoeff(q);
    if (is_p_coeff != is_q_coeff)
        return false;

    if (is_p_coeff) // obydwa są coeffami
        return p->coeff == q->coeff;

    // obydwa nie są coeffami
    if (p->size == q->size)
    {
        bool is_eq = true;
        for (size_t i = 0; i < p->size; i++)
        {
            if (p->arr[i].exp == q->arr[i].exp)
                is_eq &= PolyIsEq(&p->arr[i].p, &q->arr[i].p);
            else
                return false;
        }
        return is_eq;
    }

    return false;
}

poly_exp_t PolyDegBy(const Poly *p, size_t var_idx)
{
    if (PolyIsZero(p))
        return -1;
    poly_exp_t max_exp = 0;
    PolyDegByHelp(p, var_idx, 0, &max_exp);
    return max_exp;
}

poly_exp_t PolyDeg(const Poly *p)
{
    if (PolyIsZero(p))
        return -1;
    poly_exp_t max_exp = 0;
    PolyDegHelp(p, &max_exp, 0);
    return max_exp;
}

/**
 * Kopiuje (płytko lub głęboko, w zależności od wartości parametru @p is_deep )
 * tablicę @p monos.
 * @param[in] count : liczba jednomianów w @p monos
 * @param[in] monos : tablica jednomianów
 * @param[in] is_deep : czy głęboka kopia
 * @return skopiowana tablica @p monos
 */
static Mono *MonosCopyArray(size_t count, const Mono monos[], bool is_deep)
{
    Mono *monos_copy = malloc(count * sizeof(Mono));
    CHECK_PTR(monos_copy);

    for (size_t i = 0; i < count; i++)
    {
        if (is_deep)
            monos_copy[i] = MonoClone(monos + i);
        else
            monos_copy[i] = monos[i];
    }

    return monos_copy;
}

Poly PolyAddMonos(size_t count, const Mono monos[])
{
    if (count == 0 || monos == NULL)
        return PolyZero();

    return PolyOwnMonos(count, MonosCopyArray(count, monos, false));
}

Poly PolyOwnMonos(size_t count, Mono *monos)
{
    if (count == 0)
    {
        if (monos != NULL)
            free(monos);
        
        return PolyZero();
    }
    if (monos == NULL)
        return PolyZero();

    MonosSort(monos, count);

    return PolyFromMonos(count, monos);
}

Poly PolyCloneMonos(size_t count, const Mono monos[])
{
    if (count == 0 || monos == NULL)
        return PolyZero();

    return PolyOwnMonos(count, MonosCopyArray(count, monos, true));
}

Poly PolyMul(const Poly *p, const Poly *q)
{
    if (PolyIsCoeff(p))
        return PolyMulByCoeff(q, p->coeff);

    if (PolyIsCoeff(q))
        return PolyMulByCoeff(p, q->coeff);

    size_t monos_size = p->size * q->size;
    Mono *monos = malloc(monos_size * sizeof(Mono));
    CHECK_PTR(monos);

    for (size_t i = 0; i < p->size; i++)
        for (size_t j = 0; j < q->size; j++)
            monos[i * q->size + j] = MonoMul(&p->arr[i], &q->arr[j]);

    Poly res_poly = PolyOwnMonos(monos_size, monos);

    return res_poly;
}

Poly PolyAt(const Poly *p, poly_coeff_t x)
{
    if (PolyIsCoeff(p))
        return PolyClone(p);

    Poly *polies = malloc(p->size * sizeof(Poly));
    CHECK_PTR(polies);

    for (size_t i = 0; i < p->size; i++)
        polies[i] = PolyMulByCoeff(&p->arr[i].p, Power(x, p->arr[i].exp));

    for (size_t i = 1; i < p->size; i++)
        PolyAddTo(&polies[0], &polies[i]);

    Poly res_poly = polies[0];

    for (size_t i = 1; i < p->size; i++)
        PolyDestroy(&polies[i]);

    free(polies);

    return res_poly;
}

Poly PolyCompose(const Poly *p, size_t k, const Poly q[])
{
    Poly res_poly = PolyClone(p);
    Poly *q_copy = malloc(k * sizeof(Poly));
    CHECK_PTR(q_copy);
    for (size_t i = 0; i < k; i++)
        q_copy[i] = PolyClone(&q[i]);

    PolyComposeTo(&res_poly, k, q_copy);

    free(q_copy);

    return res_poly;
}