/** @file
  Implementacja biblioteki kalkulatora wielomianów

  @authors Michał Molas
  <mm429570@students.mimuw.edu.pl>
  @copyright Uniwersytet Warszawski
  @date 2021
*/

#include "calc.h"
#include "calc_parse.h"
#include "poly.h"
#include "poly_lib.h"
#include "stack.h"
#include <ctype.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/** znak rozpoczynający komentarz */
#define COMMENT '#'

void InstZero(Stack *s) { StackPush(s, PolyZero()); }

void InstIsCoeff(const Stack *s) { printf("%d\n", PolyIsCoeff(StackPeek(s))); }

void InstIsZero(const Stack *s) { printf("%d\n", PolyIsZero(StackPeek(s))); }

void InstClone(Stack *s) { StackPush(s, PolyClone(StackPeek(s))); }

void InstAdd(Stack *s)
{
    Poly *p = StackPop(s);
    PolyAddTo(StackPeek(s), p);
    PolyDestroy(p);
}

void InstMul(Stack *s)
{
    Poly *p_first = StackPop(s);
    Poly *p_second = StackPeek(s);
    if (PolyIsCoeff(p_first))
    {
        PolyMulByCoeffTo(p_second, p_first->coeff);
    }
    else if (PolyIsCoeff(p_second))
    {
        PolyMulByCoeffTo(p_first, p_second->coeff);
        StackChangeTop(s, *p_first);
    }
    else
    {
        p_second = StackPop(s);
        Poly new_poly = PolyMul(p_first, p_second);
        PolyDestroy(p_first);
        PolyDestroy(p_second);
        StackPush(s, new_poly);
    }
}

void InstNeg(Stack *s) { PolyNegTo(StackPeek(s)); }

void InstSub(Stack *s)
{
    Poly *p = StackPop(s);
    PolyNegTo(StackPeek(s));
    PolyAddTo(StackPeek(s), p);
    PolyDestroy(p);
}

void InstIsEq(const Stack *s)
{
    printf("%d\n", PolyIsEq(StackPeek(s), StackPeekNext(s)));
}

void InstDeg(const Stack *s) { printf("%d\n", PolyDeg(StackPeek(s))); }

void InstDegBy(const Stack *s, size_t idx)
{
    printf("%d\n", PolyDegBy(StackPeek(s), idx));
}

void InstAt(Stack *s, poly_coeff_t x)
{
    Poly *p = StackPop(s);
    Poly q = PolyAt(p, x);
    PolyDestroy(p);
    StackPush(s, q);
}

void InstPrint(const Stack *s) { PolyPrint(StackPeek(s)); }

void InstPop(Stack *s) { PolyDestroy(StackPop(s)); }

void InstCompose(Stack *s, size_t k)
{
    Poly *p = StackPop(s);
    Poly *q = StackPopK(s, k);

    PolyComposeTo(p, k, q);
    StackPush(s, *p);
}

static void PolyPrintHelp(const Poly *p);

/**
 * Wypisuje jednomian.
 * @param[in] m : jednomian
 */
static void MonoPrint(const Mono *m)
{
    printf("(");
    PolyPrintHelp(&m->p);
    printf(",%d)", m->exp);
}

/**
 * Wykonuje faktyczne wypisywanie wielomianu, bez znaku nowej linii na końcu.
 * @param[in] p : wielomian @f$p@f$
 */
static void PolyPrintHelp(const Poly *p)
{
    if (PolyIsCoeff(p))
    {
        printf("%ld", p->coeff);
    }
    else
    {
        MonoPrint(&p->arr[p->size - 1]);
        for (size_t i = p->size - 1; i-- > 0;)
        {
            printf("+");
            MonoPrint(&p->arr[i]);
        }
    }
}

void PolyPrint(const Poly *p)
{
    PolyPrintHelp(p);
    printf("\n");
}

/**
 * Wykonuje zadaną instrukcję na stosie @p stack.
 * @param[in,out] stack : stos wielomianów
 * @param[in] inst : instrukcja
 * @return czy wystąpił stack underflow
 */
static bool RunInstruction(Stack *stack, const Instruction inst)
{
    if (STR_EQ(inst.type, ZERO))
    {
        InstZero(stack);
        return false;
    }
    else if (!StackIsEmpty(stack))
    {
        if (STR_EQ(inst.type, IS_COEFF))
            InstIsCoeff(stack);
        else if (STR_EQ(inst.type, IS_ZERO))
            InstIsZero(stack);
        else if (STR_EQ(inst.type, CLONE))
            InstClone(stack);
        else if (STR_EQ(inst.type, NEG))
            InstNeg(stack);
        else if (STR_EQ(inst.type, DEG))
            InstDeg(stack);
        else if (STR_EQ(inst.type, DEG_BY))
            InstDegBy(stack, inst.idx);
        else if (STR_EQ(inst.type, AT))
            InstAt(stack, inst.x);
        else if (STR_EQ(inst.type, PRINT))
            InstPrint(stack);
        else if (STR_EQ(inst.type, POP))
            InstPop(stack);
        else if (STR_EQ(inst.type, COMPOSE))
        {
            if (inst.k != SIZE_MAX && StackHasEnoughElements(stack, inst.k + 1))
                InstCompose(stack, inst.k);
            else
                return true;
        }
        else if (!StackIsAlmostEmpty(stack))
        {
            if (STR_EQ(inst.type, ADD))
                InstAdd(stack);
            else if (STR_EQ(inst.type, MUL))
                InstMul(stack);
            else if (STR_EQ(inst.type, SUB))
                InstSub(stack);
            else if (STR_EQ(inst.type, IS_EQ))
                InstIsEq(stack);
        }
        else
            return true;
    }
    else
    {
        return true;
    }
    return false;
}

/**
 * Wypisuje błąd parsowania na wyjście diagnostyczne.
 * @param[in] index : numer parsowanej linii
 * @param[in] type : rodzaj błędu
 */
static void PrintError(size_t index, const char *type)
{
    fprintf(stderr, "ERROR %ld %s\n", index, type);
}

bool ParseAndExecuteLine(Stack *stack, size_t index)
{
    ParsingStatus status = NewParsingStatus();

    // Mylić może, że stosuję typ int i funkcję getc zamiast char i getchar,
    // jednak jest to kierowane tym, by niektóre nietypowe znaki, nie mieszczące
    // się w zakresie char nie były interpretowane jako EOF, co prowadziłoby do
    // błędu wczesnego zakończenia wczytywania
    int c = getc(stdin);
    if (isalpha(c))
    {
        ungetc(c, stdin);
        Instruction inst = ParseInstruction(&status);

        if (status.is_correct)
        {
            if (RunInstruction(stack, inst))
                PrintError(index, ERROR_STACK_UNDERFLOW);

            return status.is_eol;
        }
        else
        {
            PrintError(index, inst.type);
            if (status.is_eof || status.is_eol)
                return status.is_eol;
        }
    }
    else if (c != COMMENT && c != '\n' && c != EOF)
    {
        ungetc(c, stdin);
        Poly p = ParsePoly(&status);
        if (status.is_correct)
        {
            StackPush(stack, p);
            return status.is_eol;
        }
        else
        {
            PrintError(index, ERROR_WRONG_POLY);
            if (status.is_eof || status.is_eol)
                return status.is_eol;
        }
    }

    // Kiedy ta pętla się wykona?
    // - jeśli c (wczytane przed ifami) było '\n' lub EOF to nie
    // - jeśli było '#' to tak
    // - jeśli był error w jednym z ifów to program tutaj dojdzie tylko jeśli
    // error nie był na '\n' ani EOF. Wtedy poniższy if będzie prawdziwy, a
    // pętla się wykona, czyli prawdiłowo.
    //
    // (piszę to ponieważ sam się zastanawiałem czy to jest dobrze, ale jest)
    if (c != '\n' && c != EOF)
        while ((c = getc(stdin)) != EOF && c != '\n')
            ;

    return c == '\n';
}

/**
 * Uruchamia kalkulator wielomianów
 * @return kod wyścia programu
 */
int main(void)
{
    Stack stack = StackNewEmpty();

    size_t line_cnt = 0;
    while (ParseAndExecuteLine(&stack, ++line_cnt))
        ;

    StackDestroy(&stack);

    return 0;
}