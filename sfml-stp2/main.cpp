#include <vector>
#include <string>
#include <iostream>

using namespace std;

enum class Level
{
    easy,
    intermediate,
    expert
};

class Cell {
    int row, column, neighborBombs = 0;
    bool bomb, flagged, opened = false;

public:

    Cell() : row(0), column(0) {} // Initialize with default values

    Cell(int r, int c) : row(r), column(c) {};

    void open() {
        this->opened = true;
    }

    void toggleFlag() {
        this->flagged = !flagged;
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
};


class Board {
    int rows, columns, bombs, closedCells, remainingBombs = 0;
    Cell** cells;
public:
    Board(int r, int c, int bombs) :
        rows(r), columns(c), bombs(bombs), closedCells(r* c), remainingBombs(bombs) {
        cells = new Cell * [rows];
        for (int i = 0; i < rows; ++i) {
            cells[i] = new Cell[columns];
            for (int j = 0; j < columns; ++j) {
                // Initialize each cell with row and column values
                cells[i][j] = Cell(i, j);
            }
        }
    }

    ~Board() {
        for (int i = 0; i < rows; ++i) {
            delete[] cells[i];
        }
        delete[] cells;
    }

    Cell& getCell(int row, int column) const {
        return this->cells[row][column];
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


int main() {
    Board board = Board(10, 6, 5);

    // Print details of each cell
    for (int i = 0; i < board.getRows(); ++i) {
        for (int j = 0; j < board.getColumns(); ++j) {
            Cell& currentCell = board.getCell(i, j);
            cout << "Cell[" << i << "][" << j << "]: Row - " << currentCell.getRow() << ", Column - " << currentCell.getColumn() << endl;
        }
    }

    // Print out some details about the board
    cout << "Rows: " << board.getRows() << endl;
    cout << "Columns: " << board.getColumns() << endl;
    cout << "Bombs: " << board.getRemainedBombs() << endl;

    return 0;
}
