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
#include "gtimer.h"
#include "simpio.h" // for getLine
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
  if (!file) {
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

static std::string getFileNameFromUser() {
  std::string filename;
  cout << "Enter data file name for a grid ([enter] for random-generated "
          "grid): ";
  getline(cin, filename);
  return filename;
}

static Grid<int> newGridFromUser() {
  Grid<int> grid;
  const std::string filename = getFileNameFromUser();
  if (filename.empty()) {
    grid = generateRandomGrid();
  } else {
    grid = readGridFromFile(filename);
  }
  return grid;
}

static void drawGrid(LifeDisplay &disp, const Grid<int> &grid) {
  for (int row = 0; row < grid.numRows(); ++row) {
    for (int col = 0; col < grid.numCols(); ++col) {
      disp.drawCellAt(row, col, grid[row][col]);
    }
  }
  disp.printBoard();
  // Clear and show the grid on the windows
  disp.repaint();
}

static int countNeighborCell(const Grid<int> &grid, int row, int col) {
  int count = 0;
  for (int drow = -1; drow <= 1; ++drow) {
    for (int dcol = -1; dcol <= 1; ++dcol) {
      if (drow == 0 && dcol == 0)
        continue;
      int y = row + drow;
      int x = col + dcol;
      if (grid.inBounds(y, x) && grid[y][x] > 0)
        count++;
    }
  }
  return count;
}

static Grid<int> cloneGrid(const Grid<int> &grid) {
  Grid<int> newGrid(grid.numRows(), grid.numCols());
  for (int row = 0; row < grid.numRows(); ++row) {
    for (int col = 0; col < grid.numCols(); ++col) {
      newGrid[row][col] = grid[row][col];
    }
  }
  return newGrid;
}

static Grid<int> generateNextGenerationGrid(Grid<int> &grid) {
  auto newGrid = cloneGrid(grid);
  for (int row = 0; row < grid.numRows(); ++row) {
    for (int col = 0; col < grid.numCols(); ++col) {
      int numNeighborCell = countNeighborCell(grid, row, col);
      if (numNeighborCell <= 1 || numNeighborCell > 3) {
        // kill a cell if it is lonely or overcrowded
        newGrid[row][col] = 0;
      } else if (numNeighborCell == 2) {
        // the cell remains
        if (grid[row][col] > 0 && grid[row][col] < kMaxAge) {
          newGrid[row][col] += 1;
        }
      } else {
        // bear a new cell
        if (grid[row][col] < kMaxAge)
          newGrid[row][col] += 1;
      }
    }
  }
  return newGrid;
}

static bool isStableGrid(Grid<int> &currGrid, Grid<int> &newGrid) {
  for (auto &cell : newGrid) {
    if (cell > 0 && cell < kMaxAge) {
      return false;
    }
  }
  return currGrid == newGrid;
}

/**
 * Function: advanceGrid
 * -----------------
 * Advance the grid its next generation. Return false if the generation is
 * stable after advancing, true otherwise.
 */
static bool advanceGrid(LifeDisplay &disp, Grid<int> &grid) {
  auto newGrid = generateNextGenerationGrid(grid);
  bool canAdvance = !isStableGrid(grid, newGrid);
  grid = newGrid;
  drawGrid(disp, grid);
  return canAdvance;
}

static void clearScreen(LifeDisplay &disp, Grid<int> &grid) {
  for (auto &cell : grid) {
    cell = 0;
  }
  drawGrid(disp, grid);
}

/**
 * Function: runAnimation
 * -----------------
 * An event loop that constantly polls for 2 events: a mouse click on the
 * windows and timer. After the timer elapse, advancing the grid to the next
 * generation.
 */
static void runAnimation(LifeDisplay &disp, Grid<int> &grid, int ms) {
  GTimer timer(ms);
  timer.start();
  while (true) {
    GEvent ev = waitForEvent(TIMER_EVENT + MOUSE_EVENT);
    if (ev.getEventClass() == TIMER_EVENT) {
      if (!advanceGrid(disp, grid))
        break;
    } else if (ev.getEventType() == MOUSE_PRESSED) {
      break;
    }
  }
  timer.stop();
}

static void runManualAnimation(LifeDisplay &disp, Grid<int> &grid) {
  string line;
  while (true) {
    cout << "Press enter to advance the grid, type quit to stop the "
            "simulation: ";
    getline(cin, line);
    bool isEnter = line.empty();
    bool cond = isEnter && !advanceGrid(disp, grid);
    if (cond || line == "quit") {
      break;
    } else if (!isEnter) {
      cout << "Command not support, quitting" << endl;
      exit(0);
    }
  }
}

static void initializeGridAndDisplay(LifeDisplay &disp, Grid<int> &grid) {
  grid = newGridFromUser();
  cout << "Grid's width is " << grid.numRows() << endl;
  cout << "Grid's height is " << grid.numCols() << endl;
  disp.setDimensions(grid.numRows(), grid.numCols());
  //  Write the grid out of the console and draw the grid
  drawGrid(disp, grid);
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
  Grid<int> currGrid;
  initializeGridAndDisplay(display, currGrid);

  // The loop of the simulation
  string line;
  while (true) {
    cout << "Enter manual for manual mode otherwise the simulation is run "
            "automatically: ";
    getline(cin, line);
    if (line == "manual") {
      runManualAnimation(display, currGrid);
    } else {
      int speed = 0;
      cout << "Enter the simulation speed: " << endl;
      cout << "1. slow" << endl;
      cout << "2. medium" << endl;
      cout << "3. fast" << endl;
      cout << "Pick either 1, 2, or 3 to choose the simulation speed: ";
      getline(cin, line);
      if (line == "1") {
        speed = 1000;
      } else if (line == "2") {
        speed = 500;
      } else if (line == "3") {
        speed = 100;
      } else {
        cout << "The option is not supported, quitting" << endl;
        exit(1);
      }
      runAnimation(display, currGrid, speed);
    }

    clearScreen(display, currGrid);
    cout << "Press enter to start a new simulation, type quit to stop the "
            "simulation: ";
    getline(cin, line);
    if (line.empty()) {
      initializeGridAndDisplay(display, currGrid);
      continue;
    } else if (line == "quit") {
      break;
    } else {
      cout << "Command not support, quitting" << endl;
      exit(0);
    }
  }
  return 0;
}
