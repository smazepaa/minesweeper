#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <cstdlib> // For rand() and srand()
#include <ctime>   // For time()

using namespace std;
using namespace sf;

float CELL_SIZE = 50;

enum class Level
{
    easy,
    intermediate,
    expert
};

class Cell {
    int row, column = 0;
    bool bomb = false;
    bool flagged, opened = false;
    RectangleShape shape;

public:

    int neighborBombs = 0;

    Cell(){}

    Cell(int r, int c) : row(r), column(c) {
        shape.setSize(Vector2f(CELL_SIZE, CELL_SIZE));
        shape.setPosition(c * CELL_SIZE, r * CELL_SIZE);
        shape.setOutlineColor(Color::Black);
        shape.setOutlineThickness(1);
        shape.setFillColor(Color(192, 192, 192));
    };

    void open(RenderWindow& window) {
        this->opened = true;
        if (this->bomb) {
            this->shape.setFillColor(Color::Red);
        }
        else { 
            switch (neighborBombs) {
				case 0:
					this->shape.setFillColor(Color::White);
					break;
				case 1:
					this->shape.setFillColor(Color::Blue);
					break;
				case 2:
					this->shape.setFillColor(Color::Green);
					break;
				case 3:
					this->shape.setFillColor(Color::Red);
					break;
				case 4:
					this->shape.setFillColor(Color::Magenta);
					break;
				case 5:
					this->shape.setFillColor(Color::Cyan);
					break;
				case 6:
					this->shape.setFillColor(Color::Yellow);
					break;
				case 7:
					this->shape.setFillColor(Color(255, 128, 0));
					break;
				case 8:
					this->shape.setFillColor(Color(128, 128, 128));
					break;
			}
            //this->shape.setFillColor(Color::White);

            ////The one thing I hate about SFML is that in order to draw an image, I have to make 2 objects
            //Sprite icon_sprite;

            //Texture icons_texture;
            //if (!icons_texture.loadFromFile("C:/Users/sofma/source/repos/minesweeper/sfml-stp2/x64/Debug/Icons16.png")) {
            //    cerr << "Error loading Icons16.png" << endl;
            //}

            //icon_sprite.setTexture(icons_texture);

            //if (neighborBombs > 0) {
            //    icon_sprite.setTextureRect(IntRect(16 * neighborBombs, 0, 16, 16));
            //    icon_sprite.setPosition(row, column);
            //    window.draw(icon_sprite);
            //}
        }
    }

    void toggleFlag() {
        if (!this->opened) 
        {
            if (this->flagged) 
            {
                this->shape.setFillColor(Color::Yellow);
            }
            else {
                this->shape.setFillColor(Color(192, 192, 192));
            }
            this->flagged = !flagged;
        }
    }

    bool isBomb() const {
        return this->bomb;
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
        //if (bomb) {
        //    this->shape.setFillColor(Color::Red);
        //}
        window.draw(this->shape);
    }

    RectangleShape getShape() const {
		return this->shape;
	}
};


class Board {
    int rows, columns, bombs, closedCells, remainingBombs = 0;
    vector<vector<Cell*>> cells;

    void generateBombs() {
        int bombsToAdd = bombs;
        while (bombsToAdd > 0) {
            // Seed the random number generator
            srand(static_cast<unsigned>(time(nullptr))); 

            // Generate random row and column
            int row = rand() % rows;
            int column = rand() % columns;

            if (!cells[row][column]->isBomb()) {
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
        generateBombs();
        calculateNghbBombs();
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
};

class Minesweeper {
	Board board;
	Level level;

public:
    Minesweeper() : board(9, 9, 10), level(Level::easy) {};
    
    Board& getBoard() {
		return this->board;
	}
    
    void setLevel(Level l) {
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
            if (event.type == Event::Closed) { // If the window is closed
                window.close(); // Close the window
            }
            else if (event.type == Event::MouseButtonPressed) {
                // Get the position of the mouse
                int x = event.mouseButton.x;
                int y = event.mouseButton.y;

                // Get the row and column of the cell
                int row = y / CELL_SIZE;
                int column = x / CELL_SIZE;

                if (event.mouseButton.button == Mouse::Left) {
                    // Open the cell
                    board.getCell(row, column).open(window);
                }
                else if (event.mouseButton.button == Mouse::Right) {
                    // Toggle the flag of the cell
                    board.getCell(row, column).toggleFlag();
                }
            };
        }
    }

    void draw() {

        window.clear();
        // Draw each cell on the window
        for (int i = 0; i < board.getRows(); ++i) {
            for (int j = 0; j < board.getColumns(); ++j) {
                board.getCell(i, j).draw(window);
            }
        }
        window.display();
    }

public:
    Renderer(Minesweeper& m) : game(m), board(m.getBoard()) {
        window.create(sf::VideoMode(board.getColumns() * CELL_SIZE, 
            board.getRows() * CELL_SIZE), "Minesweeper", 
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
