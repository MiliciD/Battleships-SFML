#include "Ships.hpp"
#include <cstdlib>


// Functii helper pentru nave


std::vector<ShipDef> getShipTypes() {
    // Lista navelor + lungime + orientare 
    std::vector<ShipDef> s;
    s.push_back({ "Nava 3 (H)", 3, true });
    s.push_back({ "Nava 4 (H)", 4, true });
    s.push_back({ "Nava 2 (H)", 2, true });
    s.push_back({ "Nava 2 (V)", 2, false });
    s.push_back({ "Nava 3 (V)", 3, false });
    return s;
}

void placeShipsRandom(Board& board, const std::vector<ShipDef>& ships) {
    // Validare pozitie pt nave
    for (size_t i = 0; i < ships.size(); ++i) {
        const auto& sh = ships[i];

        bool placed = false;
        while (!placed) {
            int r = std::rand() % BOARD_SIZE;
            int c = std::rand() % BOARD_SIZE;
            placed = board.placeShip(r, c, sh.length, sh.horizontal);
        }
    }
}
