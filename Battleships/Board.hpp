#pragma once
#include <cstdint>


// Reprezinta tabla de joc 10x10
// Pastreaza starea celulelor (hit/miss/ship/empty)


static constexpr int BOARD_SIZE = 10;

// Starea celulei
enum class CellState : std::uint8_t {
    Empty,  // apa / gol
    Ship,   // nava
    Hit,    // nava lovita
    Miss    // lovitura ratata
};

class Board {
private:
  // Grid de 10x10
    CellState grid[BOARD_SIZE][BOARD_SIZE];

public:
    // Construieste o tabla goala
    Board();

    // Reseteaza tabla la starea initiala
    void reset();

    // Validare la linii si coloane
    bool isInside(int row, int col) const;

// Validare la nave
    bool canPlaceShip(int row, int col, int length, bool horizontal) const;

    bool placeShip(int row, int col, int length, bool horizontal);


    CellState hit(int row, int col);

    // Citeste starea celulei
    CellState getCell(int row, int col) const;

    // Returneaza true daca toate celulele "ship" sunt "hit"
    bool allShipsSunk() const;
};
