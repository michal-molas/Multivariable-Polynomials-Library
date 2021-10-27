/** @file
  Biblioteka wczytująca input do kalkulatora wielomianów

  @authors Michał Molas
  <mm429570@students.mimuw.edu.pl>
  @copyright Uniwersytet Warszawski
  @date 2021
*/

#ifndef __CALC_PARSE_H__
#define __CALC_PARSE_H__

#include "poly.h"
#include "string.h"

/** sprawdza, czy napisy są równe */
#define STR_EQ(str1, str2) (strcmp(str1, str2) == 0)

/** Wartość zwracana w przypadku wczytania błędnej nazwy instrukcji oraz tekst
 * wypisywanego błędu */
#define ERROR_COMMAND "WRONG COMMAND"
/** Wartość zwracana w przypadku wczytania błędnego parametru instrukcji AT oraz
 * tekst wypisywanego błędu */
#define ERROR_AT_VAR "AT WRONG VALUE"
/** Wartość zwracana w przypadku wczytania błędnego parametru instrukcji DEG_BY
 * oraz tekst wypisywanego błędu */
#define ERROR_DEG_BY_VAR "DEG BY WRONG VARIABLE"
/** Wartość zwracana w przypadku wczytania błędnego parametru instrukcji COMPOSE
 * oraz tekst wypisywanego błędu */
#define ERROR_COMPOSE_VAR "COMPOSE WRONG PARAMETER"
/** Wartość zwracana w przypadku błędu braku wystarczającej liczby elementów na
 * stosie oraz tekst wypisywanego błędu */
#define ERROR_STACK_UNDERFLOW "STACK UNDERFLOW"
/** Wartość zwracana w przypadku wczytania błędnego wielomianu oraz tekst
 * wypisywanego błędu */
#define ERROR_WRONG_POLY "WRONG POLY"

/** Nazwa instrukcji ZERO */
#define ZERO "ZERO"
/** Nazwa instrukcji IS_COEFF */
#define IS_COEFF "IS_COEFF"
/** Nazwa instrukcji IS_ZERO */
#define IS_ZERO "IS_ZERO"
/** Nazwa instrukcji CLONE */
#define CLONE "CLONE"
/** Nazwa instrukcji NEG */
#define NEG "NEG"
/** Nazwa instrukcji DEG */
#define DEG "DEG"
/** Nazwa instrukcji DEG_BY */
#define DEG_BY "DEG_BY"
/** Nazwa instrukcji AT */
#define AT "AT"
/** Nazwa instrukcji PRINT */
#define PRINT "PRINT"
/** Nazwa instrukcji POP */
#define POP "POP"
/** Nazwa instrukcji ADD */
#define ADD "ADD"
/** Nazwa instrukcji MUL */
#define MUL "MUL"
/** Nazwa instrukcji SUB */
#define SUB "SUB"
/** Nazwa instrukcji IS_EQ */
#define IS_EQ "IS_EQ"
/** Nazwa instrukcji COMPOSE */
#define COMPOSE "COMPOSE"

/**
 * Struktura przechowująca dane o aktualnym statusie parsowania.
 */
typedef struct
{
    /** czy ostatnim wczytanym znakiem był EOF */
    bool is_eof;
    /** czy ostatnim wczytanym znakiem był '\n' */
    bool is_eol;
    /** czy wczytywanie ostatniego znaku zakończyło się sukcesem */
    bool is_correct;
} ParsingStatus;

/**
 * Struktura przechowująca informacje o wczytanej instrukcji.
 */
typedef struct
{
    /** typ instrukcji lub nazwa błędu w przypadku nieprawidłowego wczytania */
    const char *type;
    union
    {
        /** parametr do instrukcji DEG_BY */
        size_t idx;
        /** parametr do instrukcji AT */
        poly_coeff_t x;
        /** parametr do instrukcji COMPOSE */
        size_t k;
    };
} Instruction;

/**
 * Tworzy nowy status parsowania (::ParsingStatus).
 * @return nowy status parsowania
 */
ParsingStatus NewParsingStatus();

/**
 * Parsuje instrukcję ze standardowego wejścia.
 *
 * Zaczyna wczytywać od początku nowej linii, więc należy cofnąć (ungetc)
 * wczytany znak przed wywołaniem.
 *
 * @param[in,out] status : status parsowania
 * @return wczytana instrukcja
 */
Instruction ParseInstruction(ParsingStatus *status);

/**
 * Parsuje wielomian ze standardowego wejścia.
 *
 * Zaczyna wczytywać od początku nowej linii, więc należy cofnąć (ungetc)
 * wczytany znak przed wywołaniem.
 *
 * @param[in,out] status : status parsowania
 * @return wczytany wielomian
 */
Poly ParsePoly(ParsingStatus *status);

#endif