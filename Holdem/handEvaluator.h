#pragma once
#include <vector>
#include "card.h"
#include "combination.h"

class HandEvaluator {
public:
    // Принимает 5-7 карт и возвращает лучшую комбинацию из них
    static HandInfo evaluate(const std::vector<Card>& allCards);

    // Сравнивает две руки (нужно для симуляции игр против оппонентов)
    // Возвращает 1 если первая рука сильнее, -1 если вторая, 0 если ничья
    static int compareHands(const std::vector<Card>& hand1, const std::vector<Card>& hand2);

private:
    // Внутренние проверки (ты будешь писать их реализацию в .cpp)
    static bool hasFlush(const std::vector<Card>& cards);
    static bool hasStraight(const std::vector<Card>& cards);
    static bool hasFullHouse(const std::vector<Card>& cards);
};