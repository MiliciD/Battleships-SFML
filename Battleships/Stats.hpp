#pragma once
#include <string>
#include <vector>
#include "Game.hpp"


// Scrie rezultatele intr-un fisier CSV
// Citeste istoricul si calculeaza totaluri


// Structura reprezinta o intrare din istoric 
struct HistoryEntry {
    std::string text;   // textul afisat
};

// Clasa Stats: utilitare statice pentru istoric
class Stats {
public:
    // Adauga un rezultat in CSV 
    static void appendResult(bool playerWon, Difficulty diff);

    // Citeste istoricul si il transforma intr-o lista de linii pentru UI
    static std::vector<HistoryEntry> readHistory();

    // Calculeaza totalul de win/lose
    static void computeTotals(int& wins, int& losses);

private:
    // Path spre fisierul CSV
    static const std::string FILE_PATH;

    // Converteste dificultatea in text
    static std::string difficultyToString(Difficulty diff);
};
