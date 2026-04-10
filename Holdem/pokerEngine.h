#pragma once
#pragma once
#include <vector>
#include "card.h"
#include "deck.h"
#include "handEvaluator.h"

class PokerEngine {
public:
    // Инициализация: сколько человек за столом
    PokerEngine(int playersCount);

    // Методы для добавления карт (когда напарник нажимает кнопки в UI)
    void addPlayerCard(Rank r, Suit s);
    void addTableCard(Rank r, Suit s);

    // Главная функция для UI: получить полный анализ текущей ситуации
    struct Analysis {
        double winChance;           // Шанс на победу в %
        std::string currentCombo;    // Название текущей руки
        std::string nextBestCombo;   // Что мы пытаемся собрать (например, "Шанс на Флеш")
        std::string action;          // Рекомендация (Fold/Call/Raise)
    };

    Analysis getFullAnalysis();

    void resetGame(); // Очистить стол для новой раздачи

private:
    int m_playersCount;
    std::vector<Card> m_playerHand;
    std::vector<Card> m_tableCards;
    Deck m_deck;

    // Метод Монте-Карло (запускает тысячи симуляций внутри)
    double runMonteCarlo(int simulations = 10000);
};