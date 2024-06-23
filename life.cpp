/**
 * File: life.cpp
 * --------------
 * Implements the Game of Life.
 */

#include <cassert> // assert the condition
#include <fstream>
#include <iostream> // for cout
#include <random>   // for random utilities
using namespace std;

#include "console.h" // required of all files that contain the main function
#include "gevent.h"  // for mouse event detection
#include "simpio.h"  // for getLine
#include "strlib.h"

#include "life-constants.h" // for kMaxAge
#include "life-graphics.h"  // for class LifeDisplay

static constexpr int kLowerBound = 40;
static constexpr int kUpperBound = 60;

enum CellState : int { Empty = 0, Occupied = 1 };

/**
 * Function: welcome
 * -----------------
 * Introduces the user to the Game of Life and its rules.
 */
static void welcome() {
  cout << "Welcome to the game of Life, a simulation of the lifecycle of a "
          "bacteria colony."
       << endl;
  cout << "Cells live and die by the following rules:" << endl << endl;
  cout << "\tA cell with 1 or fewer neighbors dies of loneliness" << endl;
  cout << "\tLocations with 2 neighbors remain stable" << endl;
  cout << "\tLocations with 3 neighbors will spontaneously create life" << endl;
  cout << "\tLocations with 4 or more neighbors die of overcrowding" << endl
       << endl;
  cout << "In the animation, new cells are dark and fade to gray as they age."
       << endl
       << endl;
  getLine("Hit [enter] to continue....   ");
}

static Grid<int> generateRandomGrid() {
  // Initialize the random generator
  std::random_device rd;  // a seed source for the random number engine
  std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<> distrib(kLowerBound, kUpperBound);
  // Generate random height and width
  int width = distrib(gen);
  int height = distrib(gen);

  Grid<int> grid(height, width);
  // Mark each Cell with either empty or occupied
  // random generator to flip either empty or occupied
  std::uniform_int_distribution<> boolGen(0, 1);
  // age generator between 1 and kMaxAge
  std::uniform_int_distribution<> ageGen(1, kMaxAge);
  for (auto &cell : grid) {
    cell = boolGen(gen) == 1 ? ageGen(gen) : CellState::Empty;
  }
  return grid;
}

static Grid<int> readGridFromFile(const std::string &filename) {
  std::ifstream file(filename);
  if (file.fail()) {
    cerr << "Can not open file " << filename << endl;
    exit(1);
  }
  std::string line;
  int height;
  int width;

  while (true) {
    std::getline(file, line);
    if (line[0] == '#')
      continue;
    height = stringToInteger(line);
    std::getline(file, line);
    width = stringToInteger(line);
    break;
  }

  Grid<int> grid(height, width);
  for (int row = 0; row < grid.numRows(); ++row) {
    std::getline(file, line);
    for (int col = 0; col < grid.numCols(); ++col) {
      grid[row][col] =
          line[col] == 'X' ? CellState::Occupied : CellState::Empty;
    }
  }
  return grid;
}

static void drawGrid(LifeDisplay &disp, const Grid<int> &grid) {
  for (int row = 0; row < grid.numRows(); ++row) {
    for (int col = 0; col < grid.numCols(); ++col) {
      disp.drawCellAt(row, col, grid[row][col]);
    }
  }
}

static std::string getFileNameFromUser() {
  std::string filename;
  cout << "Enter data file name for a grid ([enter] for random-generated "
          "grid): ";
  getline(cin, filename);
  return filename;
}

/**
 * Function: main
 * --------------
 * Provides the entry point of the entire program.
 */
int main() {
  LifeDisplay display;
  display.setTitle("Game of Life");
  welcome();
  // Goal is to build a conway game of life
  // Steps:
  // I. Set up interfaces for user:
  Grid<int> currGrid;
  const std::string filename = getFileNameFromUser();
  if (filename.empty()) {
    currGrid = generateRandomGrid();
  } else {
    currGrid = readGridFromFile(filename);
  }
  Grid<int> nextGrid = currGrid;
  cout << "Grid's width is " << currGrid.numRows() << endl;
  cout << "Grid's height is " << currGrid.numCols() << endl;
  display.setDimensions(currGrid.numRows(), currGrid.numCols());
  //  Write the grid out of the console
  drawGrid(display, currGrid);
  display.printBoard();
  // Clear and show the grid on the windows
  display.repaint();
  // II. Implement the logic and cell generation:
  // 1. Impl the generations
  // 2. Impl the rules
  // 3. Implementing User interaction To change the board
  // III. Set up simluation speed:
  // 1. Provide different options, default to be medium for debugging
  // 2. Provide options to stop by the user
  // 3. Provide logic to check stability
  // TODO: while loop to show the board, remove it later on
  while (1) {
  }
  return 0;
}
