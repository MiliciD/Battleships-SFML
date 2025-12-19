# Battleships (SFML 3) – C++

Jocul Battleships implementat in C++ 17 + SFML 3, cu interfata grafica, AI pe doua dificultati, audio si istoric al rezultatelor.

---

# Functionalitati
- Meniu principal
- Doua dificultati: Usor si Ceva mai complicat
- Plasare manuala a navelor
- User vs AI
- Muzica de fundal + efecte sonore
- Sunete dedicate pentru victorie / infrangere
- Slider de volum
- Istoric rezultate

---

# Tehnologii utilizate
- C++ 17
- SFML 3
- Visual Studio 2022

---

# Controale
- Click pe butoane: Usor / Ceva mai complicat / Istoric
- Plasare nave: click pe tabla din stanga
- Rotire nava: R
- Atac: click pe tabla din dreapta
- Inapoi la meniu: R (History / GameOver)
- Iesire: ESC
- Volum: slider cu mouse-ul

---

# Structura proiectului
- main.cpp – UI, input, desenare, audio, volum
- Game.hpp / Game.cpp – logica jocului si AI
- Board.hpp / Board.cpp – tabla de joc 10x10
- Ships.hpp / Ships.cpp – definitii nave
- Stats.hpp / Stats.cpp – istoric rezultate

---

## Diagrama UML

```mermaid
classDiagram
direction TB

class GameState {
  MainMenu
  History
  PlacingShips
  Playing
  GameOver
}

class Difficulty {
  Easy
  Smarter
}

class CellState {
  Empty
  Ship
  Hit
  Miss
}

class ShipDef {
  +name: string
  +length: int
  +horizontal: bool
}

class ShotInfo {
  +r: int
  +c: int
  +before: CellState
  +after: CellState
}

class Board {
  -grid: CellState[10][10]
  +getCell(r:int, c:int) CellState
  +placeShip(r:int, c:int, length:int, horizontal:bool) bool
  +canPlaceShip(r:int, c:int, length:int, horizontal:bool) bool
  +hit(r:int, c:int) CellState
  +allShipsSunk() bool
}

class Game {
  +state: GameState
  +difficulty: Difficulty
  +playerWon: bool
  +player: Board
  +computer: Board
  +lastPlayerShot: optional~ShotInfo~
  +lastComputerShot: optional~ShotInfo~
  +startNewGame(d:Difficulty) void
  +resetToMenu() void
  +hasShipsToPlace() bool
  +currentShip() ShipDef
  +rotateCurrentShip() void
  +placeCurrentShip(r:int, c:int) bool
  +playerShoot(r:int, c:int) bool
}

class Stats {
  +appendResult(playerWon:bool, d:Difficulty) void
  +readHistory() vector~HistoryEntry~
  +computeTotals(out wins:int, out losses:int) void
}

class HistoryEntry {
  +text: string
}

Game "1" *-- "1" Board : player
Game "1" *-- "1" Board : computer
Game "1" o-- "0..1" ShotInfo : lastPlayerShot
Game "1" o-- "0..1" ShotInfo : lastComputerShot
Board ..> ShipDef : foloseste la plasare
Stats ..> HistoryEntry : produce/citeste
