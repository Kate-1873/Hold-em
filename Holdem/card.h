#pragma once
#include <string>

enum class Suit { Spades, Hearts, Diamonds, Clubs };
enum class Rank {
    Two = 2, Three, Four, Five, Six, Seven, Eight, Nine, Ten,
    Jack, Queen, King, Ace
};

struct Card {
    Rank rank;
    Suit suit;

    std::string toString() const;
};