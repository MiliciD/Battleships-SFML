#pragma once
#include <string>
#include <vector>
#include "Board.hpp"



struct ShipDef {
    std::string name;   // nume folosit optional 
    int length;         // cate celule ocupa
    bool horizontal;    // true = orizontal, false = vertical
};

// Returneaza lista standard de nave a jocului
std::vector<ShipDef> getShipTypes();

// Plaseaza toate navele random pe tabla data
void placeShipsRandom(Board& board, const std::vector<ShipDef>& ships);
