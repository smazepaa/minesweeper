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

    void open() {
        this->opened = true;
        if (this->bomb) {
            this->shape.setFillColor(Color::Red);
        }
        else {
            // Show the number of neighboring bombs if it's greater than 0
            if (this->neighborBombs > 0) {
                // Assuming you have a font loaded and set up for text display
                Text text(std::to_string(this->neighborBombs), FONT, 20); // Change 20 to your desired font size
                // Calculate the position to place the text at the center of the cell
                float textX = this->column * CELL_SIZE + (CELL_SIZE / 2.0f) - text.getLocalBounds().width / 2.0f;
                float textY = this->row * CELL_SIZE + (CELL_SIZE / 2.0f) - text.getLocalBounds().height / 2.0f;
                text.setPosition(textX, textY);
                window.draw(text);
            }
            else {
                this->shape.setFillColor(Color::White);
            }
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
        if (bomb) {
            this->shape.setFillColor(Color::Red);
        }
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

    void calculateNghbBombs() {
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < columns; ++j) {
                if (cells[i][j]->isBomb()) {
                    if (i > 0 && j > 0 && !cells[i - 1][j - 1]->isBomb()) {
						cells[i - 1][j - 1]->neighborBombs++;
					}
                    if (i > 0 && !cells[i - 1][j]->isBomb()) {
						cells[i - 1][j]->neighborBombs++;
					}
                    if (i > 0 && j < columns - 1 && !cells[i - 1][j + 1]->isBomb()) {
						cells[i - 1][j + 1]->neighborBombs++;
					}
                    if (j > 0 && !cells[i][j - 1]->isBomb()) {
						cells[i][j - 1]->neighborBombs++;
					}
                    if (j < columns - 1 && !cells[i][j + 1]->isBomb()) {
						cells[i][j + 1]->neighborBombs++;
					}
                    if (i < rows - 1 && j > 0 && !cells[i + 1][j - 1]->isBomb()) {
						cells[i + 1][j - 1]->neighborBombs++;
					}
                    if (i < rows - 1 && !cells[i + 1][j]->isBomb()) {
						cells[i + 1][j]->neighborBombs++;
					}
                    if (i < rows - 1 && j < columns - 1 && !cells[i + 1][j + 1]->isBomb()) {
						cells[i + 1][j + 1]->neighborBombs++;
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
                    board.getCell(row, column).open();
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

    void createButtons(float centerX, float centerY, Font font, string text) {

		// Calculate button position
		float buttonWidth = 220.0f;
		float buttonHeight = 40.0f;
		float buttonX = centerX - (buttonWidth / 2.0f);
		float buttonY = centerY - (buttonHeight / 2.0f);

		// Draw buttons or UI elements for level selection
		RectangleShape easyButton(Vector2f(buttonWidth, buttonHeight));
		easyButton.setPosition(buttonX, buttonY); // Position the button
		easyButton.setFillColor(Color::Red);
		window.draw(easyButton);

		// Display text within the button
		Text easyText(text, font, 30); // Text for "Easy" level
		FloatRect textRect = easyText.getLocalBounds();

		easyText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
		easyText.setPosition(buttonX + (buttonWidth / 2.0f), buttonY + (buttonHeight / 2.0f)); // Center the text in the button
		window.draw(easyText);
		// Add similar code for other level buttons and texts
	}

    void levelSelection() {

        static Font font;
        if (!font.loadFromFile("C:/Users/sofma/Downloads/Montserrat-Bold.ttf")) {
            cerr << "Failed to load font!" << endl;
        }

        float centerX = window.getSize().x / 2.0f;
        float centerY = window.getSize().y / 2.0f;

        window.clear();

        Text title("Minesweeper", font, 40);
        Text levelText("Select a level:", font, 28);
        title.setPosition(centerX - title.getLocalBounds().getSize().x / 2, 20);
        levelText.setPosition(centerX - levelText.getLocalBounds().getSize().x / 2, 70);
        window.draw(title);
        window.draw(levelText);

        createButtons(centerX, centerY - 30, font, "Easy");
        createButtons(centerX, centerY + 30, font, "Intermediate");
        createButtons(centerX, centerY + 90, font, "Expert");

        window.display();
        handleLevelSelection();
    }

    void handleLevelSelection() {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) { // If the window is closed
                window.close(); // Close the window
            }
            else if (event.type == Event::MouseButtonPressed) {
                // Get the position of the mouse
                int x = event.mouseButton.x;
                int y = event.mouseButton.y;

                // Get the center and size of the buttons
                float centerX = window.getSize().x / 2.0f;
                float centerY = window.getSize().y / 2.0f;
                float buttonWidth = 100.0f;
                float buttonHeight = 30.0f;

                // Check if the mouse position is within the bounds of the Easy button
                if (x >= centerX - (buttonWidth / 2.0f) && x <= centerX + (buttonWidth / 2.0f) &&
                    y >= centerY - (buttonHeight / 2.0f) - 30 && y <= centerY + (buttonHeight / 2.0f) - 30) {
                    game.getBoard() = Board(9, 9, 10); // Generate the board for "Easy" level
                    return; // Exit the function after generating the board
                }
            };
        }
    }


public:
    Renderer(Minesweeper& m) : game(m), board(m.getBoard()) {
        window.create(VideoMode(board.getColumns() * CELL_SIZE, 
            board.getRows() * CELL_SIZE), "Minesweeper");
        window.setFramerateLimit(60);
    }

    void run() {
        while (window.isOpen()) {
            // levelSelection();
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
