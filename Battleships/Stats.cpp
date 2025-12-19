#include "Stats.hpp"
#include <fstream>
#include <ctime>
#include <sstream>



// Scriere/citire istoric in format CSV

// Fisierul este in folderul assets
const std::string Stats::FILE_PATH = "assets/stats.csv";

// Format CSV:
// timestamp,result,difficulty
void Stats::appendResult(bool playerWon, Difficulty diff) {
    std::ofstream file(FILE_PATH, std::ios::app);
    if (!file.is_open()) return;

    // Timpul curent (time_t)
    std::time_t now = std::time(nullptr);

    // Structura tm pentru data/ora locala
    std::tm tmBuf{};
    localtime_s(&tmBuf, &now);

    // Convertim timpul intr-un string (YYYY-MM-DD HH:MM:SS)
    char buf[64]{};
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tmBuf);

    // Scriere in CSV
    file << buf << ","
        << (playerWon ? "Victorie" : "Infrangere") << ","
        << difficultyToString(diff)
        << "\n";
}

std::vector<HistoryEntry> Stats::readHistory() {
    std::vector<HistoryEntry> entries;

    std::ifstream file(FILE_PATH);
    if (!file.is_open()) return entries;

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);

        std::string timestamp, result, diff;
        std::getline(ss, timestamp, ',');
        std::getline(ss, result, ',');
        std::getline(ss, diff, ',');

        HistoryEntry e;
        e.text = "User vs Computer - " + result + " - " + diff;
        entries.push_back(e);
    }

    return entries;
}

void Stats::computeTotals(int& wins, int& losses) {
    // Calculeaza totalurile cautand cuvintele cheie in fiecare linie
    wins = 0;
    losses = 0;

    std::ifstream file(FILE_PATH);
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        if (line.find("Victorie") != std::string::npos) {
            wins++;
        }
        else if (line.find("Infrangere") != std::string::npos) {
            losses++;
        }
    }
}

std::string Stats::difficultyToString(Difficulty diff) {
    // Textul trebuie sa fie consistent cu ce afisam in UI
    return (diff == Difficulty::Easy) ? "Usor" : "Ceva mai complicat";
}
