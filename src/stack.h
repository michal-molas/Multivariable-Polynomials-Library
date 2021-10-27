/** @file
  Biblioteka struktury stosu wielomianów

  @authors Michał Molas
  <mm429570@students.mimuw.edu.pl>
  @copyright Uniwersytet Warszawski
  @date 2021
*/

#ifndef __STACK_H__
#define __STACK_H__

#include "poly.h"

/**
 * Stos wielomianów.
 * Zaimplementowany jako tablica dynamiczna, ze względu na brak korzyści z
 * implementacji listowej.
 */
typedef struct
{
    /** tablica zawierająca elementy stosu */
    Poly *polies;
    /** liczba elementów na stosie */
    size_t size;
    /** liczba zaalokowanej pamięci w tablicy */
    size_t max_size;
} Stack;

/**
 * Usuwa wszystkie wielomiany ze stosu, usuwa je z pamięci, zwalnia pamięć
 * stosu.
 * @param[out] s : stos wielomianów
 */
void StackDestroy(Stack *s);

/**
 * Zwraca wskaźnik na wielomian z wierzchołka stosu.
 * Uwaga: Nie należy zwalniać pamięci z pobranego wielomianu
 * @param[in] s : stos wielomianów
 * @return wielomian z wierzchołka stosu
 */
Poly *StackPeek(const Stack *s);

/**
 * Zwraca wskaźnik na drugi element stosu.
 * @param[in] s : stos wielomianów
 * @return drugi wielomian ze stosu
 */
Poly *StackPeekNext(const Stack *s);

/**
 * Zwraca wskaźnik na wielomian z wierzchołku stosu, zmniejsza rozmiar stosu.
 * Uwaga: Użytkownik jest odpowiedzialny za ewentualne zwolnienie pamięci
 * pobranego wielomianu
 * @param[in,out] s : stos wielomianów
 * @return wielomian z wierzchołka stosu
 */
Poly *StackPop(Stack *s);

/**
 * Dodaje wielomian @f$p@f$ na wierzchołek stosu.
 * @param[out] s : stos wielomianów
 * @param[in] p : wielomian @f$p@f$
 */
void StackPush(Stack *s, Poly p);

/**
 * Dodaje wielomian na wierzchołku stosu na wielomian @f$p@f$. Zwalnia pamięć
 * pierwotnego wielomianu
 * @param[out] s : stos wielomianów
 * @param[in] p : wielomian @f$p@f$
 */
void StackChangeTop(Stack *s, Poly p);

/**
 * Inicjalizuje i zwraca pusty stos.
 * @return pusty stos
 */
Stack StackNewEmpty();

/**
 * Sprawdza czy stos jest pusty.
 * @param[in] s : stos wielomianów
 * @return czy stos jest pusty
 */
bool StackIsEmpty(const Stack *s);

/**
 * Sprawdza czy stos ma conajwyżej jeden element.
 * @param[in] s : stos wielomianów
 * @return czy stos ma conajwyżej jeden element
 */
bool StackIsAlmostEmpty(const Stack *s);

/**
 * Sprawdza czy stos ma conajmniej @p n elementów
 * @param[in] s : stos wielomianów
 * @param[in] n : liczba elementów
 * @return czy stos ma conajmniej @p n elementów
 */
bool StackHasEnoughElements(const Stack *s, size_t n);

/**
 * Zwraca tablicę (wskaźnik na k-ty element od wierzchołka) pierwszych @p k
 * wielomianów ze stosu @p s
 *
 * Uwagi:
 * - Użytkownik jest odpowiedzialny za zwolnienie pamięci z pobranych
 * wielomianów.
 * - Należy upewnić się, że na stosie jest wystarczająca liczba wielomianów
 * przed wykonaniem
 * @param[in,out] s : stos wielomianów
 * @param[in] k : liczba wielomianów do pobrania
 * @return tablica @p k pierwszych wielomianów ze stosu
 */
Poly *StackPopK(Stack *s, size_t k);

#endif