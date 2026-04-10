#pragma once
#pragma once
#include <vector>
#include <algorithm> // Для std::shuffle
#include <random>    // Для генерации случайных чисел
#include "Card.h"

class Deck {
public:
    // Конструктор: создает стандартную колоду из 52 карт
    Deck();

    // Перемешать колоду
    void shuffle();

    // Взять одну карту сверху колоды
    Card drawCard();

    // Убрать конкретную карту из колоды (важно для инициализации 
    // перед расчетом шансов, чтобы знать, какие карты остались)
    void removeCard(Rank r, Suit s);

    // Вернуть все карты в колоду и подготовить к новой раздаче
    void reset();

    // Узнать, сколько карт осталось в колоде
    size_t cardsLeft() const;

private:
    std::vector<Card> cards;
    std::default_random_engine rd; // Движок для рандома
};