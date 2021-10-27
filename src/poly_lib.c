/** @file
  Implementacja biblioteki wielomianów rzadkich wielu zmiennych

  @authors Michał Molas
  <mm429570@students.mimuw.edu.pl>
  @copyright Uniwersytet Warszawski
  @date 2021
*/

#include "poly_lib.h"
#include "calc.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

/**
 * Porównuje jednomiany malejąco po wykładnikach. Używana w funkcji qsort.
 * @param[in] a : jednomian @f$a@f$
 * @param[in] b : jednomian @f$b@f$
 * @return 1 jeśli @f$a > b@f$, 0 jeśli @f$a == b@f$, -1 jeśli @f$a < b@f$
 */
static int MonoCompFunc(const void *a, const void *b)
{
    return (((Mono *)b)->exp > ((Mono *)a)->exp) -
           (((Mono *)b)->exp < ((Mono *)a)->exp);
}

/**
 * Zwraca liczbę różnych potęg w jednomianach sumy @f$p@f$ i @f$q@f$
 * @param[in] p : wielomian @f$p@f$
 * @param[in] q : wielomian @f$q@f$
 * @return liczba różnych potęg w jednomianach sumy @f$p@f$ i @f$q@f$
 */
static size_t PolySummedSize(const Poly *p, const Poly *q)
{
    assert(!PolyIsCoeff(p) && !PolyIsCoeff(q));

    size_t i = 0;
    size_t j = 0;

    size_t cnt = 0;

    // pętla zlicza jednomiany o różnych wykładnikach,
    // aż nie dojdzie do końca jednego z wielomianów
    while (i < p->size && j < q->size)
    {
        cnt++;

        if (p->arr[i].exp > q->arr[j].exp)
            j--;
        else if (p->arr[i].exp < q->arr[j].exp)
            i--;

        i++;
        j++;
    }

    // zwracane są policzone w pętli jednomiany plus reszta z wielomianu,
    // przez którą nie przeszła pętla
    return cnt + (p->size - i) + (q->size - j);
}

/**
 * Oblicza maksimum z wykładników @f$a@f$ i @f$b@f$
 * @param[in] a : Wykładnik @f$a@f$
 * @param[in] b : Wykładnik @f$b@f$
 * @return maksimum z @f$a@f$ i @f$b@f$
 */
static poly_exp_t MaxExp(poly_exp_t a, poly_exp_t b) { return a > b ? a : b; }

/**
 * Zwraca rozmiar, który należy zaalokować, będący najmniejszą potęgą 2 większą
 * od liczby elementów, dla liczby elementów większej od INIT_SIZE oraz
 * INIT_SIZE wpp
 * @param[in] count : liczba elementów
 * @return rozmiar do zaalokowania
 */
static size_t PolyRequiredSize(size_t count)
{
    if (count <= INIT_SIZE)
        return INIT_SIZE;

    size_t req_size = MEM_SIZE_MULT * INIT_SIZE;
    while (req_size < count)
        req_size *= MEM_SIZE_MULT;

    return req_size;
}

/**
 * Sprawdza, czy jednomian @f$m@f$ jest równy zero
 * @param[in] m : jednomian @f$m@f$
 * @return czy jednomian @f$m@f$ jest równy zero
 */
static bool MonoIsZero(const Mono *m)
{
    return PolyIsCoeff(&m->p) && m->p.coeff == 0;
}

/**
 * Liczy jednomiany zerowe w wielomianie @f$p@f$
 * @param[in] p : wielomian @f$p@f$
 * @return liczba zerowych jednomianów w @f$p@f$
 */
static size_t PolyCountZeroMonos(const Poly *p)
{
    size_t count = 0;
    for (size_t i = 0; i < p->size; i++)
        if (MonoIsZero(&p->arr[i]))
            count++;

    return count;
}

/**
 * Usuwa zawartość tablicy arr wielomianu @f$p@f$ (niebędącego coeffem) i
 * zamienia go na coeff @f$c@f$
 * @param[in,out] p : wielomian @f$p@f$
 * @param[in] c : współczynnik @f$c@f$
 */
static void PolyToCoeff(Poly *p, poly_coeff_t c)
{
    assert(p != NULL && !PolyIsCoeff(p));

    for (size_t i = 0; i < p->size; i++)
        MonoDestroy(&p->arr[i]);

    free(p->arr);
    p->arr = NULL;
    p->coeff = c;
}

void MonosSort(Mono *monos, size_t size)
{
    qsort(monos, size, sizeof(Mono), MonoCompFunc);
}

/**
 * Dodaje wielomian @f$q@f$ będący coeffem do wielomianu @f$p@f$ nie będącego
 * coeffem
 * @param[in,out] p : wielomian @f$p@f$
 * @param[in] q : wielomian @f$q@f$
 */
static void PolyAddCoeffTo(Poly *p, const Poly *q)
{
    assert(p != NULL && q != NULL && !PolyIsCoeff(p) && PolyIsCoeff(q));

    // sprawdzam czy w p jest już jednomian z x^0
    if (p->arr[p->size - 1].exp == 0)
    {
        PolyAddTo(&p->arr[p->size - 1].p, q);
    }
    else
    {
        p->size++;
        if (p->size > p->max_size)
        {
            p->max_size *= MEM_SIZE_MULT;
            p->arr = realloc(p->arr, p->max_size * sizeof(Mono));
            CHECK_PTR(p->arr);
        }

        p->arr[p->size - 1].exp = 0;
        p->arr[p->size - 1].p = PolyFromCoeff(q->coeff);
    }
}

/**
 * Dodaje jednomiany wielomianu @f$q@f$ do wielomianu @f$p@f$
 * @param[in,out] p : wielomian @f$p@f$
 * @param[in] q : wielomian @f$q@f$
 */
static void PolyCombineTo(Poly *p, const Poly *q)
{
    assert(p != NULL && q != NULL && !PolyIsCoeff(p) && !PolyIsCoeff(q));

    size_t curr_old = 0; // indeks na wykładniki z p
    size_t old_size = p->size;
    size_t curr_new = p->size; // indeks na nowe wykładniki

    p->size = PolySummedSize(p, q);
    if (p->size > p->max_size)
    {
        p->max_size = PolyRequiredSize(p->size);
        p->arr = realloc(p->arr, p->max_size * sizeof(Mono));
        CHECK_PTR(p->arr);
    }

    poly_exp_t curr_exp;

    for (size_t i = 0; i < q->size; i++)
    {
        curr_exp = q->arr[i].exp;
        while (curr_old < old_size && curr_exp < p->arr[curr_old].exp)
            curr_old++;

        if (curr_old < old_size && curr_exp == p->arr[curr_old].exp)
        {
            MonoAddTo(&p->arr[curr_old], &q->arr[i]);
            curr_old++;
        }
        else // curr_exp > p1->arr[curr_old].exp
        {
            p->arr[curr_new] = MonoClone(&q->arr[i]);
            curr_new++;
        }
    }
    MonosSort(p->arr, p->size);
}

/**
 * Sprawdza czy wielomian @f$p@f$ jest w postaci @f$p = Cx^0@f$
 * @param[in] p : wielomian @f$p@f$
 * @return czy wielomian @f$p@f$ jest w postaci @f$p = Cx^0@f$
 */
static bool PolyIsMonoCoeff(const Poly *p)
{
    assert(p != NULL);
    return !PolyIsCoeff(p) && p->size == 1 && p->arr[0].exp == 0 &&
           PolyIsCoeff(&p->arr[0].p);
}

/**
 * Przekształca wielomian (jeśli jest w takiej postaci) @f$p = Cx^0@f$ na
 * wielomian @f$p = C@f$
 * @param[in,out] p : wielomian @f$p@f$
 */
static void PolySimplifyCoeff(Poly *p)
{
    assert(p != NULL);
    if (PolyIsMonoCoeff(p))
        PolyToCoeff(p, p->arr[0].p.coeff);
}

/**
 * Sprawdza czy jednomian @f$m@f$ jest w postaci @f$m=(Cx^0)x^n@f$
 * @param[in] m : jednomian @f$m@f$
 * @return czy jednomian @f$m@f$ jest w postaci @f$m=(Cx^0)x^n@f$
 */
static bool MonoIsDeepCoeff(const Mono *m)
{
    assert(m != NULL);
    return !PolyIsCoeff(&m->p) && m->p.size == 1 && m->p.arr[0].exp == 0 &&
           PolyIsCoeff(&m->p.arr[0].p);
}

/**
 * Upraszcza jednomian @f$m@f$ w postaci @f$m=(Cx^0)x^n@f$ do jednomianu w
 * postaci @f$Cx^n@f$
 * @param[in,out] m : jednomian @f$m@f$
 */
static void MonoSimplifyDeepCoeff(Mono *m)
{
    assert(m != NULL);
    if (MonoIsDeepCoeff(m))
        PolyToCoeff(&m->p, m->p.arr[0].p.coeff);
}

/**
 * Usuwa jednomiany zerowe z wielomianu @f$p@f$
 * @param[in,out] p : wielomian @f$p@f$
 */
static void PolyReduceZeros(Poly *p)
{
    assert(p != NULL && !PolyIsCoeff(p));

    size_t zero_monos = PolyCountZeroMonos(p);

    if (zero_monos != 0)
    {
        if (p->size == zero_monos)
        {
            PolyToCoeff(p, 0);
            return;
        }

        size_t zero_cnt = 0;
        for (size_t i = 0; i < p->size; i++)
        {
            if (MonoIsZero(&p->arr[i]))
            {
                MonoDestroy(&p->arr[i]);
                zero_cnt++;
            }
            else if (zero_cnt != 0)
            {
                p->arr[i - zero_cnt] = p->arr[i];
            }
        }

        p->size -= zero_cnt;
    }
}

/**
 * Sprowadza wielomian do uproszczonej formy.
 * Uproszczona forma to:
 * - brak zerowych jednomianów
 * - wielomian nie ma postaci @f$p = Cx^0@f$
 * - brak zagnieżdżonych w dół @f$x^0@f$, kończących się na @f$x^0@f$
 * - jednomiany są posortowane malejąco według potęg
 * @param[in,out] p : wielomian @f$p@f$
 */
static void PolySimplify(Poly *p)
{
    assert(p != NULL);

    if (!PolyIsCoeff(p))
    {
        PolyReduceZeros(p);
        for (size_t i = 0; i < p->size; i++)
        {
            PolySimplify(&p->arr[i].p);
            MonoSimplifyDeepCoeff(&p->arr[i]);
        }
        PolySimplifyCoeff(p);
    }
}

Poly PolyFromMonos(size_t count, Mono *monos)
{
    assert(count >= 1);

    size_t offset = 0;
    poly_exp_t curr_exp = monos[0].exp;

    size_t simplified_size = 1;

    for (size_t i = 1; i < count; i++)
    {
        if (monos[i].exp == curr_exp)
        {
            offset++;
            MonoAddTo(&monos[i - offset], &monos[i]);
            MonoDestroy(&monos[i]);
        }
        else
        {
            simplified_size++;
            monos[i - offset] = monos[i];
            curr_exp = monos[i].exp;
        }
    }

    Poly poly_from_monos;

    poly_from_monos.size = simplified_size;
    poly_from_monos.arr = monos;
    poly_from_monos.max_size = count;

    PolyReduceZeros(&poly_from_monos);
    PolySimplifyCoeff(&poly_from_monos);

    return poly_from_monos;
}

void PolyToMonoCoeff(Poly *p)
{
    assert(p != NULL && PolyIsCoeff(p));

    p->arr = malloc(INIT_SIZE * sizeof(Mono));
    CHECK_PTR(p->arr);

    p->arr[0].exp = 0;
    p->arr[0].p = PolyFromCoeff(p->coeff);
    p->size = 1;
    p->max_size = INIT_SIZE;
}

void PolyAddTo(Poly *p, const Poly *q)
{
    assert(p != NULL && q != NULL);
    if (PolyIsCoeff(p))
    {
        if (PolyIsCoeff(q))
        {
            p->coeff += q->coeff;
            return;
        }

        PolyToMonoCoeff(p);
    }

    if (PolyIsCoeff(q))
        PolyAddCoeffTo(p, q);
    else // p, q NIE są coeffami
        PolyCombineTo(p, q);

    PolySimplify(p);
}

void PolyNegTo(Poly *p) { PolyMulByCoeffTo(p, -1); }

void PolyMulByCoeffTo(Poly *p, poly_coeff_t c)
{
    assert(p != NULL);

    if (PolyIsCoeff(p))
    {
        p->coeff *= c;
    }
    else if (c == 0)
    {
        PolyToCoeff(p, 0);
    }
    else
    {
        for (size_t i = 0; i < p->size; i++)
            PolyMulByCoeffTo(&p->arr[i].p, c);
    }
}

void PolyDegByHelp(const Poly *p, size_t var_idx, size_t curr_idx,
                   poly_exp_t *max_exp)
{
    if (!PolyIsCoeff(p))
    {
        if (var_idx == curr_idx)
        {
            *max_exp = MaxExp(*max_exp, p->arr[0].exp);
        }
        else
        {
            for (size_t i = 0; i < p->size; i++)
                PolyDegByHelp(&p->arr[i].p, var_idx, curr_idx + 1, max_exp);
        }
    }
}

void PolyDegHelp(const Poly *p, poly_exp_t *max_exp, poly_exp_t curr_exp)
{
    if (PolyIsCoeff(p))
    {
        *max_exp = MaxExp(curr_exp, *max_exp);
    }
    else
    {
        for (size_t i = 0; i < p->size; i++)
            PolyDegHelp(&p->arr[i].p, max_exp, curr_exp + p->arr[i].exp);
    }
}

Mono MonoMul(const Mono *m, const Mono *n)
{
    return (Mono){.exp = m->exp + n->exp, .p = PolyMul(&m->p, &n->p)};
}

Poly PolyMulByCoeff(const Poly *p, poly_coeff_t c)
{
    Poly res_poly = PolyClone(p);

    PolyMulByCoeffTo(&res_poly, c);
    PolySimplify(&res_poly);

    return res_poly;
}

poly_coeff_t Power(poly_coeff_t x, poly_exp_t exp)
{
    if (x == 1 || exp == 0)
        return 1;

    if (x == 0)
        return 0;

    poly_coeff_t remainder = exp % 2;
    poly_coeff_t half_power = Power(x, exp / 2);
    return half_power * half_power * (remainder == 1 ? x : 1);
}

/**
 * Zwraca liczbę potęg 2 mniejszych od exp
 * @param[in] exp
 * @return liczba potęg 2 mniejszych od exp
 */
static size_t ExpLogSize(poly_exp_t exp)
{
    size_t size = 0;
    while (exp != 0)
    {
        size++;
        exp /= 2;
    }
    return size;
}

/**
 * Oblicza wielomian, którego potęgi 2 zapisane w @p power_table do potęgi @p
 * exp. Wykonuje to sprawdzając binarny zapis @p exp i mnożąc odpowiednie potęgi
 * z @p power_table
 * @param[in] power_table : tablica potęg 2 wielomianu
 * @param[in] exp : potęga
 * @return wielomian, którego potęgi 2 zapisane w @p power_table do potęgi @p
 * exp
 */
static Poly PolyPower(const Poly *power_table, poly_exp_t exp)
{
    if (exp == 0)
        return PolyFromCoeff(1);

    bool is_first_power = true;
    bool is_power_of_2 = true;
    size_t pow_tab_idx = 0;

    Poly mult_poly;
    Poly res_poly;

    while (exp != 0)
    {
        if (exp % 2 == 1)
        {
            if (is_first_power)
            {
                is_first_power = false;
                res_poly = power_table[pow_tab_idx];
            }
            else
            {
                if (PolyIsCoeff(&power_table[pow_tab_idx]))
                {
                    if (is_power_of_2)
                        res_poly = PolyClone(&res_poly);
                    PolyMulByCoeffTo(&res_poly, power_table[pow_tab_idx].coeff);
                }
                else
                {
                    mult_poly = PolyMul(&res_poly, &power_table[pow_tab_idx]);
                    if (!is_power_of_2)
                        PolyDestroy(&res_poly);
                    res_poly = mult_poly;
                }
                is_power_of_2 = false;
            }
        }
        pow_tab_idx++;
        exp /= 2;
    }

    if (is_power_of_2)
        return PolyClone(&res_poly);
    else
        return res_poly;
}

/**
 * Zwraca maksimum z liczb @f$a,b@f$.
 * @param[in] a : liczba @f$a@f$
 * @param[in] b : liczba @f$b@f$
 * @return maksimum z @f$a@f$ i @f$b@f$
 */
#define MAX(a, b) ((a > b) ? (a) : (b))

/**
 * Znajduje rekurencyjnie największy wykładnik przy @f$x_{\mathrm{idx}}@f$.
 * Wyniki zapisuje w @p max_exp
 * @param[in] p : wielomian @f$p@f$
 * @param[in,out] max_exp : tablica maksymalnych potęg dla danej zmiennej
 * @param[in] k : rozmiar tablicy @p max_exp
 * @param[in] idx : aktualny indeks zmiennej
 */
static void CheckMaxExps(const Poly *p, poly_exp_t *max_exp, size_t k,
                         size_t idx)
{
    if (!PolyIsCoeff(p) && idx < k)
    {
        for (size_t i = 0; i < p->size; i++)
        {
            max_exp[idx] = MAX(max_exp[idx], p->arr[i].exp);
            CheckMaxExps(&p->arr[i].p, max_exp, k, idx + 1);
        }
    }
}

/**
 * Zwraca tablicę @f$n@f$ pierwszych potęg 2 wielomianu @f$p@f$. Przejmuje
 * wielomian @f$p@f$ na własność.
 * @param[in,out] p : wielomian @f$p@f$
 * @param[in] n : liczba potęg
 * @return tablica @f$n@f$ pierwszych potęg 2 wielomianu @f$p@f$
 */
static Poly *PolyPowerTable(Poly *p, size_t n)
{
    if (n == 0)
        return NULL;

    Poly *power_table = malloc(n * sizeof(Poly));
    CHECK_PTR(power_table);
    power_table[0] = *p;

    for (size_t i = 1; i < n; i++)
        power_table[i] = PolyMul(&power_table[i - 1], &power_table[i - 1]);

    return power_table;
}

static void PolyComposeHelp(Poly *p, size_t k, Poly **poly_pow, size_t idx);

/**
 * Funkcja wykonywana, w przypadku, gdy wielomianów składanych jest mniej niż
 * zmiennych wielomianu @f$p@f$.
 * @param[in,out] p : wielomian @f$p@f$
 * @param[in] k : liczba składanych wielomianów
 * @param[in] poly_pow : tablica tablic potęg 2 składanych wielomianów
 * @param[in] idx : aktualny indeks zmiennej
 */
static void PolyComposeNotEnough(Poly *p, size_t k, Poly **poly_pow, size_t idx)
{
    if (!PolyIsCoeff(p))
    {
        if (p->arr[p->size - 1].exp == 0)
        {
            // sprawdzam, czy poniższy wielomian się nie wyzeruje
            PolyComposeHelp(&p->arr[p->size - 1].p, k, poly_pow, idx + 1);
            if (PolyIsZero(&p->arr[p->size - 1].p))
            {
                PolyDestroy(p);
                *p = PolyZero();
            }
            else
            {
                Poly p_temp = *p;
                // wiem, że poniżej są wyłącznie wielomiany z x^0, bo
                // inaczej by się wyzerowały
                while (!PolyIsCoeff(&p_temp))
                    p_temp = p_temp.arr[p->size - 1].p;

                poly_coeff_t last_coeff = p_temp.coeff;
                PolyDestroy(p);
                *p = PolyFromCoeff(last_coeff);
            }
        }
        else
        {
            PolyDestroy(p);
            *p = PolyZero();
        }
    }
}

/**
 * Wykonuje operację składania na tablicy jednomianów wielomianu @f$p@f$.
 * Otrzymaną tablicę wielomianów zapisuje do wskaźnika @p polies i zwraca
 * liczbę elementów w tej tablicy.
 * @param[in,out] p : wielomian @f$p@f$
 * @param[in] k : liczba składanych wielomianów
 * @param[in] poly_pow : tablica tablic potęg 2 składanych wielomianów
 * @param[in] idx : aktualna głębokość rekurencji
 * @param[out] polies : wskaźnik na tablicę wielomianów
 * @return liczba wielomianów w tablicy @p polies
 */
static size_t PolyComposeMonos(Poly *p, size_t k, Poly **poly_pow, size_t idx,
                               Poly **polies)
{
    size_t arr_size = p->size;

    size_t cnt_zeros = 0;
    Poly x_p;

    for (size_t i = 0; i < p->size; i++)
    {
        PolyComposeHelp(&p->arr[i].p, k, poly_pow, idx + 1);
        if (PolyIsZero(&p->arr[i].p))
        {
            cnt_zeros++;
        }
        else
        {
            if (*polies == NULL)
            {
                *polies = malloc((arr_size - cnt_zeros) * sizeof(Poly));
                CHECK_PTR(polies);
            }

            x_p = PolyPower(poly_pow[idx], p->arr[i].exp);

            // poniższe ify są tutaj, aby nieco zmniejszyć alokacje, mogłoby być
            // tylko to co w else
            if (PolyIsCoeff(&x_p))
            {
                (*polies)[i - cnt_zeros] = p->arr[i].p;
                PolyMulByCoeffTo(&((*polies)[i - cnt_zeros]), x_p.coeff);
                continue;
            }

            if (PolyIsCoeff(&p->arr[i].p))
            {
                (*polies)[i - cnt_zeros] = x_p;
                PolyMulByCoeffTo(&((*polies)[i - cnt_zeros]),
                                 p->arr[i].p.coeff);
            }
            else
            {
                (*polies)[i - cnt_zeros] = PolyMul(&x_p, &p->arr[i].p);
                PolyDestroy(&x_p);
            }
        }
        MonoDestroy(&p->arr[i]);
    }

    return arr_size - cnt_zeros;
}

/**
 * Funkcja pomocnicza do ::PolyComposeTo. Wykonuje opisane tam zadanie
 * rekurencyjnie.
 * @param[in,out] p : wielomian @f$p@f$
 * @param[in] k : liczba składanych wielomianów
 * @param[in] poly_pow : tablica tablic potęg 2 składanych wielomianów
 * @param[in] idx : aktualna głębokość rekurencji
 */
static void PolyComposeHelp(Poly *p, size_t k, Poly **poly_pow, size_t idx)
{
    if (idx >= k)
    {
        PolyComposeNotEnough(p, k, poly_pow, idx);
        return;
    }
    if (PolyIsCoeff(p))
        return;

    Poly *polies = NULL;
    size_t polies_cnt = PolyComposeMonos(p, k, poly_pow, idx, &polies);

    if (polies_cnt > 0)
    {
        for (size_t i = 1; i < polies_cnt; i++)
            PolyAddTo(&polies[0], &polies[i]);

        free(p->arr);
        *p = polies[0];

        for (size_t i = 1; i < polies_cnt; i++)
            PolyDestroy(&polies[i]);

        free(polies);
    }
    else
    {
        PolyDestroy(p);
        *p = PolyZero();
    }
}

void PolyComposeTo(Poly *p, size_t k, Poly q[])
{
    poly_exp_t *max_exp = calloc(k, sizeof(poly_exp_t));
    CHECK_PTR(max_exp);
    CheckMaxExps(p, max_exp, k, 0);

    // Tworzenie tablicy z potrzebnymi potęgami 2 dla każdego q_i
    Poly **poly_pow = malloc(k * sizeof(Poly *));
    CHECK_PTR(poly_pow);
    for (size_t i = 0; i < k; i++)
    {
        poly_pow[i] = PolyPowerTable(q + i, ExpLogSize(max_exp[i]));
        if (poly_pow[i] == NULL)
            PolyDestroy(q + i);
    }

    PolyComposeHelp(p, k, poly_pow, 0);

    // Zwalnianie pamięci
    for (size_t i = 0; i < k; i++)
    {
        if (poly_pow[i] != NULL)
        {
            size_t log_size = ExpLogSize(max_exp[i]);
            for (size_t j = 0; j < log_size; j++)
                PolyDestroy(&poly_pow[i][j]);

            free(poly_pow[i]);
        }
    }

    free(max_exp);
    free(poly_pow);
}
