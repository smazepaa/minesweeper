#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <cstdlib> // For rand() and srand()
#include <ctime>   // For time()

using namespace std;
using namespace sf;

float CELL_SIZE = 50;
bool LOST = false;
float ADDITIONAL_SPACE = 100;
int REMAINING_BOMBS = 0;

enum class Level
{
    easy,
    intermediate,
    expert
};

class Cell {
    int row, column = 0;
    bool bomb = false;
    bool flagged = false;
    bool opened = false;
    RectangleShape shape;

public:

    int neighborBombs = INT16_MAX;

    Cell(){}

    Cell(int r, int c) : row(r), column(c) {
        shape.setSize(Vector2f(CELL_SIZE, CELL_SIZE));
        shape.setPosition(c * CELL_SIZE, r * CELL_SIZE + ADDITIONAL_SPACE);
        shape.setOutlineColor(Color::Black);
        shape.setOutlineThickness(1);
        shape.setFillColor(Color(182, 189, 200));
    };

    void open(RenderWindow& window) {
        this->opened = true;
        if (this->bomb) {
            this->shape.setFillColor(Color(99, 0, 16));
            LOST = true;
            return;
        }
        this->shape.setFillColor(Color(241, 242, 243));
    }

    void toggleFlag() {
        if (!this->opened) {
            
            if (!this->flagged) {
                this->flagged = true;
                REMAINING_BOMBS--;
            }
            else {
                this->flagged = false;
                REMAINING_BOMBS++;
            }
        }
    }

    bool isFlagged() const {
		return this->flagged;
	}

    void drawFlag(RenderWindow& window) {
        static Texture flag_texture; // static to load the texture only once
        if (flag_texture.getSize().x == 0) {
            flag_texture.loadFromFile("icons/red-flag.png");
        }

        if (flagged && !opened) {
            createSprite(window, flag_texture);
        }
    }

    void drawNumberBombs(RenderWindow& window) {
        Texture number_texture;
        bool textureLoaded = false;

        switch (neighborBombs) {
        case 1:
            textureLoaded = number_texture.loadFromFile("icons/one.png");
            break;
        case 2:
            textureLoaded = number_texture.loadFromFile("icons/two.png");
            break;
        case 3:
            textureLoaded = number_texture.loadFromFile("icons/three.png");
            break;
        case 4:
            textureLoaded = number_texture.loadFromFile("icons/four.png");
            break;
        case 5:
            textureLoaded = number_texture.loadFromFile("icons/five.png");
            break;
        case 6:
            textureLoaded = number_texture.loadFromFile("icons/six.png");
            break;
        case 7:
            textureLoaded = number_texture.loadFromFile("icons/seven.png");
            break;
        case 8:
            textureLoaded = number_texture.loadFromFile("icons/eight.png");
            break;
        }

        if (opened && textureLoaded) {
            createSprite(window, number_texture);
        }
    }

    void createSprite(RenderWindow& window, Texture& texture) {

        Sprite sprite(texture);
        sprite.setTextureRect(IntRect(0, 0, 40, 40));
        
        float sprite_size = 40;
        float dif = (CELL_SIZE - sprite_size) / 2;
        sprite.setPosition(column * CELL_SIZE + dif, row * CELL_SIZE + ADDITIONAL_SPACE + dif);
        window.draw(sprite);
    }

    bool isBomb() const {
        return this->bomb;
    }

    bool isOpen() const {
		return this->opened;
	}

    int getRow() const {
        return this->row;
    }

    int getColumn() const {
        return this->column;
    }

    void setBomb() {
        this->bomb = true;
    }

    void draw(RenderWindow& window) {
        if (opened && bomb) {
            this->shape.setFillColor(Color(126, 27, 33));
        }
        else if (opened) {
            this->shape.setFillColor(Color(241, 242, 243));
		}
        else {
			this->shape.setFillColor(Color(182, 189, 200));
		}
        window.draw(this->shape);
    }

    void drawBomb(RenderWindow& window) {
        static Texture bomb_texture;
        if (bomb_texture.getSize().x == 0) {
            bomb_texture.loadFromFile("icons/bomb.png");
        }

        if (bomb) {
            createSprite(window, bomb_texture);
        }
    }

    void reset() {
        bomb = false;
        flagged = false;
        opened = false;
        neighborBombs = INT16_MAX;
        shape.setFillColor(Color(182, 189, 200));
    }
};


class Board {
    int rows, columns, bombs, closedCells = 0;
    vector<vector<Cell*>> cells;

    bool firstMove = true;

    // generate bombs, excluding the first clicked cell and its neighbors
    void generateBombs(int clickedRow, int clickedCol) {
        int bombsToAdd = bombs;
        srand(static_cast<unsigned>(time(nullptr))); // Seed the random number generator
        while (bombsToAdd > 0) {
            int row = rand() % rows;
            int column = rand() % columns;

            if (!cells[row][column]->isBomb() &&
                !(abs(row - clickedRow) <= 1 && abs(column - clickedCol) <= 1)) {
                cells[row][column]->setBomb();
                --bombsToAdd;
            }
        }
    }

    void calculateNghbBombs() {
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < columns; ++j) {
                if (!cells[i][j]->isBomb()) {
                    int bombs = 0;
                    for (int k = i - 1; k <= i + 1; ++k) {
                        for (int l = j - 1; l <= j + 1; ++l) {
                            if (k >= 0 && k < rows && l >= 0 && l < columns) {
                                if (cells[k][l]->isBomb()) {
                                    ++bombs;
                                }
                            }
                        }
                    }
                    cells[i][j]->neighborBombs = bombs;
                }
            }
        }
    }

public:

    Board(int r, int c, int bombs) :
        rows(r), columns(c), bombs(bombs), closedCells(r* c) {
        // Initialize the board with empty cells
        REMAINING_BOMBS = bombs;
        cells = vector<vector<Cell*>>(rows, vector<Cell*>(columns));

        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < columns; ++j) {
                cells[i][j] = new Cell(i, j);
            }
        }
    }

    void reset() {
        // Close all cells, remove flags, and reset bombs
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < columns; ++j) {
                cells[i][j]->reset();
            }
        }

        // Reset game state
        firstMove = true;
        closedCells = rows * columns;
        REMAINING_BOMBS = bombs;
    }

    Cell& getCell(int row, int column) const {
        return *(this->cells[row][column]);
    }

    int getRows() const {
        return this->rows;
    }

    int getColumns() const {
        return this->columns;
    }

    void openCells(int row, int col, RenderWindow& window) {
        if (row < 0 || col < 0 || row >= rows || col >= columns) {
            return; // Check for out-of-bounds
        }

        Cell& cell = *(cells[row][col]);
        if (cell.isOpen() || cell.isFlagged()) {
            return; // If cell is already open or flagged, return
        }

        cell.open(window);

        if (firstMove) {
            firstMove = false;
            generateBombs(row, col);
            calculateNghbBombs();
        }

        // if the cell has no neighboring bombs, recursively open adjacent neighbors
        if (cell.neighborBombs == 0) {
            for (int k = row - 1; k <= row + 1; ++k) {
                for (int l = col - 1; l <= col + 1; ++l) {
                    if ((k != row || l != col) && k >= 0 && l >= 0 && k < rows && l < columns) {
                        openCells(k, l, window);
                    }
                }
            }
        }
    }

    bool checkWinCondition() const {
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < columns; ++j) {
                const Cell& cell = *(cells[i][j]);
                if ((!cell.isOpen() && !cell.isBomb()) || (cell.isBomb() && !cell.isFlagged())) {
                    return false; // If a non-bomb cell is closed or a bomb cell isn't flagged, game is not won
                }
            }
        }
        return true; // All non-bomb cells are opened, and all bomb cells are flagged
    }

};

class Minesweeper {
	Board board;
	Level level;

public:
    Minesweeper(): board(8, 8, 10), level(Level::easy) {};
    
    Board& getBoard() {
		return this->board;
	}

    void resetBoard() {
        board.reset();  // Call the reset method of the Board
    }
    
    void setLevel(Level newLevel) {
        level = newLevel;
        // Reinitialize the board with new level settings
        switch (level) {
        case Level::easy:
            board = Board(8, 8, 10);
            break;
        case Level::intermediate:
            board = Board(15, 15, 40);
            break;
        case Level::expert:
            board = Board(16, 32, 99);
            break;
        }
    }

};

class Renderer {   
    Board& board;
    RenderWindow window;
    Minesweeper& game;
    Font font;
    Clock timer; // Timer to keep track of elapsed time

    int seconds = 0; // Elapsed time in seconds
    bool suspiciousMode = false;
    bool carefulMode = false;
    int row, col;
    bool firstClick = true;
    bool gameOver = false;

    bool dropdownOpen = false;
    vector<pair<string, Level>> levels = { {"Easy", Level::easy}, {"Intermediate", Level::intermediate}, {"Expert", Level::expert} };
    RectangleShape dropdown;
    vector<Text> levelTexts;
    vector<RectangleShape> dropdownRects;

    void loadFont() {
        if (!font.loadFromFile("font/Montserrat-Bold.ttf")) {
            cerr << "Failed to load font" << endl;
        }
    }

    void updateWindowSize() {
        
        int windowWidth = board.getColumns() * CELL_SIZE;
        int windowHeight = board.getRows() * CELL_SIZE + ADDITIONAL_SPACE;
        window.setSize(Vector2u(windowWidth, windowHeight));
        window.setView(View(FloatRect(0, 0, windowWidth, windowHeight)));
        restartGame();
        //gameOver = false;
    }

    void handleEvents() {
        Event event;

        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
            }
            else if (event.type == Event::MouseButtonPressed) {
                Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);

                if (dropdownOpen || dropdown.getGlobalBounds().contains(mousePos)) {
                    handleDropdownEvent(event);
                }
                else {
                    // Handle cell interactions if the dropdown is not open
                    int x = event.mouseButton.x;
                    int y = event.mouseButton.y - ADDITIONAL_SPACE;

                    FloatRect smileyBounds(
                        board.getColumns() * CELL_SIZE / 2 - 30,
                        (ADDITIONAL_SPACE - 60) / 2,
                        60, 60);

                    // Check if click is within smiley face boundaries
                    if (smileyBounds.contains(x, y + ADDITIONAL_SPACE)) {
                        restartGame();
                        return;
                    }

                    if (LOST || board.checkWinCondition()) {
                        return;
                    }

                    // Ensure the click is within the board boundaries
                    if (x >= 0 && x < board.getColumns() * CELL_SIZE &&
                        y >= 0 && y < board.getRows() * CELL_SIZE + ADDITIONAL_SPACE) {
                        row = y / CELL_SIZE;
                        col = x / CELL_SIZE;

                        if (event.mouseButton.button == Mouse::Left || event.mouseButton.button == Mouse::Right) {
                            // Start the timer on the first valid cell click
                            if (firstClick) {
                                timer.restart();
                                firstClick = false;
                            }
                            handleCellClick(event.mouseButton.button);
                        }
                    }
                }
            }
        }
    }

    void handleCellClick(Mouse::Button button) {
        if (button == Mouse::Left) {
            carefulMode = false;
            if (row >= 0 && col >= 0 && row < board.getRows() && col < board.getColumns() &&
                !board.getCell(row, col).isFlagged()) {
                // Open the cell and its neighbors
                board.openCells(row, col, window);
            }
        }
        else if (button == Mouse::Right) {
            suspiciousMode = false;
            if (row >= 0 && col >= 0 && row < board.getRows() && col < board.getColumns()) {
                board.getCell(row, col).toggleFlag();
            }
        }
    }

    void drawTimer() {
        if (!gameOver) { // Only update timer if game is not over
            if (firstClick) {
                seconds = 0;
            }
            else {
                seconds = static_cast<int>(timer.getElapsedTime().asSeconds());
            }
        }
        
        Text timerText("Time: " + to_string(seconds) + "s", font, 20);
        timerText.setFillColor(Color(241, 242, 243));
        timerText.setStyle(Text::Bold);

        float centerX = window.getSize().x / 2.0f - timerText.getLocalBounds().width / 2.0f;
        timerText.setPosition(10, (ADDITIONAL_SPACE - timerText.getLocalBounds().height) / 2);

        window.draw(timerText);
    }

    void RemainingBombsCount() {
        Text bombsText("Bombs: " + to_string(REMAINING_BOMBS), font, 20);
        bombsText.setFillColor(Color(241, 242, 243));
        bombsText.setStyle(Text::Bold);

        float posX = window.getSize().x - bombsText.getLocalBounds().width - 10;
        bombsText.setPosition(posX, (ADDITIONAL_SPACE - bombsText.getLocalBounds().height) / 2);

        window.draw(bombsText);
    }

    void setupDropdown() {

        dropdown.setFillColor(Color(192, 192, 192));
        dropdown.setSize(Vector2f(140, 30));
        dropdown.setPosition(10, 10); 

        // Setup level texts
        for (int i = 0; i < levels.size(); ++i) {
            Text text(levels[i].first, font, 20);
            text.setPosition(12, 40 + i * 30);
            text.setFillColor(Color::Magenta);
            levelTexts.push_back(text);

            // Create a rectangle for each dropdown option
            RectangleShape rect;
            rect.setSize(Vector2f(140, 30));
            rect.setPosition(10, 40 + i * 30);
            rect.setFillColor(Color(192, 192, 192));
            rect.setOutlineColor(Color::Black);
            dropdownRects.push_back(rect);
        }
    }

    void handleDropdownEvent(Event event) {
        if (event.type == Event::MouseButtonPressed) {
            Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);
            if (dropdown.getGlobalBounds().contains(mousePos)) {
                dropdownOpen = !dropdownOpen;
            }
            else if (dropdownOpen) {

                for (int i = 0; i < levelTexts.size(); ++i) {
                    if (levelTexts[i].getGlobalBounds().contains(mousePos)) {
                        game.setLevel(levels[i].second);
                        dropdownOpen = false;
                        restartGame(); // Restart game with new level
                        updateWindowSize();
                        break;
                    }
                }
            }
        }
    }

    void draw() {
        window.clear(Color(22, 25, 29));

        drawBob();
        drawTimer();
        RemainingBombsCount();
        drawCells();

        if (LOST) {
            for (int i = 0; i < board.getRows(); ++i) {
                for (int j = 0; j < board.getColumns(); ++j) {

                    if (board.getCell(i, j).isBomb() && !board.getCell(i, j).isFlagged()) {
                        board.getCell(i, j).drawBomb(window);
                    }
                }
            }

            drawMessage("You Lost", Color::Red);
        }

        if (board.checkWinCondition()) {
			drawMessage("You Won", Color::Green);
		}

        drawDropdown();

        window.display();
    }

    void drawCells() {
        for (int i = 0; i < board.getRows(); ++i) {
            for (int j = 0; j < board.getColumns(); ++j) {

                Cell cell = board.getCell(i, j);
                cell.draw(window); // initial draw

                // depending on the state of the cell, draw the appropriate icon
                if (cell.isFlagged()) {
                    cell.drawFlag(window);
                }

                else if (cell.isOpen()) {
                    if (cell.isBomb()) {
                        cell.drawBomb(window);
                    }
                    else {
                        cell.drawNumberBombs(window);
                    }
                }
            }
        }
    }

    void drawMessage(const string& message, const Color& color) {
        gameOver = true;
        Text wonText(message, font, 60);
        wonText.setFillColor(color);
        wonText.setStyle(Text::Bold);

        float centerX = window.getSize().x / 2.0f - wonText.getLocalBounds().width / 2.0f;
        float centerY = window.getSize().y / 2.0f - wonText.getLocalBounds().height / 2.0f;
        wonText.setPosition(centerX, centerY);

        window.draw(wonText);
    }

    void drawBob() {
        Texture smiley_texture;
        if (LOST) {
            smiley_texture.loadFromFile("icons/loser.png");
        }
        else if (board.checkWinCondition()) {
			smiley_texture.loadFromFile("icons/winner.png");
		}
        else if (suspiciousMode) {
            smiley_texture.loadFromFile("icons/suspicious.png");
        }
        else if (carefulMode) {
			smiley_texture.loadFromFile("icons/careful.png");
		}
        else {
            smiley_texture.loadFromFile("icons/smiling.png");
        }

        Sprite bob(smiley_texture);
        bob.setTextureRect(IntRect(0, 0, 60, 60));
        float smiley_size = 60;
        float dif = (ADDITIONAL_SPACE - smiley_size) / 2;
        bob.setPosition(board.getColumns() * CELL_SIZE / 2 - smiley_size / 2, dif);
        window.draw(bob);
    }

    void drawDropdown() {
        window.draw(dropdown);
        
        Text title("Levels:", font, 20);
        title.setPosition(12, 10); // Adjust position as needed
        title.setFillColor(Color::Black);
        window.draw(title);

        if (dropdownOpen) {
            for (auto& rect : dropdownRects) {
                window.draw(rect);
            }

            for (auto& text : levelTexts) {
                window.draw(text);
            }
        }
    }

    void restartGame() {
        window.clear();
        game.resetBoard();
        board = game.getBoard();
        LOST = false; 
        suspiciousMode = false;
        carefulMode = false;
        row = -1;
        col = -1;
        firstClick = true;
        gameOver = false;
        timer.restart();
    }

public:
    Renderer(Minesweeper& m) : game(m), board(m.getBoard()) {
        int windowWidth = board.getColumns() * CELL_SIZE;
        int windowHeight = board.getRows() * CELL_SIZE + ADDITIONAL_SPACE;

        window.create(VideoMode(windowWidth, windowHeight), "Minesweeper");
        window.setFramerateLimit(60);

        loadFont();
        setupDropdown();
    }

    void run() {
        while (window.isOpen()) {
            handleEvents();
            draw();
		}
	}
};

int main() {
    Minesweeper game;
    Renderer renderer(game);
    renderer.run();
    return 0;
}
