#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <iostream>
#include <ctime>
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>

#include "Game.hpp"
#include "Stats.hpp"




// - UI (SFML 3): meniuri, desenare table, input mouse/tastatura
// - Audio: muzica de fundal + efecte + slider de volum

// RectF - dreptunghi simplu pentru butoane

struct RectF {
    float x, y, w, h;

    // Verifica daca punctul p este in interiorul dreptunghiului
    bool contains(sf::Vector2f p) const {
        return p.x >= x && p.x <= x + w &&
            p.y >= y && p.y <= y + h;
    }
};


// VolumeSlider - slider pentru volum 
// - afecteaza muzica + toate sunetele

struct VolumeSlider {
    RectF bar;             // zona slider-ului
    float value = 80.f;    // volum 0-100
    bool dragging = false; // true daca utilizatorul tine apasat pe slider

    // Seteaza value in functie de mouseX
    void updateFromMouse(float mouseX) {
        float t = (mouseX - bar.x) / bar.w; // 0..1
        t = std::clamp(t, 0.f, 1.f);
        value = t * 100.f;
    }

    // Deseneaza slider-ul
    void draw(sf::RenderWindow& w) const {
        // fundal
        sf::RectangleShape bg;
        bg.setPosition(sf::Vector2f(bar.x, bar.y));
        bg.setSize(sf::Vector2f(bar.w, bar.h));
        bg.setFillColor(sf::Color(55, 55, 75));
        w.draw(bg);

        // fill (procent din bara)
        sf::RectangleShape fill;
        fill.setPosition(sf::Vector2f(bar.x, bar.y));
        fill.setSize(sf::Vector2f(bar.w * (value / 100.f), bar.h));
        fill.setFillColor(sf::Color(120, 200, 255));
        w.draw(fill);

        // knob (cerc mic la pozitia curenta)
        float knobX = bar.x + bar.w * (value / 100.f);
        sf::CircleShape knob(7.f);
        knob.setFillColor(sf::Color::White);
        knob.setPosition(sf::Vector2f(knobX - 7.f, bar.y + bar.h * 0.5f - 7.f));
        w.draw(knob);

        // border
        sf::RectangleShape border;
        border.setPosition(sf::Vector2f(bar.x, bar.y));
        border.setSize(sf::Vector2f(bar.w, bar.h));
        border.setFillColor(sf::Color::Transparent);
        border.setOutlineThickness(2.f);
        border.setOutlineColor(sf::Color::White);
        w.draw(border);
    }
};

// GridUI - descrie o tabla desenata pe ecran
// - origin: coltul stanga-sus al tablei in coordonate ecran
// - cellSize: marimea unei celule (pixeli)

struct GridUI {
    sf::Vector2f origin;
    float cellSize;

    // Converteste pozitia mouse-ului in (r,c) pe tabla
    bool mouseToCell(const sf::Vector2i& mouse, int& r, int& c) const {
        float x = float(mouse.x) - origin.x;
        float y = float(mouse.y) - origin.y;
        if (x < 0.f || y < 0.f) return false;

        c = int(x / cellSize);
        r = int(y / cellSize);

        return r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE;
    }

    // Centrul unei celule (util pt marker / animatii)
    sf::Vector2f cellCenter(int r, int c) const {
        return sf::Vector2f(
            origin.x + c * cellSize + cellSize * 0.5f,
            origin.y + r * cellSize + cellSize * 0.5f
        );
    }
};


// HitAnim - animatie  la hit

struct HitAnim {
    bool active = false;
    sf::Vector2f center;
    float t = 0.f;
    float duration = 0.35f;

    void start(sf::Vector2f c) { active = true; center = c; t = 0.f; }
    void update(float dt) {
        if (!active) return;
        t += dt;
        if (t >= duration) active = false;
    }
};

static void drawHitAnim(sf::RenderWindow& w, const HitAnim& a) {
    if (!a.active) return;

    float p = a.t / a.duration;     // progres 0..1
    float radius = 8.f + 40.f * p;  // creste in timp

    int alphaInt = int(180 * (1.f - p));
    if (alphaInt < 0) alphaInt = 0;
    if (alphaInt > 255) alphaInt = 255;

    std::uint8_t alpha = (std::uint8_t)alphaInt;

    sf::CircleShape ring(radius);
    ring.setFillColor(sf::Color::Transparent);
    ring.setOutlineThickness(3.f);
    ring.setOutlineColor(sf::Color(255, 255, 255, alpha));
    ring.setPosition(sf::Vector2f(a.center.x - radius, a.center.y - radius));
    w.draw(ring);
}


// Desenare UI

static sf::Color cellColor(CellState st, bool revealShips) {
    if (st == CellState::Empty) return sf::Color(30, 60, 130);
    if (st == CellState::Ship)  return revealShips ? sf::Color(90, 90, 90) : sf::Color(30, 60, 130);
    if (st == CellState::Hit)   return sf::Color(200, 60, 60);
    return sf::Color(120, 160, 220); // Miss
}

static void drawBoard(sf::RenderWindow& w, const Board& b, const GridUI& ui, bool revealShips) {
    sf::RectangleShape cell;
    cell.setSize(sf::Vector2f(ui.cellSize - 2.f, ui.cellSize - 2.f));

    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            cell.setPosition(sf::Vector2f(
                ui.origin.x + c * ui.cellSize + 1.f,
                ui.origin.y + r * ui.cellSize + 1.f
            ));
            cell.setFillColor(cellColor(b.getCell(r, c), revealShips));
            w.draw(cell);
        }
    }

    sf::RectangleShape border;
    border.setPosition(ui.origin);
    border.setSize(sf::Vector2f(ui.cellSize * BOARD_SIZE, ui.cellSize * BOARD_SIZE));
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineThickness(3.f);
    border.setOutlineColor(sf::Color::White);
    w.draw(border);
}

static void drawButton(sf::RenderWindow& w, const sf::Font& font, const RectF& r, const std::string& label, bool hovered) {
    sf::RectangleShape box;
    box.setPosition(sf::Vector2f(r.x, r.y));
    box.setSize(sf::Vector2f(r.w, r.h));
    box.setFillColor(hovered ? sf::Color(70, 70, 95) : sf::Color(50, 50, 70));
    box.setOutlineThickness(2.f);
    box.setOutlineColor(sf::Color::White);
    w.draw(box);

    sf::Text t(font, label, 20);
    t.setFillColor(sf::Color::White);
    t.setPosition(sf::Vector2f(r.x + 18.f, r.y + 16.f));
    w.draw(t);
}

// Marker pentru ultima lovitura
static void drawShotMarker(sf::RenderWindow& w, const GridUI& ui, const std::optional<ShotInfo>& shot) {
    if (!shot.has_value()) return;

    sf::CircleShape dot(4.f);
    dot.setFillColor(sf::Color::White);

    sf::Vector2f center = ui.cellCenter(shot->r, shot->c);
    dot.setPosition(sf::Vector2f(center.x - 4.f, center.y - 4.f));
    w.draw(dot);
}

// Preview pentru nava in PlacingShips
static void drawShipPreview(sf::RenderWindow& w, const GridUI& ui,
    int r, int c, int length, bool horizontal, bool valid) {
    sf::RectangleShape cell;
    cell.setSize(sf::Vector2f(ui.cellSize - 2.f, ui.cellSize - 2.f));
    cell.setFillColor(valid ? sf::Color(0, 255, 0, 120) : sf::Color(255, 0, 0, 120));

    for (int i = 0; i < length; ++i) {
        int rr = r + (horizontal ? 0 : i);
        int cc = c + (horizontal ? i : 0);
        if (rr < 0 || rr >= BOARD_SIZE || cc < 0 || cc >= BOARD_SIZE) continue;

        cell.setPosition(sf::Vector2f(
            ui.origin.x + cc * ui.cellSize + 1.f,
            ui.origin.y + rr * ui.cellSize + 1.f
        ));
        w.draw(cell);
    }
}

// Incarcare sunet
static bool tryLoadSound(sf::SoundBuffer& buf, const char* path) {
    if (!buf.loadFromFile(path)) {
        std::cerr << "[WARNING] Nu pot incarca: " << path << "\n";
        return false;
    }
    return true;
}

static std::string diffToString(Difficulty d) {
    return (d == Difficulty::Easy) ? "Usor" : "Ceva mai complicat";
}

int main() {
    std::srand((unsigned)std::time(nullptr));

    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(1000u, 600u)), "Battleships (SFML 3)");
    window.setFramerateLimit(60);

    Game game;

    GridUI playerUI{ sf::Vector2f(60.f, 120.f), 34.f };
    GridUI compUI{ sf::Vector2f(560.f, 120.f), 34.f };

    sf::Font font("assets/arial.ttf");

    sf::Text title(font, "BATTLESHIPS", 32);
    title.setFillColor(sf::Color::White);
    title.setPosition(sf::Vector2f(60.f, 20.f));

    sf::Text info(font, "", 18);
    info.setFillColor(sf::Color::White);
    info.setPosition(sf::Vector2f(60.f, 70.f));

    // Butoane meniu
    RectF btnEasy{ 60.f, 160.f, 360.f, 60.f };
    RectF btnHard{ 60.f, 240.f, 360.f, 60.f };
    RectF btnHistory{ 60.f, 320.f, 360.f, 60.f };


    VolumeSlider volume;
    volume.bar = RectF{ 60.f, 560.f - 20.f, 260.f, 14.f }; // jos stanga
    volume.value = 80.f;

    sf::Text volLabel(font, "Volum", 14);
    volLabel.setFillColor(sf::Color(200, 200, 200));
    volLabel.setPosition(sf::Vector2f(60.f, 560.f - 42.f));

    sf::Text volValueText(font, "", 14);
    volValueText.setFillColor(sf::Color(200, 200, 200));


    // muzica de fundal
    // - porneste din meniu
    // - se opreste la GameOver, ca sa se auda doar win/lose
 
    sf::Music bgm;
    bool hasBgm = bgm.openFromFile("assets/music.ogg");
    if (!hasBgm) {
        std::cerr << "[WARNING] Nu pot incarca muzica: assets/music.ogg\n";
    }
    else {
        bgm.setLooping(true);       
        bgm.setVolume(volume.value);
        bgm.play();                 // porneste din MainMenu
    }


    sf::SoundBuffer bufHit, bufMiss, bufPlace, bufWin, bufLose;
    std::optional<sf::Sound> sndHit, sndMiss, sndPlace, sndWin, sndLose;

    if (tryLoadSound(bufHit, "assets/hit.wav"))       sndHit.emplace(bufHit);
    if (tryLoadSound(bufMiss, "assets/miss.wav"))     sndMiss.emplace(bufMiss);
    if (tryLoadSound(bufPlace, "assets/place.wav"))   sndPlace.emplace(bufPlace);
    if (tryLoadSound(bufWin, "assets/victory.wav"))   sndWin.emplace(bufWin);
    if (tryLoadSound(bufLose, "assets/defeat.wav"))   sndLose.emplace(bufLose);

    // Animatii hit
    HitAnim animOnComputer, animOnPlayer;

    int historyScroll = 0;

    // Sa nu se redea sunetul GameOver de mai multe ori
    bool recordedGameOver = false;
    bool playedGameOverSound = false;

    sf::Clock clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        animOnComputer.update(dt);
        animOnPlayer.update(dt);

        while (auto ev = window.pollEvent()) {
            if (ev->is<sf::Event::Closed>()) {
                window.close();
            }

            // Mouse wheel doar in History
            if (ev->is<sf::Event::MouseWheelScrolled>() && game.state == GameState::History) {
                const auto* mw = ev->getIf<sf::Event::MouseWheelScrolled>();
                if (mw) {
                    if (mw->delta > 0) historyScroll += 1;
                    else historyScroll -= 1;
                    if (historyScroll < 0) historyScroll = 0;
                }
            }

            // Start drag pe slider la click stanga
            if (ev->is<sf::Event::MouseButtonPressed>()) {
                const auto* mb = ev->getIf<sf::Event::MouseButtonPressed>();
                if (mb && mb->button == sf::Mouse::Button::Left) {
                    sf::Vector2f mp(float(mb->position.x), float(mb->position.y));

                    if (volume.bar.contains(mp)) {
                        volume.dragging = true;
                        volume.updateFromMouse(mp.x);
                    }
                }
            }

            // Stop drag la release
            if (ev->is<sf::Event::MouseButtonReleased>()) {
                const auto* mb = ev->getIf<sf::Event::MouseButtonReleased>();
                if (mb && mb->button == sf::Mouse::Button::Left) {
                    volume.dragging = false;
                }
            }

            // Update drag la mouse move
            if (ev->is<sf::Event::MouseMoved>() && volume.dragging) {
                const auto* mm = ev->getIf<sf::Event::MouseMoved>();
                if (mm) volume.updateFromMouse(float(mm->position.x));
            }

            // Taste
            if (ev->is<sf::Event::KeyPressed>()) {
                const auto* k = ev->getIf<sf::Event::KeyPressed>();
                if (!k) continue;

                if (k->code == sf::Keyboard::Key::Escape) {
                    window.close();
                }

                // R are rol diferit in functie de stare
                if (k->code == sf::Keyboard::Key::R) {
                    if (game.state == GameState::PlacingShips) {
                        game.rotateCurrentShip();
                    }
                    else if (game.state == GameState::History) {
                        game.state = GameState::MainMenu;
                        historyScroll = 0;
                    }
                    else if (game.state == GameState::GameOver) {
                        game.resetToMenu();
                        recordedGameOver = false;
                        playedGameOverSound = false;
                        historyScroll = 0;

                        // La intoarcerea in meniu repornim muzica (daca exista)
                        if (hasBgm) {
                            bgm.setVolume(volume.value);
                            if (bgm.getStatus() != sf::SoundSource::Status::Playing) { // FIX SFML 3
                                bgm.play();
                            }
                        }
                    }
                }
            }

            // Click mouse (pentru butoane / tabla)
            if (ev->is<sf::Event::MouseButtonPressed>()) {
                const auto* mb = ev->getIf<sf::Event::MouseButtonPressed>();
                if (!mb) continue;

                if (mb->button != sf::Mouse::Button::Left) continue;

                sf::Vector2f mp(float(mb->position.x), float(mb->position.y));

                // Meniu principal
                if (game.state == GameState::MainMenu) {
                    if (volume.bar.contains(mp)) {
                    }
                    else if (btnEasy.contains(mp)) {
                        game.startNewGame(Difficulty::Easy);
                        recordedGameOver = false;
                        playedGameOverSound = false;

                        // Muzica trebuie sa cante in timpul jocului
                        if (hasBgm && bgm.getStatus() != sf::SoundSource::Status::Playing) { 
                            bgm.play();
                        }
                    }
                    else if (btnHard.contains(mp)) {
                        game.startNewGame(Difficulty::Smarter);
                        recordedGameOver = false;
                        playedGameOverSound = false;

                        if (hasBgm && bgm.getStatus() != sf::SoundSource::Status::Playing) { 
                            bgm.play();
                        }
                    }
                    else if (btnHistory.contains(mp)) {
                        game.state = GameState::History;
                        historyScroll = 0;
                    }
                }
                // plasarea navelor pe tabela
                else if (game.state == GameState::PlacingShips) {
                    int r, c;
                    if (playerUI.mouseToCell(mb->position, r, c)) {
                        if (game.placeCurrentShip(r, c)) {
                            if (sndPlace) sndPlace->play();
                        }
                    }
                }
                // Starea de joc
                else if (game.state == GameState::Playing) {
                    int r, c;
                    if (compUI.mouseToCell(mb->position, r, c)) {
                        bool ok = game.playerShoot(r, c);

                        if (ok && game.lastPlayerShot.has_value()) {
                            if (game.lastPlayerShot->after == CellState::Hit) {
                                if (sndHit) sndHit->play();
                                animOnComputer.start(compUI.cellCenter(game.lastPlayerShot->r, game.lastPlayerShot->c));
                            }
                            else if (game.lastPlayerShot->after == CellState::Miss) {
                                if (sndMiss) sndMiss->play();
                            }
                        }

                        if (ok && game.lastComputerShot.has_value() && game.lastComputerShot->after == CellState::Hit) {
                            animOnPlayer.start(playerUI.cellCenter(game.lastComputerShot->r, game.lastComputerShot->c));
                        }
                    }
                }
            }
        }
            // Volum global

        if (hasBgm) bgm.setVolume(volume.value);
        if (sndHit)   sndHit->setVolume(volume.value);
        if (sndMiss)  sndMiss->setVolume(volume.value);
        if (sndPlace) sndPlace->setVolume(volume.value);
        if (sndWin)   sndWin->setVolume(volume.value);
        if (sndLose)  sndLose->setVolume(volume.value);


        // Sfaristul jocului se salveaza in istoric + sunet win/lose 
        // + oprire muzica de fundal
        if (game.state == GameState::GameOver && !recordedGameOver) {
            Stats::appendResult(game.playerWon, game.difficulty);
            recordedGameOver = true;
        }


        if (game.state == GameState::GameOver && hasBgm &&
            bgm.getStatus() == sf::SoundSource::Status::Playing) { 
            bgm.stop();
        }

        if (game.state == GameState::GameOver && !playedGameOverSound) {
         
            if (game.playerWon) { if (sndWin) sndWin->play(); }
            else { if (sndLose) sndLose->play(); }
            playedGameOverSound = true;
        }


        if (game.state != GameState::GameOver && hasBgm) {
            if (bgm.getStatus() != sf::SoundSource::Status::Playing) { 
                bgm.play();
            }
        }


        window.clear(sf::Color(18, 18, 24));
        window.draw(title);
        window.draw(volLabel);
        volume.draw(window);

        // Volumul muzicii 
        volValueText.setString(std::to_string(int(volume.value)) + "%");
        volValueText.setPosition(sf::Vector2f(volume.bar.x + volume.bar.w + 12.f, volume.bar.y - 4.f));
        window.draw(volValueText);

        // -------- MainMenu --------
        // Usor / Ceva mai complicat / Istoric
        if (game.state == GameState::MainMenu) {
            info.setString("Selecteaza dificultatea sau istoricul:");
            window.draw(info);

            sf::Vector2i mousePix = sf::Mouse::getPosition(window);
            sf::Vector2f mouseF(float(mousePix.x), float(mousePix.y));

            drawButton(window, font, btnEasy, "Usor", btnEasy.contains(mouseF));
            drawButton(window, font, btnHard, "Ceva mai complicat", btnHard.contains(mouseF));
            drawButton(window, font, btnHistory, "Istoric", btnHistory.contains(mouseF));

            sf::Text hint(font, "ESC = Iesire", 16);
            hint.setFillColor(sf::Color(200, 200, 200));
            hint.setPosition(sf::Vector2f(60.f, 400.f));
            window.draw(hint);

            window.display();
            continue;
        }

        // -------- Istoric --------
        if (game.state == GameState::History) {
            int wins = 0, losses = 0;
            Stats::computeTotals(wins, losses);
            auto entries = Stats::readHistory();

            info.setString("Istoric | Victorii: " + std::to_string(wins) +
                " | Infrangeri: " + std::to_string(losses));
            window.draw(info);

            const int linesOnScreen = 14;
            int total = (int)entries.size();

            int maxScroll = std::max(0, total - linesOnScreen);
            if (historyScroll > maxScroll) historyScroll = maxScroll;

            int endIndex = total - historyScroll;
            int startIndex = std::max(0, endIndex - linesOnScreen);

            float y = 120.f;
            for (int i = startIndex; i < endIndex; ++i) {
                sf::Text line(font, entries[i].text, 16);
                line.setFillColor(sf::Color::White);
                line.setPosition(sf::Vector2f(60.f, y));
                y += 24.f;
                window.draw(line);
            }

            sf::Text hint(font, "R = Inapoi la meniu | Mouse wheel = scroll | ESC = iesire", 16);
            hint.setFillColor(sf::Color(200, 200, 200));
            hint.setPosition(sf::Vector2f(60.f, 560.f - 60.f));
            window.draw(hint);

            window.display();
            continue;
        }

        // -------- (Placing / Playing / GameOver) --------
        drawBoard(window, game.player, playerUI, true);
        drawBoard(window, game.computer, compUI, false);

        drawShotMarker(window, compUI, game.lastPlayerShot);
        drawShotMarker(window, playerUI, game.lastComputerShot);

        drawHitAnim(window, animOnComputer);
        drawHitAnim(window, animOnPlayer);

        // Preview nava curenta in PlacingShips
        if (game.state == GameState::PlacingShips && game.hasShipsToPlace()) {
            sf::Vector2i m = sf::Mouse::getPosition(window);
            int rr, cc;
            if (playerUI.mouseToCell(m, rr, cc)) {
                const ShipDef& sh = game.currentShip();
                bool ok = game.player.canPlaceShip(rr, cc, sh.length, sh.horizontal);
                drawShipPreview(window, playerUI, rr, cc, sh.length, sh.horizontal, ok);
            }
        }

        // Text instructiuni sus
        if (game.state == GameState::PlacingShips) {
            info.setString("Dificultate: " + diffToString(game.difficulty) +
                " | Plasare nave: click pe tabla stanga | R = rotire | ESC = iesire");
        }
        else if (game.state == GameState::Playing) {
            info.setString("Dificultate: " + diffToString(game.difficulty) +
                " | Ataca tabla computerului (dreapta) cu click | ESC = iesire");
        }
        else if (game.state == GameState::GameOver) {
            info.setString(std::string(game.playerWon ? "Victorie (Ez)" : "Skill Issue") +
                " | R = meniu | ESC = iesire");
        }
        window.draw(info);

        window.display();
    }

    return 0;
}
