#pragma once
#include <string>
#include "card.h"

std::string Card::toString() const {
    std::string r, s;

    switch (this->rank) {
    case Rank::Two:   r = "2"; break;
    case Rank::Three: r = "3"; break;
    case Rank::Four:  r = "4"; break;
    case Rank::Five:  r = "5"; break;
    case Rank::Six:   r = "6"; break;
    case Rank::Seven: r = "7"; break;
    case Rank::Eight: r = "8"; break;
    case Rank::Nine:  r = "9"; break;
    case Rank::Ten:   r = "10"; break;
    case Rank::Jack:  r = "Jack"; break;
    case Rank::Queen: r = "Queen"; break;
    case Rank::King:  r = "King"; break;
    case Rank::Ace:   r = "Ace"; break;
    default:          r = "?"; break;
    }

    switch (this->suit) {
    case Suit::Spades:   s = "Spades"; break;   // или "♠"
    case Suit::Hearts:   s = "Hearts"; break;   // или "♥"
    case Suit::Diamonds: s = "Diamonds"; break; // или "♦"
    case Suit::Clubs:    s = "Clubs"; break;    // или "♣"
    default:             s = "?"; break;
    }

    return r + " of " + s;
}