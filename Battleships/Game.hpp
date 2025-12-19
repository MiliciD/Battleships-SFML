#pragma once
#include "Board.hpp"
#include "Ships.hpp"
#include <vector>
#include <optional>
#include <utility>


// - Ordoneaza logica pe stari (menu / placing / playing / game over)
// - Contine AI-ul


// Dificultatea jocului
enum class Difficulty { Easy, Smarter };

// Starile principale ale aplicatiei
enum class GameState { MainMenu, History, PlacingShips, Playing, GameOver };

// Informatii despre ultima lovitura
struct ShotInfo {
    int r = -1;
    int c = -1;
    CellState before = CellState::Empty;
    CellState after = CellState::Empty;
};

//  Game logica joc + AI + stari
class Game {
public:
    // jucatorul (stanga) si ai-ul (dreapta)
    Board player;
    Board computer;

    // Starea curenta 
    GameState state = GameState::MainMenu;

    // Dificultatea selectata 
    Difficulty difficulty = Difficulty::Easy;

    // win check
    bool playerWon = false;

    // Ultimele lovituri folosite pentru animatii
    std::optional<ShotInfo> lastPlayerShot;
    std::optional<ShotInfo> lastComputerShot;

    Game();

    // Porneste o noua runda
    void startNewGame(Difficulty diff);

    // Revine la meniul principal
    void resetToMenu();

    // Plasare nave (manual la jucator)
    bool hasShipsToPlace() const;
    const ShipDef& currentShip() const;
    void rotateCurrentShip();
    bool placeCurrentShip(int r, int c);

    // Tura jucatorului
    bool playerShoot(int r, int c);

private:
    // Lista de nave ce trebuie plasate
    std::vector<ShipDef> ships;

    // Indexul navei
    size_t shipIndex = 0;

    // Lista de tinte pentru AI Smarter
    std::vector<std::pair<int, int>> aiTargets;

    // Pregateste tablele si plaseaza navele computerului
    void setupBoards();

    // Tura computerului
    void computerShoot();

    // Check ca o celula sa nu mai fi fost lovita (sa fie available)
    bool isShootable(CellState st) const;

    // Verificare vecini
    void aiAddNeighbors(int r, int c);

    // Alege tinta in functie de dificultate
    std::pair<int, int> aiChooseTarget();
};
