/** @file
  Implementacja struktury stosu wielomianów

  @authors Michał Molas <mm429570@students.mimuw.edu.pl>
  @copyright Uniwersytet Warszawski
  @date 2021
*/

#include "stack.h"
#include "calc.h"
#include "poly.h"
#include "poly_lib.h"
#include <stdio.h>

/** minimalny rozmiar stosu, przy którym można zmniejszać pamięć */
#define MIN_REDUCING_SIZE 32

/** początkowy rozmiar stosu */
#define STACK_INIT_SIZE 8

void StackDestroy(Stack *s)
{
    for (size_t i = 0; i < s->size; i++)
        PolyDestroy(&s->polies[i]);

    free(s->polies);
}

Poly *StackPeek(const Stack *s)
{
    assert(!StackIsEmpty(s));
    return s->polies + s->size - 1;
}

Poly *StackPeekNext(const Stack *s)
{
    assert(!StackIsAlmostEmpty(s));
    return s->polies + s->size - 2;
}

/**
 * Sprawdza, czy należy zmniejszyć pamięć przed usuięciem elementu, czyli
 * sprawdza czy stos po usunięciu elementu będzie zapełniony w 1/4.
 * @param[in,out] s : stos wielomianów
 * @return czy należy zmniejszyć pamięć stosu
 */
static bool StackShouldReduce(Stack *s)
{
    return s->max_size >= MIN_REDUCING_SIZE &&
           s->size - 1 <= s->max_size / (MEM_SIZE_MULT * MEM_SIZE_MULT);
}

/**
 * Zmniejsza pojemność stosu o połowę
 * @param[out] s : stos wielomianów
 */
static void StackReduceMemory(Stack *s)
{
    s->max_size /= MEM_SIZE_MULT;
    s->polies = realloc(s->polies, s->max_size * sizeof(Poly));
    CHECK_PTR(s->polies);
}

Poly *StackPop(Stack *s)
{
    if (StackShouldReduce(s))
        StackReduceMemory(s);

    Poly *temp = StackPeek(s);
    s->size--;

    return temp;
}

/**
 * Zwiększa pamięć stosu
 * @param[out] s : stos wielomianów
 */
static void StackIncreaseSize(Stack *s)
{
    s->max_size *= MEM_SIZE_MULT;
    s->polies = realloc(s->polies, s->max_size * sizeof(Poly));
    CHECK_PTR(s->polies);
}

void StackPush(Stack *s, Poly p)
{
    s->size++;
    if (s->size > s->max_size)
        StackIncreaseSize(s);

    s->polies[s->size - 1] = p;
}

void StackChangeTop(Stack *s, Poly p)
{
    PolyDestroy(&s->polies[s->size - 1]);
    s->polies[s->size - 1] = p;
}

Stack StackNewEmpty()
{
    Stack s;
    s.size = 0;
    s.max_size = STACK_INIT_SIZE;
    s.polies = malloc(s.max_size * sizeof(Poly));
    CHECK_PTR(s.polies);
    return s;
}

bool StackIsEmpty(const Stack *s) { return s->size == 0; }

bool StackIsAlmostEmpty(const Stack *s) { return s->size < 2; }

bool StackHasEnoughElements(const Stack *s, size_t n) { return s->size >= n; }

Poly *StackPopK(Stack *s, size_t k)
{
    assert(StackHasEnoughElements(s, k));
    s->size -= k - 1;

    return StackPop(s);
}