#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <cstring>
#include <string>
#include <fstream>
#include <thread>

constexpr int surviving_min_required = 2;
constexpr int surviving_max_allowed = 3;
constexpr int revive_required = 3;
constexpr double default_probability = 0.5;

enum State {
    dead,
    alive,
    undefined
};

struct Cell {
    std::vector<Cell *> neighbour_cells;
    State current_state;
    State upcoming_state;
};

void ComputeUpcomingCellState(Cell *cell) {
    int alive_neighbours_count = 0;

    for (auto neighbour : cell->neighbour_cells) {
        if (neighbour->current_state == alive) {
            ++alive_neighbours_count;
        }
    }

    if (cell->current_state == alive) {
        if (alive_neighbours_count < surviving_min_required ||
            alive_neighbours_count > surviving_max_allowed) {
            cell->upcoming_state = dead;
        }
    } else {
        if (alive_neighbours_count == revive_required) {
            cell->upcoming_state = alive;
        }
    }
    if (cell->upcoming_state == undefined) {
        cell->upcoming_state = cell->current_state;
    }
}

void ComputeUpcomingBoardState(std::vector<std::vector<Cell>> *board) {
    for (auto &row : *board) {
        for (auto &cell : row) {
            ComputeUpcomingCellState(&cell);
        }
    }
}

void StepToUpcomingState(std::vector<std::vector<Cell>> *board) {


    for (auto &row : *board) {
        for (auto &cell : row) {
            cell.current_state = cell.upcoming_state;
            cell.upcoming_state = undefined;
        }
    }
}

void SetNeighbours(std::vector<std::vector<Cell>> *board,
                   size_t i, size_t j, size_t rows, size_t columns) {
    if (i > 0 && j > 0) {
        (*board)[i][j].neighbour_cells.push_back(&(*board)[i - 1][j - 1]);
    }
    if (i > 0) {
        (*board)[i][j].neighbour_cells.push_back(&(*board)[i - 1][j]);
    }
    if (i > 0 && j + 1 < columns) {
        (*board)[i][j].neighbour_cells.push_back(&(*board)[i - 1][j + 1]);
    }
    if (j > 0) {
        (*board)[i][j].neighbour_cells.push_back(&(*board)[i][j - 1]);
    }
    if (j + 1 < columns) {
        (*board)[i][j].neighbour_cells.push_back(&(*board)[i][j + 1]);
    }
    if (i + 1 < rows && j > 0) {
        (*board)[i][j].neighbour_cells.push_back(&(*board)[i + 1][j - 1]);
    }
    if (i + 1 < rows) {
        (*board)[i][j].neighbour_cells.push_back(&(*board)[i + 1][j]);
    }
    if (i + 1 < rows && j + 1 < columns) {
        (*board)[i][j].neighbour_cells.push_back(&(*board)[i + 1][j + 1]);
    }
}

void InitializeBoard(std::vector<std::vector<Cell>> *board,
                     size_t rows, size_t columns) {
    board->resize(rows);
    for (auto &row : *board) {
        row.resize(columns, {{}, undefined, undefined});
    }

    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < columns; ++j) {
            SetNeighbours(board, i, j, rows, columns);
        }
    }
}

void RandomizeBoard(std::vector<std::vector<Cell>> *board, double alive_probability) {
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_real_distribution<double> dis(0, 1.0);

    for (auto &row : *board) {
        for (auto &cell : row) {
            if (dis(generator) < alive_probability) {
                cell.current_state = alive;
            } else {
                cell.current_state = dead;
            }
            cell.upcoming_state = undefined;
        }
    }
}

void Print(const std::vector<std::vector<Cell>> &board) {
    for (const auto &row : board) {
        for (const auto &cell : row) {
            std::cout << cell.current_state;
        }
        std::cout << '\n';
    }
}

int main(int argc, char *argv[]) {

    size_t rows, columns;
    if (argc < 4) {
        std::cout << "Not enough arguments" << '\n';
        return 0;
    }

    if (atol(argv[1]) < 1 || atol(argv[2]) < 1) {
        std::cout << "Invalid arguments provided" << '\n';
        return 0;
    }

    rows = atol(argv[1]);
    columns = atol(argv[2]);
    std::vector<std::vector<Cell>> board = {{}};

    InitializeBoard(&board, rows, columns);

    if (strcmp(argv[3], "-random") == 0) {
        double alive_probability = default_probability;
        if (argc == 5) {
            alive_probability = atof(argv[4]);
            if (alive_probability <= 0 || alive_probability >= 1) {
                std::cout << "Invalid probability" << '\n';
                return 0;
            }
        }
        RandomizeBoard(&board, alive_probability);
    } else if (strcmp(argv[3], "-file") == 0) {
        std::ifstream file(argv[4]);
        std::string str;

        if (!file) {
            std::cout << "Problems occurred with file path given. Can't open such file" << '\n';
            return 0;
        }

        for (size_t i = 0; i < rows; ++i) {
            getline(file, str);
            for (size_t j = 0; j < columns; ++j) {
                board[i][j].current_state = (str[j] == '0') ? dead : alive;
            }
        }

        file.close();
    } else {
        std::cout << "Unknown parameters" << '\n';
        return 0;
    }

    auto start = std::chrono::high_resolution_clock::now();

    while (true) {
        ComputeUpcomingBoardState(&board);

        auto finish = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> interval = finish - start;
        std::this_thread::sleep_for(static_cast<std::chrono::duration<double>>(1.) - interval);
        start = std::chrono::high_resolution_clock::now();

        Print(board);
        StepToUpcomingState(&board);
        std::cout << '\n';
    }
}