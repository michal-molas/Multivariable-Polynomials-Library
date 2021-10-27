/** @file
  Implementacja biblioteki wczytującej input do kalkulatora wielomianów

  @authors Michał Molas
  <mm429570@students.mimuw.edu.pl>
  @copyright Uniwersytet Warszawski
  @date 2021
*/

#include "calc_parse.h"
#include "poly.h"
#include "poly_lib.h"
#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** rozmiar buffera wystarczający do wczytania każdej z instrukcji bez znaku
 * '\0' (IS_COEFF ma najwięcej znaków, czyli 8) */
#define MAX_INST_LEN 8

/** Wartość zwracana w przypadku błędu wczytywania wielomianu */
#define ERROR_POLY PolyZero()

/** Wartość zwracana w przypadku błędu wczytywania instrukcji */
#define ERROR_INST(ERROR_TYPE)                                                 \
    (Instruction) { .type = ERROR_TYPE, .x = 0 }

/** liczba operacji bezparametrowych */
#define INST_NO_ARG_SIZE 12

/** lista nazw instrukcji bez parametrów */
static const char *INST_NO_ARG_LIST[] = {
    ZERO, IS_COEFF, IS_ZERO, CLONE, ADD, MUL, NEG, SUB, IS_EQ, DEG, PRINT, POP};

/** znak minusa */
#define MINUS '-'

/** znak plusa */
#define PLUS '+'

/** znak spacji */
#define SPACE ' '

/** znak przecinka */
#define COMMA ','

/** znak otwierającego nawiasu */
#define LBRAC '('

/** znak zamykającego nawiasu */
#define RBRAC ')'

/** znak podkreślnika */
#define UNDERSCR '_'

/**
 * Sprawdza, czy liczba @p x zmieści się w zakresie @p range po dodaniu cyfry @p
 * new_digit na koniec tej liczby.
 *
 * Uwagi:
 * - należy zapewnić, że jeśli zakres jest dodatni to liczba musi być nieujemna,
 * a jeśli zakres jest ujemny to liczba musi być ujemna.
 * - @p new_digit musi być cyfrą, tj. liczbą całkowitą z zakresu [0,9]
 * - porównanie zakresu jest z 1, a nie 0, aby zapobiec ostrzeżeniom kompilatora
 * przy porównywaniu typów unsigned. Nie wpływa to jednak na wynik nawet w
 * ekstremalnych przypadkach.
 *
 * @param[in] x : liczba @p x
 * @param[in] new_digit : cyfra @p new_digit
 * @param[in] range : zakres @p range
 * @return czy liczba mieści się w zadanym zakresie
 */
#define IS_IN_RANGE(x, new_digit, range)                                       \
    (range < 1                                                                 \
         ? (x > range / 10 || (x == range / 10 && new_digit <= -(range % 10))) \
         : (x < range / 10 || (x == range / 10 && new_digit <= range % 10)))

ParsingStatus NewParsingStatus() { return (ParsingStatus){.is_correct = true}; }

/**
 * Ustawia parametry statusu parsowania w przypadku niepoprawnego wczytania
 * linii
 * @param[out] status : status parsowania
 * @param[in] last_char : ostatni wczytany znak z wejścia
 */
static void StatusSetError(ParsingStatus *status, int last_char)
{
    status->is_correct = false;
    status->is_eof = last_char == EOF;
    status->is_eol = last_char == '\n';
}

/**
 * Ustawia parametry statusu parsowania w przypadku poprawnego wczytania linii.
 * @param[out] status : status parsowania
 * @param[in] is_eof : czy ostatnim znakiem był EOF (jeśli nie to był nim '\n')
 */
static void StatusSetCorrect(ParsingStatus *status, bool is_eof)
{
    status->is_correct = true;
    status->is_eof = is_eof;
    status->is_eol = !is_eof;
}

/**
 * Parsuje współczynnik
 *
 * Ostatnim wczytanym znakiem przed wywołaniem ma być spacja po "AT" lub '('
 * przy wczytywaniu wielomianu w jednomianie lub wczytywanie ma się zacząć od
 * początku linii, jeśli wielomian nie jest w jednomianie.
 *
 * Ostatnim wczytanym znakiem (o ile nie wystąpi błąd) będzie cyfra jedności
 * współczynnika.
 *
 * @param[out] status : status parsowania
 * @return wczytany współczynnik lub 0 w przypadku błędu
 */
static poly_coeff_t ParseCoeff(ParsingStatus *status)
{
    int c = getc(stdin);
    bool is_neg = c == MINUS;
    poly_coeff_t num_val = c - '0';
    poly_coeff_t coeff = is_neg ? 0 : num_val;

    bool was_in_loop = false;

    while ((c = getc(stdin)) != EOF && c != '\n' && c != COMMA)
    {
        was_in_loop = true;

        num_val = c - '0';
        if (isdigit(c) && (is_neg ? IS_IN_RANGE(coeff, num_val, LONG_MIN)
                                  : IS_IN_RANGE(coeff, num_val, LONG_MAX)))
        {
            coeff = 10 * coeff + (is_neg ? -1 : 1) * num_val;
        }
        else
        {
            StatusSetError(status, c);
            return 0;
        }
    }

    if (!was_in_loop && is_neg)
    {
        StatusSetError(status, c);
        return 0;
    }

    ungetc(c, stdin);
    return coeff;
}

/**
 * Parsuje wielomian będący współczynnikiem.
 * W przypadku braku błędu zapisuje wielomian we wskaźniku @p p.
 *
 * Ostatnim wczytanym znakiem przed wywołaniem ma być '(', jeśli wielomian jest
 * wewnątrz jednomianu lub wczytywanie ma się zaczynać od początku linii, jeśli
 * wielomian nie jest w jednomianie.
 *
 * Ostatnim wczytanym znakiem (o ile nie wystąpi błąd) będzie '\n' lub EOF jeśli
 * wielomian nie jest w jednomianie lub ',' jeśli wielomian jest w jednomianie.
 *
 * @param[in,out] status : status parsowania
 * @param[in] is_in_mono : czy wielomian jest wewnątrz jednomianu
 * @param[out] p : wskaźnik na wielomian
 */
static void ParsePolyCoeff(ParsingStatus *status, bool is_in_mono, Poly *p)
{
    if (status->is_correct)
    {
        poly_coeff_t coeff = ParseCoeff(status);
        if (!status->is_correct)
            return;

        int c = getc(stdin);
        if (is_in_mono != (c == COMMA))
        {
            StatusSetError(status, c);
            return;
        }

        if (!is_in_mono)
            StatusSetCorrect(status, c == EOF);

        *p = PolyFromCoeff(coeff);
    }
}

/**
 * Parsuje wykładnik.
 *
 * Ostatnim wczytanym znakiem przed wywołaniem ma być ','.
 *
 * Ostatnim wczytanym znakiem (o ile nie wystąpi błąd) będzie ')'.
 *
 * @param[in,out] status : status parsowania
 * @return wykładnik lub -1 w przypadku błędu
 */
static poly_exp_t ParseExp(ParsingStatus *status)
{
    if (status->is_correct)
    {
        poly_exp_t exp = 0;
        poly_exp_t num_val;
        int c;

        while ((c = getc(stdin)) != RBRAC)
        {
            num_val = c - '0';
            if (isdigit(c) && IS_IN_RANGE(exp, num_val, INT_MAX))
            {
                exp = 10 * exp + num_val;
            }
            else
            {
                StatusSetError(status, c);
                return -1;
            }
        }

        return exp;
    }

    return -1;
}

static void ParsePolyHelp(ParsingStatus *status, bool is_in_mono, Poly *p);

/**
 * Parsuje jednomian.
 * W przypadku braku błędu zapisuje jednomian we wskaźniku @p m.
 *
 * Ostatnim wczytanym znakiem przed wywołaniem ma być '('.
 *
 * Ostatnim wczytanym znakiem (o ile nie wystąpi błąd) będzie ')'.
 *
 * @param[in,out] status : status parsowania
 * @param[out] m : wskaźnik na jednomian
 */
static void ParseMono(ParsingStatus *status, Mono *m)
{
    if (status->is_correct)
    {
        ParsePolyHelp(status, true, &m->p);
        if (!status->is_correct)
            return;

        m->exp = ParseExp(status);
        if (!status->is_correct)
            PolyDestroy(&m->p);
    }
}

/**
 * Parsuje sumę jednomianów.
 * W przypadku braku błędu zapisuje tablicę jednomianów we wskaźniku @p monos i
 * zwraca liczbę jednomianów w tej tablicy. W przypadku błędu zwraca 0.
 *
 * Ostatnim wczytanym znakiem przed wywołaniem ma być ')' pierwszego jednomianu.
 *
 * Ostatnim wczytanym znakiem (o ile nie wystąpi błąd) będzie ')' kończący
 * ostatni jednomian w sumie.
 *
 * @param[out] status : status parsowania
 * @param[out] monos : wskaźnik na tablicę jednomianów
 * @return liczba jednomianów w tablicy pod wskaźnikiem @p monos lub 0 w
 * przypadku błędu
 */
static size_t ParseMonos(ParsingStatus *status, Mono **monos)
{
    size_t monos_cnt = 1;
    size_t monos_max = INIT_SIZE;

    int c;
    while ((c = getc(stdin)) == PLUS)
    {
        if ((c = getc(stdin)) == LBRAC)
        {
            monos_cnt++;
            if (monos_cnt > monos_max)
            {
                monos_max *= MEM_SIZE_MULT;
                *monos = realloc(*monos, monos_max * sizeof(Mono));
                CHECK_PTR(*monos);
            }
            ParseMono(status, *monos + monos_cnt - 1);
            if (!status->is_correct)
            {
                // ostatniego mono NIE MOŻNA usuwać, ponieważ został już
                // zwolniony
                for (size_t i = 0; i < monos_cnt - 1; i++)
                    MonoDestroy(*monos + i);

                free(*monos);
                return 0;
            }
        }
        else
        {
            StatusSetError(status, c);
            return monos_cnt;
        }
    }

    ungetc(c, stdin); // czyli ostatnim wczytanym znakiem będzie ')'
    return monos_cnt;
}

/**
 * Parsuje wielomian niebędący współczynnikiem.
 * W przypadku braku błędu zapisuje wielomian we wskaźniku @p p.
 *
 * Ostatnim wczytanym znakiem przed wywołaniem ma być '(', jeśli wielomian jest
 * wewnątrz jednomianu lub wczytywanie ma się zaczynać od początku linii, jeśli
 * wielomian nie jest w jednomianie.
 *
 * Ostatnim wczytanym znakiem (o ile nie wystąpi błąd) będzie '\n' lub EOF jeśli
 * wielomian nie jest w jednomianie lub ',' jeśli wielomian jest w jednomianie,
 *
 * @param[in,out] status : status parsowania
 * @param[in] is_in_mono : czy wielomian jest wewnątrz jednomianu
 * @param[out] p : wskaźnik na wielomian
 */
static void ParsePolyNonCoeff(ParsingStatus *status, bool is_in_mono, Poly *p)
{
    if (status->is_correct)
    {
        Mono first_mono;
        ParseMono(status, &first_mono);
        if (!status->is_correct)
            return;

        Mono *monos = malloc(INIT_SIZE * sizeof(Mono));
        CHECK_PTR(monos);
        monos[0] = first_mono;

        size_t monos_cnt = ParseMonos(status, &monos);

        if (status->is_correct)
        {
            int c = getc(stdin); // ostatnim wczytanym był ')'
            if (c != COMMA && c != EOF && c != '\n')
            {
                StatusSetError(status, c);
            }
            else if (status->is_correct)
            {
                if (is_in_mono != (c == COMMA))
                    StatusSetError(status, c);
                else if (!is_in_mono) // czyli c to EOF lub '\n'
                    StatusSetCorrect(status, c == EOF);
            }
        }

        // tutaj celowo nie ma else, ponieważ status może się zmienić
        if (monos_cnt != 0 && !status->is_correct)
        {
            for (size_t i = 0; i < monos_cnt; i++)
                MonoDestroy(&monos[i]);

            free(monos);
            return;
        }

        *p = PolyOwnMonos(monos_cnt, monos);
    }
}

/**
 * Parsuje wielomian z uwzględnieniem czy wielomian jest wewnątrz jednomianu,
 * czy nie. W przypadku braku błędu zapisuje wielomian we wskaźniku @p p.
 *
 * Ostatnim wczytanym znakiem przed wywołaniem ma być '(', jeśli wielomian jest
 * wewnątrz jednomianu lub wczytywanie ma się zaczynać od początku linii, jeśli
 * wielomian nie jest w jednomianie.
 *
 * Ostatnim wczytanym znakiem (o ile nie wystąpi błąd) będzie '\n' lub EOF jeśli
 * wielomian nie jest w jednomianie lub ',' jeśli wielomian jest w jednomianie.
 *
 * @param[in,out] status : status parsowania
 * @param[in] is_in_mono : czy wielomian jest wewnątrz jednomianu
 * @param[out] p : wskaźnik na wielomian
 */
static void ParsePolyHelp(ParsingStatus *status, bool is_in_mono, Poly *p)
{
    if (status->is_correct)
    {
        int c = getc(stdin);
        if (c == MINUS || isdigit(c)) // coeff
        {
            ungetc(c, stdin);
            ParsePolyCoeff(status, is_in_mono, p);
        }
        else if (c == LBRAC) // nie coeff
            ParsePolyNonCoeff(status, is_in_mono, p);
        else
            StatusSetError(status, c);
    }
}

Poly ParsePoly(ParsingStatus *status)
{
    Poly p;
    ParsePolyHelp(status, false, &p);
    if (status->is_correct)
        return p;

    return ERROR_POLY;
}

/**
 * Parsuje indeks zmiennej.
 *
 * Ostatnim wczytanym znakiem przed wywołaniem ma być spacja po nazwie
 * instrukcji.
 *
 * Ostatnim wczytanym znakiem (o ile nie wystąpi błąd) będzie cyfra jedności
 * indeksu.
 *
 * @param[out] status : status parsowania
 * @return indeks zmiennej lub 0 w przypadku błędu
 */
static size_t ParseIndex(ParsingStatus *status)
{
    size_t idx = 0;
    size_t num_val;
    bool was_in_loop = false;
    int c;
    while (isdigit(c = getc(stdin)))
    {
        was_in_loop = true;
        num_val = c - '0';
        if (IS_IN_RANGE(idx, num_val, SIZE_MAX))
        {
            idx = 10 * idx + num_val;
        }
        else
        {
            StatusSetError(status, c);
            return 0;
        }
    }

    if (!was_in_loop)
    {
        StatusSetError(status, c);
        return 0;
    }

    ungetc(c, stdin);
    return idx;
}

/**
 * Parsuje parametr instrukcji DEG_BY.
 *
 * Ostatnim wczytanym znakiem przed wywołaniem ma być spacja po "DEG_BY".
 *
 * Ostatnim wczytanym znakiem (o ile nie wystąpi błąd) będzie '\n' lub EOF.
 *
 * @param[out] status : status parsowania
 * @return instrukcja DEG_BY z parametrem lub instrukcja błędna
 */
static Instruction ParseDegBy(ParsingStatus *status)
{
    size_t idx = ParseIndex(status);
    if (!status->is_correct)
        return ERROR_INST(ERROR_DEG_BY_VAR);

    int c = getc(stdin);
    if (c == '\n' || c == EOF)
    {
        StatusSetCorrect(status, c == EOF);
        return (Instruction){.type = DEG_BY, .idx = idx};
    }
    else
    {
        StatusSetError(status, c);
        return ERROR_INST(ERROR_DEG_BY_VAR);
    }
}

/**
 * Parsuje parametr instrukcji COMPOSE.
 *
 * Ostatnim wczytanym znakiem przed wywołaniem ma być spacja po "COMPOSE".
 *
 * Ostatnim wczytanym znakiem (o ile nie wystąpi błąd) będzie '\n' lub EOF.
 *
 * @param[out] status : status parsowania
 * @return instrukcja COMPOSE z parametrem lub instrukcja błędna
 */
static Instruction ParseCompose(ParsingStatus *status)
{
    size_t k = ParseIndex(status);
    if (!status->is_correct)
        return ERROR_INST(ERROR_COMPOSE_VAR);

    int c = getc(stdin);
    if (c == '\n' || c == EOF)
    {
        StatusSetCorrect(status, c == EOF);
        return (Instruction){.type = COMPOSE, .k = k};
    }
    else
    {
        StatusSetError(status, c);
        return ERROR_INST(ERROR_COMPOSE_VAR);
    }
}

/**
 * Parsuje parametr instrukcji AT.
 *
 * Ostatnim wczytanym znakiem przed wywołaniem ma być spacja po "AT".
 *
 * Ostatnim wczytanym znakiem (o ile nie wystąpi błąd) będzie '\n' lub EOF.
 *
 * @param[out] status : status parsowania
 * @return instrukcja AT z parametrem lub instrukcja błędna
 */
static Instruction ParseAt(ParsingStatus *status)
{
    int c = getc(stdin);
    if (!isdigit(c) && c != MINUS)
    {
        StatusSetError(status, c);
        return ERROR_INST(ERROR_AT_VAR);
    }
    ungetc(c, stdin);
    poly_coeff_t x = ParseCoeff(status);
    if (!status->is_correct)
        return ERROR_INST(ERROR_AT_VAR);

    if ((c = getc(stdin)) == '\n' || c == EOF)
    {
        StatusSetCorrect(status, c == EOF);
        return (Instruction){.type = AT, .x = x};
    }
    else
    {
        StatusSetError(status, c);
        return ERROR_INST(ERROR_AT_VAR);
    }
}

/**
 * Parsuje typ instrukcji.
 *
 * Zaczyna wczytywanie od początku nowej linii.
 *
 * Ostatnim wczytanym znakiem (o ile nie wystąpi błąd) będzie ostatnia litera
 * nazwy instrukcji.
 *
 * @param[out] status : status parsowania
 * @param[out] inst_text : typ instrukcji
 */
static void ParseInstructionText(ParsingStatus *status, char inst_text[])
{
    size_t len = 0;

    int c = getc(stdin);
    while (isupper(c) || c == UNDERSCR)
    {
        len++;

        if (len > MAX_INST_LEN)
        {
            StatusSetError(status, c);
            return;
        }

        inst_text[len - 1] = c;
        c = getc(stdin);
    }

    inst_text[len] = '\0';
    ungetc(c, stdin);
}

Instruction ParseInstruction(ParsingStatus *status)
{
    // dodatkowe miejsce na '\0'
    char inst_text[MAX_INST_LEN + 1];

    ParseInstructionText(status, inst_text);
    if (!status->is_correct)
        return ERROR_INST(ERROR_COMMAND);

    int c = getc(stdin);
    for (size_t i = 0; i < INST_NO_ARG_SIZE; i++)
    {
        if (STR_EQ(inst_text, INST_NO_ARG_LIST[i]))
        {
            if (c == '\n' || c == EOF)
            {
                StatusSetCorrect(status, c == EOF);
                return (Instruction){.type = INST_NO_ARG_LIST[i]};
            }
            else
            {
                StatusSetError(status, c);
                return ERROR_INST(ERROR_COMMAND);
            }
        }
    }

    if (STR_EQ(inst_text, DEG_BY))
    {
        if (c == SPACE)
            return ParseDegBy(status);
        else if (c == '\n' || c == EOF)
        {
            StatusSetError(status, c);
            return ERROR_INST(ERROR_DEG_BY_VAR);
        }
    }
    else if (STR_EQ(inst_text, AT))
    {
        if (c == SPACE)
            return ParseAt(status);
        else if (c == '\n' || c == EOF)
        {
            StatusSetError(status, c);
            return ERROR_INST(ERROR_AT_VAR);
        }
    }
    else if (STR_EQ(inst_text, COMPOSE))
    {
        if (c == SPACE)
            return ParseCompose(status);
        else if (c == '\n' || c == EOF)
        {
            StatusSetError(status, c);
            return ERROR_INST(ERROR_COMPOSE_VAR);
        }
    }

    StatusSetError(status, c);
    return ERROR_INST(ERROR_COMMAND);
}