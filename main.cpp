// =====================================================================
//  BLOCK BUSTER  -  a brick breaker game in C++ with SFML 3
//  CS1002 Programming Fundamentals, Fall 2026  -  Semester Project
//
//  Group members (also put these in README.md):
//    1. Name ...................  Roll no. 26L-....
//    2. Name ...................  Roll no. 26L-....
//    3. Name ...................  Roll no. 26L-....
//
//  HOW TO USE THIS TEMPLATE
//  - Every function below is EMPTY. The comments say what goes inside.
//  - "WHO:" says which member writes it (change it to your names).
//  - "MARKS:" says which feature/checklist item it earns.
//  - The game logic must be your team's own work, so you write the code
//    inside the functions. Only the SFML parts (window, drawing, keys,
//    text) are given, as the project PDF allows.
//  - Delete the "TODO" lines once a part is done and working.
//
//  SUGGESTED SPLIT FOR 3 MEMBERS
//    Member 1: Features 1, 2, 3   (game loop, loading level, paddle, launch)
//    Member 2: Features 4, 5      (ball movement, bouncing, breaking bricks)
//    Member 3: Features 6, 7, 8   (lives/score/HUD, menu & states, save)
//    Everyone: Feature 9 (own commits on GitHub), Feature 10 (clean code)
//    Bonus (+5): power-ups, whoever finishes first
//
//  VISUAL STUDIO SETUP (SFML 3.0.2) - check these if it doesn't build
//  - Project Properties -> C/C++ -> Language -> C++ Language Standard:
//    ISO C++17 (or newer). SFML 3 does not work with C++14.
//  - Linker -> Input: sfml-graphics, sfml-window, sfml-system
//    (with "-d" at the end for Debug, e.g. sfml-graphics-d.lib).
//  - The SFML .dll files must be next to your .exe (or in PATH).
//  - Put level.txt and arial.ttf in the SAME folder as main.cpp
//    (the project folder), because that is where Visual Studio runs
//    the game from.
// =====================================================================

#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>     // ifstream / ofstream for level.txt and savegame.txt
#include <cstdlib>     // rand(), srand()
#include <ctime>       // time()
#include <optional>
#include <string>
using namespace std;


// =====================================================================
//  SECTION 1: GAME VALUES (named constants, no magic numbers)
//  MARKS: Feature 10 - "named constants instead of magic numbers"
//  These are the values from the "Game values" table in the PDF.
// =====================================================================
const int WINDOW_W = 600;          // window width
const int WINDOW_H = 700;          // window height
const int HUD_H    = 50;           // score/lives strip at the top

const int ROWS = 8;                // bricks: 8 rows
const int COLS = 10;               // bricks: 10 columns
const float BRICK_W  = 60.f;       // one brick is 60 x 25 pixels
const float BRICK_H  = 25.f;
const float BRICKS_Y = 100.f;      // bricks start at y = 100

const int PADDLE_W       = 100;    // paddle is 100 x 14 pixels
const int PADDLE_H       = 14;
const int PADDLE_Y       = 650;    // top edge of the paddle
const int PADDLE_START_X = 250;    // paddle starts here
const int PADDLE_SPEED   = 8;      // pixels every frame

const int BALL_SIZE = 12;          // ball fits in a 12 x 12 box (radius 6)

const int START_LIVES  = 3;
const int POINTS_HIT   = 10;       // a hit that does not break the brick
const int POINTS_BREAK = 50;       // the hit that breaks the brick

// Game states (Feature 7)
const int MENU      = 0;
const int PLAYING   = 1;
const int GAME_OVER = 2;           // also used for "YOU WIN" (see playerWon)

// Keys pressed once (saved inside the event loop)
const int KEY_NONE   = 0;
const int KEY_LAUNCH = 1;          // Space
const int KEY_ENTER  = 2;
const int KEY_ESCAPE = 3;
const int KEY_UP     = 4;
const int KEY_DOWN   = 5;

// Menu options (Feature 7: New Game, Continue, Exit - in this order)
const int OPTION_NEW      = 0;
const int OPTION_CONTINUE = 1;
const int OPTION_EXIT     = 2;


// =====================================================================
//  SECTION 2: GAME VARIABLES
//  Everything the game needs to remember between frames.
// =====================================================================
int bricks[ROWS][COLS];   // 0 = no brick, 1/2/3 = hits left (Feature 2)

int paddleX = PADDLE_START_X;   // left edge of the paddle
int ballX = 0, ballY = 0;       // top-left corner of the ball's 12x12 box
int dx = 0, dy = 0;             // ball speed (pixels per frame)
bool ballLaunched = false;      // false = ball waits on the paddle

int score = 0;
int lives = START_LIVES;

int gameState  = MENU;
bool playerWon = false;         // true -> "YOU WIN", false -> "GAME OVER"
int menuChoice = OPTION_NEW;    // which menu option is highlighted
bool showNoSavedGame = false;   // shows "No saved game" on the menu
bool levelMissing    = false;   // true if level.txt could not be opened

int pressedKey = KEY_NONE;      // key pressed once this frame

sf::Font font;                  // font for the HUD and menu text

// BONUS (power-ups) - only if you do the bonus
// bool powerUpFalling = false;
// int  powerUpX = 0, powerUpY = 0;
// int  powerUpType = 0;          // 0 = W (wide), 1 = + (life)
// int  wideTimer = 0;            // frames left of the wide paddle (600 = 10 s)


// =====================================================================
//  SECTION 3: FUNCTION LIST (prototypes)
//  MARKS: Feature 10 - small functions that each do one job
// =====================================================================
bool loadLevel();
void resetBallOnPaddle();
void startNewGame();

void handleInput(sf::RenderWindow& window);
void updateGame();
void drawGame(sf::RenderWindow& window);

void movePaddle();
void launchBall();
void moveBall();
bool hitBricks();
void bounceOnPaddle();
void loseLife();
bool allBricksBroken();

void saveGame();
bool continueGame();
void clearSave();

void drawBricks(sf::RenderWindow& window);
void drawPaddleAndBall(sf::RenderWindow& window);
void drawHUD(sf::RenderWindow& window);
void drawMenu(sf::RenderWindow& window);
void drawEndScreen(sf::RenderWindow& window);


// =====================================================================
//  SECTION 4: MAIN  (keep it SHORT - Feature 10)
//  MARKS: Feature 1 (5 marks)
//  WHO: Member 1
// =====================================================================
int main()
{
    // Window 600 x 700 titled "Block Buster", 60 frames per second (2 marks)
    sf::RenderWindow window(sf::VideoMode({600, 700}), "Block Buster");
    window.setFramerateLimit(60);   // keeps the game speed the same on every laptop

    srand((unsigned)time(0));       // random numbers, call once only

    // Font for text. Copy any .ttf file (e.g. arial.ttf from
    // C:\Windows\Fonts) into your project folder.
    if (!font.openFromFile("arial.ttf"))
        cout << "Could not load arial.ttf" << endl;

    // TODO: try loading the level once here, so the menu can show an
    //       error if level.txt is missing (Feature 2, 2 marks).

    // THE GAME LOOP: input -> update -> draw, in this order (2 marks)
    while (window.isOpen())
    {
        handleInput(window);
        updateGame();
        drawGame(window);
    }

    return 0;   // X button closes the window and the loop ends (1 mark)
}


// =====================================================================
//  SECTION 5: LEVEL AND NEW GAME
// =====================================================================

// ---------------------------------------------------------------------
//  loadLevel()  -  read level.txt into bricks[8][10]
//  MARKS: Feature 2 (15 marks)
//  WHO: Member 1
// ---------------------------------------------------------------------
bool loadLevel()
{
    // TODO: open "level.txt" with ifstream.
    // TODO: if it did not open -> set levelMissing = true and return false
    //       (the game must show an error message, NOT crash).
    // TODO: read the first two numbers (rows and columns).
    // TODO: two nested loops (r, c) and read each number with
    //       fin >> bricks[r][c];
    // TODO: close the file, set levelMissing = false, return true.

    return false;   // remove this line when you write the function
}

// ---------------------------------------------------------------------
//  resetBallOnPaddle()  -  put the ball back, waiting for Space
//  MARKS: Feature 3 / Feature 6
//  WHO: Member 1
// ---------------------------------------------------------------------
void resetBallOnPaddle()
{
    // TODO: ballLaunched = false
    // TODO: ball sits in the middle of the paddle:
    //       ballX = paddleX + 44,  ballY = 638
    // TODO: dx and dy = 0
}

// ---------------------------------------------------------------------
//  startNewGame()  -  New Game always starts fresh
//  MARKS: Feature 7 ("New Game starts fresh")
//  WHO: Member 3
// ---------------------------------------------------------------------
void startNewGame()
{
    // TODO: score = 0, lives = START_LIVES
    // TODO: load the bricks again from level.txt (call loadLevel)
    // TODO: paddle back to PADDLE_START_X
    // TODO: ball waiting on the paddle (call resetBallOnPaddle)
    // TODO: gameState = PLAYING
}


// =====================================================================
//  SECTION 6: INPUT  (step 1 of the game loop)
//  WHO: Member 1 (keys) + Member 3 (menu part)
// =====================================================================
void handleInput(sf::RenderWindow& window)
{
    pressedKey = KEY_NONE;

    // ---- Keys pressed ONCE (from the PDF, SFML part) ----
    while (const std::optional event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
            window.close();
        else if (const auto* key = event->getIf<sf::Event::KeyPressed>())
        {
            if (key->code == sf::Keyboard::Key::Space)  pressedKey = KEY_LAUNCH;
            if (key->code == sf::Keyboard::Key::Enter)  pressedKey = KEY_ENTER;
            if (key->code == sf::Keyboard::Key::Escape) pressedKey = KEY_ESCAPE;
            if (key->code == sf::Keyboard::Key::Up)     pressedKey = KEY_UP;
            if (key->code == sf::Keyboard::Key::Down)   pressedKey = KEY_DOWN;
        }
    }

    // ---- What each key does depends on the state (switch-case) ----
    // MARKS: Feature 7 - "gameState handled with switch-case"
    switch (gameState)
    {
    case MENU:
        // TODO: Up/Down change menuChoice (0, 1, 2)
        // TODO: Enter on New Game  -> startNewGame()
        // TODO: Enter on Continue  -> continueGame(); if it returns false,
        //                            showNoSavedGame = true (stay on menu)
        // TODO: Enter on Exit      -> window.close()
        break;

    case PLAYING:
        // TODO: Space -> launchBall()                         (Feature 3)
        // TODO: Escape -> saveGame(), then gameState = MENU    (Feature 8)
        break;

    case GAME_OVER:
        // TODO: Enter -> gameState = MENU                      (Feature 7)
        break;
    }
}


// =====================================================================
//  SECTION 7: UPDATE  (step 2 of the game loop)
//  WHO: Member 2
// =====================================================================
void updateGame()
{
    if (gameState != PLAYING)
        return;   // only move things while playing
    movePaddle();
    if(!ballLaunched)
    {
        ballX = paddle + 44;
        ballY = 638;
    }
    else
    {
        moveBall();
    }
    if(allBricksBroken())
    {
        playerWon = true;
        clearSave();
        gameState = GAME_OVER;
    }
}

// ---------------------------------------------------------------------
//  movePaddle()  -  Left/Right or A/D while HELD down
//  MARKS: Feature 3 (5 marks: moves 3 + never leaves window 2)
//  WHO: Member 1
// ---------------------------------------------------------------------
void movePaddle()
{
    // Held-down keys are checked directly every frame (SFML part):
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
    {
        // TODO: move the paddle left by PADDLE_SPEED
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
    {
        // TODO: move the paddle right by PADDLE_SPEED
    }

    // TODO: stop the paddle at the edges:
    //       paddleX can't go below 0, and paddleX + PADDLE_W can't go past WINDOW_W
}

// ---------------------------------------------------------------------
//  launchBall()  -  Space launches the ball upwards
//  MARKS: Feature 3 (2 marks)
//  WHO: Member 1
// ---------------------------------------------------------------------
void launchBall()
{
    // TODO: only if the ball is not launched yet
    // TODO: dy = -5 (upwards)
    // TODO: dx = 3 or -3, chosen with rand() % 2
    // TODO: ballLaunched = true
}

// ---------------------------------------------------------------------
//  moveBall()  -  move in TWO steps: x first, then y
//  MARKS: Feature 4 (10 marks) + uses Feature 5
//  WHO: Member 2
// ---------------------------------------------------------------------
void moveBall()
{
    // ---- Step 1: x ----
    // TODO: ballX = ballX + dx
    // TODO: hit the left wall (ballX < 0) or right wall (ballX + 12 > 600)?
    //       -> undo the step and dx = -dx
    // TODO: did the x step touch a brick? (hitBricks())
    //       -> undo the step and dx = -dx   (ball goes back sideways)

    // ---- Step 2: y ----
    // TODO: ballY = ballY + dy
    // TODO: went into the HUD (ballY < 50)? -> undo the step and dy = -dy
    // TODO: did the y step touch a brick? -> undo the step and dy = -dy

    // ---- Paddle and bottom ----
    // TODO: bounceOnPaddle()
    // TODO: ball below the window (ballY > 700)? -> loseLife()   (Feature 6)
}

// ---------------------------------------------------------------------
//  hitBricks()  -  find the bricks under the 4 corners of the ball
//  Returns true if the ball touched at least one brick.
//  MARKS: Feature 5 (25 marks) - the biggest feature!
//  WHO: Member 2
// ---------------------------------------------------------------------
bool hitBricks()
{
    // The 4 corners of the ball's box are:
    //   (ballX, ballY), (ballX + 11, ballY),
    //   (ballX, ballY + 11), (ballX + 11, ballY + 11)
    //
    // TODO: for each corner (cx, cy):
    //   - skip it unless it is inside the brick area:
    //       cy >= 100, cy < 300, cx >= 0, cx < 600
    //   - row = (cy - 100) / 25,   col = cx / 60   (integer division)
    //   - if bricks[row][col] > 0, that brick was touched
    //
    // TODO: every touched brick loses ONE hit (subtract 1).
    //       Careful: if two corners are on the SAME brick, it loses only one hit.
    // TODO: score: +10 for a hit, +50 if that hit made it 0 (broke it)
    // BONUS: when a brick breaks, rand() % 5 == 0 -> drop a power-up
    // TODO: return true if any brick was touched, otherwise false

    return false;   // remove this line when you write the function
}

// ---------------------------------------------------------------------
//  bounceOnPaddle()  -  the paddle decides the new direction
//  MARKS: Feature 4 (5 marks: only moving down 2 + dx from part 3)
//  WHO: Member 2
// ---------------------------------------------------------------------
void bounceOnPaddle()
{
    // TODO: only when the ball is moving DOWN (dy > 0)
    // TODO: check if the ball's box and the paddle's box touch
    //       (overlap both left-right AND up-down)
    // TODO: if they touch:
    //   - ballY = 638 (put it just on top of the paddle), dy = -5
    //   - part = (ballX + 6 - paddleX) * 4 / PADDLE_W
    //   - if part < 0 use 0, if part > 3 use 3
    //   - part 0 -> dx = -6, part 1 -> dx = -3, part 2 -> dx = 3, part 3 -> dx = 6
}

// ---------------------------------------------------------------------
//  loseLife()  -  the ball fell past the paddle
//  MARKS: Feature 6 (3 marks)
//  WHO: Member 3
// ---------------------------------------------------------------------
void loseLife()
{
    lives--;
    if(lives==0)
    {
        playerWon = false;
        clearSave();
        gameState = GAME_OVER;
    }
}

// ---------------------------------------------------------------------
//  allBricksBroken()  -  true when every brick is 0
//  MARKS: Feature 6 ("YOU WIN")
//  WHO: Member 3
// ---------------------------------------------------------------------
bool allBricksBroken()
{
    for(int r = 0; r < ROWS; r++)
        {
            for(int c = 0; c < COLS; c++)
                {
                    if(bricks[r][c] > 0)
                    {
                        return false;
                    }
                }
        }
    return true;
}



// =====================================================================
//  SECTION 8: SAVE AND CONTINUE
//  MARKS: Feature 8 (7 marks)
//  WHO: Member 3
//
//  savegame.txt format:
//    line 1:      score lives            e.g.  1250 2
//    next 8 lines: the bricks as they are now, 10 numbers per row
//  Do NOT save the paddle or the ball.
// =====================================================================
void saveGame()
{
    // TODO: open "savegame.txt" with ofstream
    // TODO: write score and lives on line 1
    // TODO: nested loops: write each row of bricks (10 numbers, spaces between)
    // TODO: close the file
}

bool continueGame()
{
    // TODO: open "savegame.txt" with ifstream
    // TODO: if it did not open OR it is empty -> return false
    //       (the menu shows "No saved game")
    // TODO: read score, lives, then the 8 x 10 bricks
    // TODO: paddle in the middle, ball waiting on it (resetBallOnPaddle)
    // TODO: gameState = PLAYING, return true

    return false;   // remove this line when you write the function
}

void clearSave()
{
    // Opening the file with ofstream and closing it straight away
    // makes it empty, so a finished game can't be continued.
    // TODO: write it
}


// =====================================================================
//  SECTION 9: DRAWING  (step 3 of the game loop)
//  SFML drawing code is allowed from the PDF / AI tools.
// =====================================================================
void drawGame(sf::RenderWindow& window)
{
    window.clear(sf::Color(25, 25, 45));   // background colour

    switch (gameState)
    {
    case MENU:
        drawMenu(window);
        break;
    case PLAYING:
        drawHUD(window);
        drawBricks(window);
        drawPaddleAndBall(window);
        break;
    case GAME_OVER:
        drawEndScreen(window);
        break;
    }

    window.display();
}

// ---------------------------------------------------------------------
//  drawBricks()  -  MARKS: Feature 2 (gap 5 marks + colours 3 marks)
//  WHO: Member 1
// ---------------------------------------------------------------------
void drawBricks(sf::RenderWindow& window)
{
    sf::Color colors[4] = { sf::Color(25, 25, 45),  // 0: no brick
                            sf::Color::Green,       // 1: one hit left
                            sf::Color::Yellow,      // 2: two hits left
                            sf::Color::Red };       // 3: three hits left

    sf::RectangleShape brick({BRICK_W - 2, BRICK_H - 2});   // smaller, so a gap shows

    // TODO: nested loops over r and c
    // TODO: skip the brick if bricks[r][c] == 0
    // TODO: otherwise use the lines below for brick [r][c]:
    //   brick.setFillColor(colors[bricks[r][c]]);
    //   brick.setPosition({c * BRICK_W + 1, BRICKS_Y + r * BRICK_H + 1});
    //   window.draw(brick);
}

// ---------------------------------------------------------------------
//  drawPaddleAndBall()  -  WHO: Member 1
// ---------------------------------------------------------------------
void drawPaddleAndBall(sf::RenderWindow& window)
{
    sf::RectangleShape paddle({100.f, 14.f});
    sf::CircleShape ball(6.f);              // radius 6, fits in a 12 x 12 box

    paddle.setFillColor(sf::Color::Cyan);
    ball.setFillColor(sf::Color::White);

    paddle.setPosition({(float)paddleX, 650.f});
    ball.setPosition({(float)ballX, (float)ballY});
    window.draw(paddle);
    window.draw(ball);

    // BONUS: if wide paddle is on, the paddle size is 150 instead of 100
    // BONUS: draw the falling power-up ("W" or "+") here
}

// ---------------------------------------------------------------------
//  drawHUD()  -  score and lives in the top 50 pixels
//  MARKS: Feature 6 (4 marks)
//  WHO: Member 3
// ---------------------------------------------------------------------
void drawHUD(sf::RenderWindow& window)
{
    // Text in SFML 3: sf::Text text(font, "words", size);
    sf::Text scoreText(font, "SCORE " + to_string(score), 20);
    scoreText.setPosition({10.f, 12.f});
    window.draw(scoreText);

    // TODO: draw the lives on the right side, either as a number
    //       ("LIVES 3") or as small circles (one CircleShape per life)
    // TODO (optional): a line under the HUD at y = 50
}

// ---------------------------------------------------------------------
//  drawMenu()  -  New Game, Continue, Exit
//  MARKS: Feature 7 (4 marks) + Feature 8 ("No saved game")
//  WHO: Member 3
// ---------------------------------------------------------------------
void drawMenu(sf::RenderWindow& window)
{
    // TODO: draw the title "BLOCK BUSTER"
    // TODO: draw the 3 options in this order: New Game, Continue, Exit
    // TODO: show the selected option differently (other colour or ">")
    //       using menuChoice
    // TODO: if showNoSavedGame -> draw "No saved game"
    // TODO: if levelMissing    -> draw "Error: level.txt not found"
}

// ---------------------------------------------------------------------
//  drawEndScreen()  -  "GAME OVER" or "YOU WIN"
//  MARKS: Feature 6 + Feature 7 (2 marks)
//  WHO: Member 3
// ---------------------------------------------------------------------
void drawEndScreen(sf::RenderWindow& window)
{
    // TODO: if playerWon draw "YOU WIN", otherwise draw "GAME OVER"
    // TODO: draw the final score
    // TODO: draw "Press Enter" (Enter goes back to the menu, see handleInput)
}
