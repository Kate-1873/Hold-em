#include <iostream>
#include <vector>
#include <string>
#include <Windows.h>
#include "PokerEngine.h"

// Функция для быстрого создания карты (чтобы не писать много кода)
void addCardByInput(PokerEngine& engine, bool toTable) {
    int r, s;
    std::cout << "Введите ранг (2-14, где 11-J, 14-A): ";
    std::cin >> r;
    std::cout << "Введите масть (0-Spades, 1-Hearts, 2-Diamonds, 3-Clubs): ";
    std::cin >> s;

    if (toTable) engine.addTableCard(static_cast<Rank>(r), static_cast<Suit>(s));
    else engine.addPlayerCard(static_cast<Rank>(r), static_cast<Suit>(s));
}

int main() {
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    std::cout << "=== POKER PREDICTOR: TEST DRIVE ===\n";

    int players;
    std::cout << "Сколько игроков за столом (включая вас)? ";
    std::cin >> players;

    PokerEngine engine(players);

    // Вводим ваши 2 карты
    std::cout << "\n--- Введите ваши 2 карты ---\n";
    addCardByInput(engine, false);
    addCardByInput(engine, false);

    // Вводим карты стола (Флоп)
    std::cout << "\n--- Введите 3 карты стола (Флоп) ---\n";
    addCardByInput(engine, true);
    addCardByInput(engine, true);
    addCardByInput(engine, true);

    std::cout << "\n[!] Считаю вероятности (10,000 симуляций)..." << std::endl;

    PokerEngine::Analysis result = engine.getFullAnalysis();

    std::cout << "\n=====================================" << std::endl;
    std::cout << "ТЕКУЩАЯ КОМБИНАЦИЯ: " << result.currentCombo << std::endl;
    std::cout << "ШАНС НА ПОБЕДУ:     " << result.winChance << "%" << std::endl;
    std::cout << "РЕКОМЕНДАЦИЯ:       " << result.action << std::endl;
    std::cout << "=====================================" << std::endl;

    std::cout << "\nНажмите Enter, чтобы выйти...";
    std::cin.ignore();
    std::cin.get();

    return 0;
}