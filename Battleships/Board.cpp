#include "Board.hpp"


Board::Board() {
    reset();
}

void Board::reset() {
    // Toate celulele sunt goale la iceput
    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            grid[r][c] = CellState::Empty;
        }
    }
}


// Valiadare limite chenar
bool Board::isInside(int row, int col) const {

    return row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE;
}

bool Board::canPlaceShip(int row, int col, int length, bool horizontal) const {
    if (!isInside(row, col)) return false;

    if (horizontal) {
        if (col + length > BOARD_SIZE) return false;
        for (int j = col; j < col + length; ++j) {
            if (grid[row][j] != CellState::Empty) return false;
        }
    }
    else {
        if (row + length > BOARD_SIZE) return false;

        for (int i = row; i < row + length; ++i) {
            if (grid[i][col] != CellState::Empty) return false;
        }
    }

    return true;
}

bool Board::placeShip(int row, int col, int length, bool horizontal) {
    // Plasare post validare
    if (!canPlaceShip(row, col, length, horizontal)) return false;

    if (horizontal) {
        for (int j = col; j < col + length; ++j) {
            grid[row][j] = CellState::Ship;
        }
    }
    else {
        for (int i = row; i < row + length; ++i) {
            grid[i][col] = CellState::Ship;
        }
    }

    return true;
}

CellState Board::hit(int row, int col) {

    if (!isInside(row, col)) return CellState::Empty;

    // Nava lovita = Hit
    if (grid[row][col] == CellState::Ship) {
        grid[row][col] = CellState::Hit;
        return CellState::Hit;
    }

    // Nava ratata/ lovit empty cell = Miss
    if (grid[row][col] == CellState::Empty) {
        grid[row][col] = CellState::Miss;
        return CellState::Miss;
    }

    // Daca e deja Hit/Miss ramane la fel
    return grid[row][col];
}

CellState Board::getCell(int row, int col) const {
    // Protectie la acces invalid
    if (!isInside(row, col)) return CellState::Empty;
    return grid[row][col];
}

bool Board::allShipsSunk() const {
    // Verifica sa fie lovite toate celulele care cotin o nava
    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            if (grid[r][c] == CellState::Ship) return false;
        }
    }
    return true;
}
