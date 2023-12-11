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
        shape.setFillColor(Color(192, 192, 192));
    };

    void open(RenderWindow& window) {
        this->opened = true;
        if (this->bomb) {
            this->shape.setFillColor(Color::Red);
            LOST = true;
            return;
        }
        this->shape.setFillColor(Color::White);
    }

    void toggleFlag() {
        if (!this->opened) {
            this->flagged = !this->flagged;
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

        switch (neighborBombs) {
        case 1:
            number_texture.loadFromFile("icons/one.png");
            break;
        case 2:
            number_texture.loadFromFile("icons/two.png");
            break;
        case 3:
            number_texture.loadFromFile("icons/three.png");
            break;
        case 4:
            number_texture.loadFromFile("icons/four.png");
            break;
        case 5:
            number_texture.loadFromFile("icons/five.png");
            break;
        case 6:
            number_texture.loadFromFile("icons/six.png");
            break;
        case 7:
            number_texture.loadFromFile("icons/seven.png");
            break;
        case 8:
            number_texture.loadFromFile("icons/eight.png");
            break;
        }

        if (opened) {
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
            this->shape.setFillColor(Color::Red);
        }
        else if (opened) {
			this->shape.setFillColor(Color::White);
		}
        else {
			this->shape.setFillColor(Color(192, 192, 192));
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
};


class Board {
    int rows, columns, bombs, closedCells, remainingBombs = 0;
    vector<vector<Cell*>> cells;

    bool firstMove = true;

    // generate bombs, excluding the first clicked cell and its neighbors
    void generateBombs(int clickedRow, int clickedCol) {
        int bombsToAdd = bombs;
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
        rows(r), columns(c), bombs(bombs), closedCells(r* c), remainingBombs(bombs){
        // Initialize the board with empty cells
        cells = vector<vector<Cell*>>(rows, vector<Cell*>(columns));

        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < columns; ++j) {
                cells[i][j] = new Cell(i, j);
            }
        }
    }

    Cell& getCell(int row, int column) const {
        return *(this->cells[row][column]);
    }

    int getClosed() const {
        return this->closedCells;
    }

    int getRemainedBombs() const {
        return this->remainingBombs;
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

};

class Minesweeper {
	Board board;
	Level level;

public:
    Minesweeper() : board(8, 8, 10), level(Level::easy) {};
    
    Board& getBoard() {
		return this->board;
	}
    
    void setLevel(Level& l) {
		this->level = l;
	}
};

class Renderer {   
    Board& board;
    RenderWindow window;
    Minesweeper& game;

    void handleEvents() {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
            }
            else if (event.type == Event::MouseButtonPressed) {
                if (LOST) return;
                int x = event.mouseButton.x;
                int y = event.mouseButton.y - ADDITIONAL_SPACE;

                // ensure the click is within the board boundaries
                if (x >= 0 && x < board.getColumns() * CELL_SIZE &&
                    y >= 0 && y < board.getRows() * CELL_SIZE + ADDITIONAL_SPACE) {
                    int row = y / CELL_SIZE;
                    int col = x / CELL_SIZE;

                    if (event.mouseButton.button == Mouse::Left) {
                        if (!board.getCell(row, col).isFlagged()) {
                            // open the cell and its neighbors
                            board.openCells(row, col, window);
                        }
                    }
                    else if (event.mouseButton.button == Mouse::Right) {
                        board.getCell(row, col).toggleFlag();
                    }
                }
            }
        }
    }

    void draw() {
        window.clear();

        for (int i = 0; i < board.getRows(); ++i) {
            for (int j = 0; j < board.getColumns(); ++j) {

                board.getCell(i, j).draw(window); // initial draw

                // depending on the state of the cell, draw the appropriate icon
                if (board.getCell(i, j).isFlagged()) {
                    board.getCell(i, j).drawFlag(window);
                }

                else if (board.getCell(i, j).isOpen()) {
					
                    if (board.getCell(i, j).isBomb()) {
						board.getCell(i, j).drawBomb(window);
					}
                    else {
                        board.getCell(i, j).drawNumberBombs(window);
                    }
				}
            }
        }

        drawLostMessage();

        window.display();
    }

    void drawLostMessage() {
        if (LOST) {
            Font font;
            if (!font.loadFromFile("font/Montserrat-Bold.ttf")) {
                cerr << "Failed to load font" << endl;
                return;
            }

            for (int i = 0; i < board.getRows(); ++i) {
                for (int j = 0; j < board.getColumns(); ++j) {

                    if (board.getCell(i, j).isBomb()) {
                        board.getCell(i, j).drawBomb(window);
                    }

                }
            }

            Text lostText("You Lost", font, 60);
            lostText.setFillColor(Color::Red);
            lostText.setStyle(Text::Bold);

            float centerX = window.getSize().x / 2.0f - lostText.getLocalBounds().width / 2.0f;
            float centerY = window.getSize().y / 2.0f - lostText.getLocalBounds().height / 2.0f;
            lostText.setPosition(centerX, centerY);

            window.draw(lostText);
        }
    }

public:
    Renderer(Minesweeper& m) : game(m), board(m.getBoard()) {
        int windowWidth = board.getColumns() * CELL_SIZE;
        int windowHeight = board.getRows() * CELL_SIZE + ADDITIONAL_SPACE;

        window.create(VideoMode(windowWidth, windowHeight), "Minesweeper",
            Style::Titlebar | Style::Close);
        window.setFramerateLimit(60);
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
