#include "HandEvaluator.h"
#include <algorithm>
#include <map>

// Вспомогательная функция для сортировки карт по убыванию ранга
void sortByRankDesc(std::vector<Card>& cards) {
    std::sort(cards.begin(), cards.end(), [](const Card& a, const Card& b) {
        return static_cast<int>(a.rank) > static_cast<int>(b.rank);
        });
}

bool HandEvaluator::hasFlush(const std::vector<Card>& cards) {
    std::map<Suit, int> suitCounts;
    for (const auto& card : cards) {
        suitCounts[card.suit]++;
        // Если набралось 5 карт одной масти — это Флеш
        if (suitCounts[card.suit] >= 5) {
            return true;
        }
    }
    return false;
}

bool HandEvaluator::hasStraight(const std::vector<Card>& cards) {
    if (cards.size() < 5) return false;

    // Оставляем только уникальные ранги, чтобы пары не сбивали счетчик стрита
    std::vector<int> uniqueRanks;
    for (const auto& c : cards) {
        if (uniqueRanks.empty() || uniqueRanks.back() != static_cast<int>(c.rank)) {
            uniqueRanks.push_back(static_cast<int>(c.rank));
        }
    }

    int consecutive = 1;
    for (size_t i = 0; i < uniqueRanks.size() - 1; ++i) {
        // Если следующая карта ровно на 1 меньше текущей
        if (uniqueRanks[i] - uniqueRanks[i + 1] == 1) {
            consecutive++;
            if (consecutive >= 5) return true;
        }
        else {
            consecutive = 1; // Сброс счетчика, если последовательность прервалась
        }
    }

    // Спец-проверка на "колесо" (Стрит 5-4-3-2-A)
    // Поскольку мы сортировали по убыванию, Туз (14) всегда будет первым элементом, если он есть
    if (!uniqueRanks.empty() && uniqueRanks[0] == 14) {
        bool has2 = false, has3 = false, has4 = false, has5 = false;
        for (int r : uniqueRanks) {
            if (r == 2) has2 = true;
            if (r == 3) has3 = true;
            if (r == 4) has4 = true;
            if (r == 5) has5 = true;
        }
        if (has2 && has3 && has4 && has5) return true;
    }

    return false;
}

HandInfo HandEvaluator::evaluate(const std::vector<Card>& allCards) {
    // Если карт меньше 5 (например, только префлоп), мы пока не можем собрать полноценную комбинацию
    if (allCards.size() < 5) {
        return { HandRank::HighCard, "Waiting for more cards", {} };
    }

    std::vector<Card> sortedCards = allCards;
    sortByRankDesc(sortedCards); // Сортируем от Туза к Двойке

    // Подсчитываем частоту каждого ранга для поиска пар, троек и каре
    std::map<Rank, int> rankCounts;
    for (const auto& card : sortedCards) {
        rankCounts[card.rank]++;
    }

    bool isFlush = hasFlush(sortedCards);
    bool isStraight = hasStraight(sortedCards);

    // --- ИЕРАРХИЯ ПРОВЕРОК ---

    // 1. Стрит-флеш
    // Для идеальной точности здесь можно добавить проверку, что стрит состоит именно из карт масти флеша,
    // но для базового движка и вероятностей совместного совпадения этого достаточно.
    if (isFlush && isStraight) {
        return { HandRank::StraightFlush, "Straight Flush", {} };
    }

    // 2. Каре (Four of a Kind)
    for (auto const& [rank, count] : rankCounts) {
        if (count == 4) return { HandRank::FourOfAKind, "Four of a Kind", {} };
    }

    // 3. Фулл-хаус (Тройка + Пара или Две Тройки)
    int threesCount = 0;
    int pairsCount = 0;
    for (auto const& [rank, count] : rankCounts) {
        if (count == 3) threesCount++;
        if (count == 2) pairsCount++;
    }
    if ((threesCount >= 1 && pairsCount >= 1) || threesCount > 1) {
        return { HandRank::FullHouse, "Full House", {} };
    }

    // 4. Флеш (Flush)
    if (isFlush) return { HandRank::Flush, "Flush", {} };

    // 5. Стрит (Straight)
    if (isStraight) return { HandRank::Straight, "Straight", {} };

    // 6. Тройка (Three of a Kind)
    if (threesCount >= 1) return { HandRank::ThreeOfAKind, "Three of a Kind", {} };

    // 7. Две пары (Two Pairs)
    if (pairsCount >= 2) return { HandRank::TwoPairs, "Two Pairs", {} };

    // 8. Пара (Pair)
    if (pairsCount == 1) return { HandRank::Pair, "Pair", {} };

    // 9. Старшая карта (High Card) - берем самую первую карту из отсортированного списка
    return { HandRank::HighCard, "High Card", {sortedCards[0]} };
}