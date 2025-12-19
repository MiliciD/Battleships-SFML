#include "Game.hpp"
#include <cstdlib>


// Implementarea logicii de joc si a AI-ului

// Helper: construieste un ShotInfo din parametri
static ShotInfo makeShot(int r, int c, CellState before, CellState after) {
    ShotInfo s;
    s.r = r;
    s.c = c;
    s.before = before;
    s.after = after;
    return s;
}

Game::Game() {
    // Preia lista standard de nave
    ships = getShipTypes();

    // Se porneste aplicatia in meniul principal
    resetToMenu();
}

void Game::resetToMenu() {
    // Se reseteaza starea si datele rundei
    state = GameState::MainMenu;
    playerWon = false;

    shipIndex = 0;
    aiTargets.clear();

    lastPlayerShot.reset();
    lastComputerShot.reset();
}

void Game::startNewGame(Difficulty diff) {
    // Setare dificultate + runda noua
    difficulty = diff;
    setupBoards();
    state = GameState::PlacingShips;
}

void Game::setupBoards() {
    // Reset tablele
    player.reset();
    computer.reset();

    // Reset starea de plasare
    shipIndex = 0;
    aiTargets.clear();

    lastPlayerShot.reset();
    lastComputerShot.reset();

    // Computerul plaseaza navele random
    placeShipsRandom(computer, ships);
}

bool Game::hasShipsToPlace() const {
    return shipIndex < ships.size();
}

const ShipDef& Game::currentShip() const {
    // IMPORTANT: se apeleaza doar cand hasShipsToPlace() este true
    return ships[shipIndex];
}

void Game::rotateCurrentShip() {
    // se roteste nava
    ships[shipIndex].horizontal = !ships[shipIndex].horizontal;
}

bool Game::placeCurrentShip(int r, int c) {
    // Plasarea navelor se face doar in starea PlacingShips
    if (state != GameState::PlacingShips) return false;
    if (!hasShipsToPlace()) return false;

    const ShipDef& sh = ships[shipIndex];

    // Se plaseaza navele + validare
    if (player.placeShip(r, c, sh.length, sh.horizontal)) {
        shipIndex++;

        // dupa plasare incepe partea de joc 
        if (!hasShipsToPlace()) state = GameState::Playing;
        return true;
    }

    return false;
}

bool Game::isShootable(CellState st) const {
    // doar celulele empty/ship se pot ataca, nu poti ataca de 2 ori in acelasi loc
    return (st == CellState::Empty || st == CellState::Ship);
}

std::pair<int, int> Game::aiChooseTarget() {
    // verifica tintele vecine
    if (difficulty == Difficulty::Smarter) {
        while (!aiTargets.empty()) {
            std::pair<int, int> p = aiTargets.back();
            aiTargets.pop_back();

            
            if (isShootable(player.getCell(p.first, p.second))) return p;
        }
    }

    // random pana gaseste o celula valida
    while (true) {
        int r = std::rand() % BOARD_SIZE;
        int c = std::rand() % BOARD_SIZE;
        if (isShootable(player.getCell(r, c))) return std::pair<int, int>(r, c);
    }
}

void Game::aiAddNeighbors(int r, int c) {
    // Dupa un Hit se incerca vecinii in cruce (sus/jos/stanga/dreapta)
    const int dr[4] = { -1, 1, 0, 0 };
    const int dc[4] = { 0, 0, -1, 1 };

    for (int k = 0; k < 4; ++k) {
        int rr = r + dr[k];
        int cc = c + dc[k];

        if (!player.isInside(rr, cc)) continue;

        if (isShootable(player.getCell(rr, cc))) {
            aiTargets.push_back(std::pair<int, int>(rr, cc));
        }
    }
}

bool Game::playerShoot(int r, int c) {
    // Tragerile se fac doar in starea Playing
    if (state != GameState::Playing) return false;

    // Resetam ultimele lovituri (vom seta altele noi in cadrul acestei runde)
    lastPlayerShot.reset();
    lastComputerShot.reset();

    // Se ignora hituri subsecvente in aceeasi celula
    CellState before = computer.getCell(r, c);
    if (!isShootable(before)) return false;

    // Lovitura pe tabla ai-ului
    CellState after = computer.hit(r, c);
    lastPlayerShot = makeShot(r, c, before, after);

    // Check pentru win state
    if (computer.allShipsSunk()) {
        state = GameState::GameOver;
        playerWon = true;
        return true;
    }
    computerShoot();

    // Check pentru lose state
    if (player.allShipsSunk()) {
        state = GameState::GameOver;
        playerWon = false;
    }

    return true;
}

void Game::computerShoot() {
    // Computerul alege o tinta
    std::pair<int, int> p = aiChooseTarget();
    int r = p.first;
    int c = p.second;

    // loveste tabla user-ului
    CellState before = player.getCell(r, c);
    CellState after = player.hit(r, c);

    lastComputerShot = makeShot(r, c, before, after);

    if (difficulty == Difficulty::Smarter && after == CellState::Hit) {
        aiAddNeighbors(r, c);
    }
}
