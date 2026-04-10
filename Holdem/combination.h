#pragma once
enum class HandRank {
    HighCard = 0,      // Старшая карта
    Pair,              // Пара
    TwoPairs,          // Две пары
    ThreeOfAKind,      // Сет / Триплет (3 карты одного достоинства)
    Straight,          // Стрит (5 последовательных карт)
    Flush,             // Флеш (5 карт одной масти)
    FullHouse,         // Фулл-хаус (3 + 2)
    FourOfAKind,       // Каре (4 карты одного достоинства)
    StraightFlush,     // Стрит-флеш (5 последовательных одной масти)
    RoyalFlush         // Роял-флеш (от десятки до туза одной масти)
};

#include <vector>
#include <string>
#include "Card.h"

struct HandInfo {
    HandRank rank;              // Тип комбинации (из списка выше)
    std::string name;           // Название для вывода на экран (например, "Full House")
    std::vector<Card> bestFive; // 5 карт, которые образуют лучшую комбинацию
};