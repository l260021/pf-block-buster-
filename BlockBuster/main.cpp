// =====================================================================
//  BLOCK BUSTER  -  a brick breaker game in C++ with SFML 3
//  CS1002 Programming Fundamentals, Fall 2026  -  Semester Project

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
// =====================================================================
const int WINDOW_W = 600;          // window width
const int WINDOW_H = 700;          // window height
const int HUD_H = 50;           // score/lives strip at the top

const int ROWS = 8;                // bricks: 8 rows
const int COLS = 10;               // bricks: 10 columns
const float BRICK_W = 60.f;       // one brick is 60 x 25 pixels
const float BRICK_H = 25.f;
const float BRICKS_Y = 100.f;      // bricks start at y = 100

const int PADDLE_W = 100;    // paddle is 100 x 14 pixels
const int PADDLE_H = 14;
const int PADDLE_Y = 650;    // top edge of the paddle
const int PADDLE_START_X = 250;    // paddle starts here
const int PADDLE_SPEED = 8;      // pixels every frame

const int BALL_SIZE = 12;          // ball fits in a 12 x 12 box (radius 6)

const int START_LIVES = 3;
const int POINTS_HIT = 10;       // a hit that does not break the brick
const int POINTS_BREAK = 50;       // the hit that breaks the brick

// Game states (Feature 7)
const int MENU = 0;
const int PLAYING = 1;
const int GAME_OVER = 2;           // also used for "YOU WIN" (see playerWon)

// Keys pressed once (saved inside the event loop)
const int KEY_NONE = 0;
const int KEY_LAUNCH = 1;          // Space
const int KEY_ENTER = 2;
const int KEY_ESCAPE = 3;
const int KEY_UP = 4;
const int KEY_DOWN = 5;

// Menu options (Feature 7: New Game, Continue, Exit - in this order)
const int OPTION_NEW = 0;
const int OPTION_CONTINUE = 1;
const int OPTION_EXIT = 2;


// =====================================================================
//  SECTION 2: GAME VARIABLEs
// =====================================================================
int bricks[ROWS][COLS];   // 0 = no brick, 1/2/3 = hits left (Feature 2)

int paddleX = PADDLE_START_X;   // left edge of the paddle
int ballX = 0, ballY = 0;       // top-left corner of the ball's 12x12 box
int dx = 0, dy = 0;             // ball speed (pixels per frame)
bool ballLaunched = false;      // false = ball waits on the paddle

int score = 0;
int lives = START_LIVES;

int gameState = MENU;
bool playerWon = false;         // true -> "YOU WIN", false -> "GAME OVER"
int menuChoice = OPTION_NEW;    // which menu option is highlighted
bool showNoSavedGame = false;   // shows "No saved game" on the menu
bool levelMissing = false;   // true if level.txt could not be opened

int pressedKey = KEY_NONE;      // key pressed once this frame

sf::Font font;                  // font for the HUD and menu text

// BONUS (power-ups) - only if you do the bonus
 bool powerUpFalling = false;
 int  powerUpX = 0, powerUpY = 0;
 int  powerUpType = 0;          // 0 = W (wide), 1 = + (life)
 int  wideTimer = 0;           // frames left of the wide paddle (600 = 10 s)
 int  paddleWidth = PADDLE_W;

// =====================================================================
//  SECTION 3: FUNCTION LIST (prototypes)
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
	sf::RenderWindow window(sf::VideoMode({ 600, 700 }), "Block Buster");
	window.setFramerateLimit(60);   // keeps the game speed the same on every laptop

	srand((unsigned)time(0));       // random numbers, call once only

	// Font for text. Copy any .ttf file (e.g. arial.ttf from
	// C:\Windows\Fonts) into your project folder.
	if (!font.openFromFile("arial.ttf"))
		cout << "Could not load arial.ttf" << endl;
	loadLevel();
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
// ---------------------------------------------------------------------
bool loadLevel()
{
	ifstream fin("level.txt");
	if (!fin) {
		levelMissing = true;
		return false;
	}
	int rows = 0, cols = 0;
	fin >> rows >> cols;
	for (int r = 0;r < ROWS;r++) {
		for (int c = 0;c < COLS;c++) {
			fin >> bricks[r][c];
		}
	}
	fin.close();
	levelMissing = false;
	return true;
}
// ---------------------------------------------------------------------
//  resetBallOnPaddle()  -  put the ball back, waiting for Space
// ---------------------------------------------------------------------
void resetBallOnPaddle()
{
    ballLaunched = false;
    ballX = paddleX + 44;
    ballY = 638;
    dx = 0;
    dy = 0;

}

// ---------------------------------------------------------------------
//  startNewGame()  -  New Game always starts fresh
// ---------------------------------------------------------------------
void startNewGame()
{
    score = 0;
    lives = START_LIVES;
    if (!loadLevel())
    {
        gameState = MENU;
        return;
    }
    paddleX = PADDLE_START_X;
    resetBallOnPaddle();
    showNoSavedGame = false;
    gameState = PLAYING;
}


// =====================================================================
//  SECTION 6: INPUT  (step 1 of the game loop)
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
    switch (gameState)
    {
    case MENU:
        if (pressedKey == KEY_UP)
        {
            menuChoice--;
            if (menuChoice < OPTION_NEW)
                menuChoice = OPTION_EXIT;
        }
        if (pressedKey == KEY_DOWN)
        {
            menuChoice++;
            if (menuChoice > OPTION_EXIT)
                menuChoice = OPTION_NEW;
        }
        if (pressedKey == KEY_ENTER)
        {
            if (menuChoice == OPTION_NEW)
            {
                startNewGame();
            }
            else if (menuChoice == OPTION_CONTINUE)
            {
                if (!continueGame())
                {
                    showNoSavedGame = true;
                }
                else
                {
                    showNoSavedGame = false;
                }
            }
            else if (menuChoice == OPTION_EXIT)
            {
                window.close();
            }
        }
        break;

    case PLAYING:
        if (pressedKey == KEY_LAUNCH)
        {
            launchBall();
        }
        if (pressedKey == KEY_ESCAPE)
        {
            saveGame();
            gameState = MENU;
        }
        
        break;

    case GAME_OVER:
        if (pressedKey == KEY_ENTER)
        {
            gameState = MENU;
            menuChoice = OPTION_NEW;
        }
        break;
    }
}


// =====================================================================
//  SECTION 7: UPDATE  (step 2 of the game loop)
// =====================================================================
void updateGame()
{
    if (gameState != PLAYING)
        return;   
    movePaddle();
    if(!ballLaunched)
    {
        ballX = paddleX + 44;
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
    if (powerUpFalling)
    {
        powerUpY += 3;

        if (powerUpY + 20 >= PADDLE_Y &&
            powerUpX + 20 >= paddleX &&
            powerUpX <= paddleX + paddleWidth)
        {
            if (powerUpType == 0)
            {
                paddleWidth = 150;
                wideTimer = 600;
            }
            else
            {
                lives++;
            }

            powerUpFalling = false;
        }

        if (powerUpY > WINDOW_H)
        {
            powerUpFalling = false;
        }
    }

    if (wideTimer > 0)
    {
        wideTimer--;

        if (wideTimer == 0)
        {
            paddleWidth = PADDLE_W;
        }
    }
}

// ---------------------------------------------------------------------
//  movePaddle()  -  Left/Right or A/D while HELD down
// ---------------------------------------------------------------------
void movePaddle()
{
	
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
    {
        paddleX = paddleX - PADDLE_SPEED;
    }

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) ||
		sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
	{
        paddleX = paddleX + PADDLE_SPEED;
	}
    if (paddleX < 0)
        paddleX = 0;
    if (paddleX + paddleWidth > WINDOW_W)
        paddleX =  WINDOW_W-paddleWidth ;
	
}

// ---------------------------------------------------------------------
//  launchBall()  -  Space launches the ball upwards
// ---------------------------------------------------------------------
void launchBall()
{
    if (ballLaunched)
        return;
    dy = -5;
    if (rand() % 2 == 0)
        dx = 3;
    else
        dx = -3;
    ballLaunched = true;
	
}

// ---------------------------------------------------------------------
//  moveBall()  -  move in TWO steps: x first, then y
// ---------------------------------------------------------------------
void moveBall()
{
    ballX += dx;
    if (ballX<0 || ballX + BALL_SIZE>WINDOW_W)
    {
        ballX -= dx;
        dx = -dx;
    }
    else if (hitBricks())
    {
        ballX -= dx;
        dx = -dx;
    }
    ballY += dy;
    if (ballY < HUD_H)
    {
        ballY -= dy;
        dy = -dy;
    }
    else if (hitBricks())
    {
        ballY -= dy;
        dy = -dy;
    }
    bounceOnPaddle();
    if (ballY > WINDOW_H)
        loseLife();
}

// ---------------------------------------------------------------------
//  hitBricks()  -  find the bricks under the 4 corners of the ball
//  Returns true if the ball touched at least one brick
// ---------------------------------------------------------------------
bool hitBricks()
{
    int cornerX[4] = { ballX, ballX + BALL_SIZE - 1,ballX,ballX + BALL_SIZE - 1 };
    int cornerY[4] = { ballY,ballY,ballY + BALL_SIZE - 1,ballY + BALL_SIZE - 1 };
    int hitRow[4], hitCol[4];
    int hits = 0;
    for (int i = 0;i < 4;i++)
    {
        int cx = cornerX[i];
        int cy = cornerY[i];
        if (cy < BRICKS_Y || cy >= BRICKS_Y + ROWS * BRICK_H || cx < 0 || cx >= WINDOW_W)
            continue;
        int row = (cy - (int)BRICKS_Y) / (int)BRICK_H;
        int col = cx / (int)BRICK_W;
        if (bricks[row][col] == 0)
            continue;
        bool already = false;
        for (int k = 0;k < hits;k++)
            if (hitRow[k] == row && hitCol[k] == col)
                already = true;
        if (!already)
         {
                hitRow[hits] = row;
                hitCol[hits] = col;
                hits++;
         }

    }
    for (int k = 0;k < hits;k++)
    {
        int r = hitRow[k];
        int c = hitCol[k];
        bricks[r][c]--;
        if (bricks[r][c] == 0)
        {
            score += POINTS_BREAK;
            if (!powerUpFalling && rand() % 5 == 0)
            {
                powerUpFalling = true;
                powerUpX = c * (int)BRICK_W + (int)BRICK_W / 2;
                powerUpY = (int)BRICKS_Y + r * (int)BRICK_H;
                powerUpType = rand() % 2;
            }
        }
        else
            score += POINTS_HIT;
    }
    return hits > 0;
}

// ---------------------------------------------------------------------
//  bounceOnPaddle()  -  the paddle decides the new direction
//  MARKS: Feature 4 (5 marks: only moving down 2 + dx from part 3)
//  WHO: Member 2
// ---------------------------------------------------------------------
void bounceOnPaddle()
{
    if (dy <= 0)
        return;
    bool overlapX = (ballX + BALL_SIZE > paddleX) && (ballX < paddleX + paddleWidth);
    bool overlapY = (ballY + BALL_SIZE > PADDLE_Y) && (ballY < PADDLE_Y + PADDLE_H);
    if (overlapX && overlapY)
    {
        ballY = PADDLE_Y - BALL_SIZE;
        dy = -5;
        int part = (ballX + BALL_SIZE / 2 - paddleX) * 4 / paddleWidth;
        if (part < 0)
            part = 0;
        if (part > 3)
            part = 3;
        if (part == 0)
            dx = -6;
        else if (part == 1)
            dx = -3;
        else if (part == 2)
            dx = 3;
        else
            dx = 6;
    }
}

// ---------------------------------------------------------------------
//  loseLife()  -  the ball fell past the paddle
//  MARKS: Feature 6 (3 marks)
//  WHO: Member 3
// ---------------------------------------------------------------------
void loseLife()
{
	lives--;
	if (lives == 0)
	{
		playerWon = false;
		clearSave();
		gameState = GAME_OVER;
	}
}
bool allBricksBroken()
{
	for (int r = 0; r < ROWS; r++)
	{
		for (int c = 0; c < COLS; c++)
		{
			if (bricks[r][c] > 0)
			{
				return false;
			}
		}
	}
	return true;
}
void saveGame()
{
    ofstream fout("savegame.txt");
    if (!fout)
    {
        cout << "Unable to save game!" << endl;
        return;
    }
    fout << score << " " << lives << endl;
    for (int r = 0;r < ROWS; r++)
    {
        for (int c = 0;c < COLS; c++)
        {
            fout << bricks[r][c] << " ";
        }
        fout << endl;
    }
    fout.close();
}

bool continueGame()
{

    ifstream fin("savegame.txt");
    if (!fin)
    {
        return false;
    }
    if (!(fin >> score >> lives))
    {
        return false;
    }
    for (int r = 0;r < ROWS;r++)
    {
        for (int c = 0;c < COLS;c++)
        {
            if (!(fin >> bricks[r][c]))
            {
                fin.close();
                return false;
            }
            if (bricks[r][c] < 0 || bricks[r][c]>3)
            {
                fin.close();
                return false;
            }
        }
    }
    fin.close();
    paddleX = PADDLE_START_X;
    resetBallOnPaddle();
    gameState = PLAYING;
    return true;
}

void clearSave()
{
    ofstream fout("savegame.txt");
    fout.close();
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

	sf::RectangleShape brick({ BRICK_W - 2, BRICK_H - 2 });   // smaller, so a gap shows
	for (int r = 0;r < ROWS;r++) {
		for (int c = 0;c < COLS;c++) {
			if (bricks[r][c] == 0) {
				continue;
			}
			brick.setFillColor(colors[bricks[r][c]]);
			  brick.setPosition({c * BRICK_W + 1, BRICKS_Y + r * BRICK_H + 1});
			  window.draw(brick);
		}
	}
	
	
}

// ---------------------------------------------------------------------
//  drawPaddleAndBall()  -  WHO: Member 1
// ---------------------------------------------------------------------
void drawPaddleAndBall(sf::RenderWindow& window)
{
	sf::RectangleShape paddle({ (float)paddleWidth, 14.f });
	sf::CircleShape ball(6.f);              // radius 6, fits in a 12 x 12 box

	paddle.setFillColor(sf::Color::Cyan);
	ball.setFillColor(sf::Color::White);

	paddle.setPosition({ (float)paddleX, 650.f });
	ball.setPosition({ (float)ballX, (float)ballY });
	window.draw(paddle);
	window.draw(ball);

    if (powerUpFalling)
    {
        string symbol;

        if (powerUpType == 0)
            symbol = "W";
        else
            symbol = "+";

        sf::Text powerText(font, symbol, 20);

        powerText.setPosition({
            (float)powerUpX,
            (float)powerUpY
            });

        powerText.setFillColor(sf::Color::Yellow);

        window.draw(powerText);
    }
	// BONUS: if wide paddle is on, the paddle size is 150 instead of 100
	// BONUS: draw the falling power-up ("W" or "+") here
}
void drawHUD(sf::RenderWindow& window)
{
    sf::Text scoreText(font, "SCORE: " + to_string(score), 20);
    scoreText.setPosition({ 10.f, 12.f });
    scoreText.setFillColor(sf::Color::White);

    window.draw(scoreText);

    sf::Text livesText(font, "LIVES: " + to_string(lives), 20);
    livesText.setPosition({ 460.f, 12.f });
    livesText.setFillColor(sf::Color::White);

    window.draw(livesText);

    sf::RectangleShape line({ 600.f, 2.f });
    line.setPosition({ 0.f, 50.f });
    line.setFillColor(sf::Color::White);

    window.draw(line);
}
void drawMenu(sf::RenderWindow& window)
{
    sf::Text title(font, "BLOCK BUSTER", 40);
    title.setPosition({ 135.f, 130.f });
    title.setFillColor(sf::Color::Cyan);
    window.draw(title);
    string options[3] =
    {
        "NEW GAME",
        "CONTINUE",
        "EXIT"
    };
    for (int i = 0; i < 3; i++)
    {
        sf::Text option(font, options[i], 25);

        option.setPosition({ 220.f, 300.f + i * 60.f });

        if (i == menuChoice)
        {
            option.setFillColor(sf::Color::Yellow);
        }
        else
        {
            option.setFillColor(sf::Color::White);
        }

        window.draw(option);
    }
    if (showNoSavedGame)
    {
        sf::Text message(font, "No saved game", 20);

        message.setPosition({ 200.f, 520.f });
        message.setFillColor(sf::Color::Red);

        window.draw(message);
    }
    if (levelMissing)
    {
        sf::Text error(font, "Error: level.txt not found", 18);

        error.setPosition({ 150.f, 560.f });
        error.setFillColor(sf::Color::Red);

        window.draw(error);
    }
}
void drawEndScreen(sf::RenderWindow& window)
{
    string message;
    if (playerWon)
    {
        message = "YOU WIN!";
    }
    else
    {
        message = "GAME OVER";
    }
    sf::Text result(font, message, 40);
    result.setPosition({ 160.f, 230.f });
    if (playerWon)
        result.setFillColor(sf::Color::Green);
    else
        result.setFillColor(sf::Color::Red);
    window.draw(result);
    sf::Text finalScore(font, "FINAL SCORE: " + to_string(score), 25);
    finalScore.setPosition({ 170.f, 330.f });
    window.draw(finalScore);
    sf::Text instruction(font, "Press Enter to return to menu", 18);
    instruction.setPosition({ 130.f, 430.f });
    window.draw(instruction);
}

