#pragma once
#include "Deck.h"
#include <algorithm>
#include <random>
#include <ctime>

Deck::Deck() {
    // Инициализируем генератор случайных чисел текущим временем
    rd.seed(static_cast<unsigned int>(std::time(nullptr)));
    reset();
}

void Deck::reset() {
    cards.clear();
    // Заполняем колоду: 4 масти по 13 рангов
    for (int s = 0; s < 4; ++s) {
        for (int r = 2; r <= 14; ++r) {
            cards.push_back({ static_cast<Rank>(r), static_cast<Suit>(s) });
        }
    }
}

void Deck::shuffle() {
    std::shuffle(cards.begin(), cards.end(), rd);
}

Card Deck::drawCard() {
    if (cards.empty()) {
        return { Rank::Two, Suit::Spades };
    }
    Card card = cards.back();
    cards.pop_back(); // Удаляем карту с "верха" колоды
    return card;
}

void Deck::removeCard(Rank r, Suit s) {
    // Ищем конкретную карту и удаляем её из вектора
    // Это нужно, когда пользователь ввел свои карты вручную
    cards.erase(std::remove_if(cards.begin(), cards.end(),
        [r, s](const Card& c) {
            return c.rank == r && c.suit == s;
        }), cards.end());
}

size_t Deck::cardsLeft() const {
    return cards.size();
}