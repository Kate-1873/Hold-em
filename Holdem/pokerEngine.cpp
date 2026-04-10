#pragma once
#include "PokerEngine.h"
#include <iostream>

PokerEngine::PokerEngine(int playersCount) : m_playersCount(playersCount) {
    m_deck.reset();
}

void PokerEngine::addPlayerCard(Rank r, Suit s) {
    m_playerHand.push_back({ r, s });
    m_deck.removeCard(r, s);
}

void PokerEngine::addTableCard(Rank r, Suit s) {
    m_tableCards.push_back({ r, s });
    m_deck.removeCard(r, s);
}

double PokerEngine::runMonteCarlo(int simulations) {
    int wins = 0;

    for (int i = 0; i < simulations; ++i) {
        Deck tempDeck = m_deck; // Копируем текущее состояние колоды
        tempDeck.shuffle();

        // 1. Досдаем карты на стол, если их меньше 5
        std::vector<Card> tempTable = m_tableCards;
        while (tempTable.size() < 5) {
            tempTable.push_back(tempDeck.drawCard());
        }

        // 2. Оцениваем нашу руку
        std::vector<Card> myFullHand = m_playerHand;
        myFullHand.insert(myFullHand.end(), tempTable.begin(), tempTable.end());
        HandInfo myResult = HandEvaluator::evaluate(myFullHand);

        // 3. Симулируем руки оппонентов
        bool lost = false;
        for (int p = 0; p < m_playersCount - 1; ++p) {
            std::vector<Card> oppHand = { tempDeck.drawCard(), tempDeck.drawCard() };
            oppHand.insert(oppHand.end(), tempTable.begin(), tempTable.end());
            HandInfo oppResult = HandEvaluator::evaluate(oppHand);

            if (static_cast<int>(oppResult.rank) > static_cast<int>(myResult.rank)) {
                lost = true;
                break;
            }
        }

        if (!lost) wins++;
    }

    return (static_cast<double>(wins) / simulations) * 100.0;
}

PokerEngine::Analysis PokerEngine::getFullAnalysis() {
    Analysis res;

    // Текущая комбинация
    std::vector<Card> current = m_playerHand;
    current.insert(current.end(), m_tableCards.begin(), m_tableCards.end());
    res.currentCombo = HandEvaluator::evaluate(current).name;

    // Считаем вероятность
    res.winChance = runMonteCarlo(10000);

    // Логика рекомендаций
    if (res.winChance > 60.0) res.action = "RAISE (Агрессивно повышайте)";
    else if (res.winChance > 30.0) res.action = "CALL (Поддержите ставку)";
    else res.action = "FOLD (Сбрасывайте карты)";

    return res;
}