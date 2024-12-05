#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SIZE 9
#define MAX_HISTORY 100  // Maximum number of moves that can be undone

// Structure to hold move information (row, column, previous value)
typedef struct {
    int row;
    int col;
    int prevValue;
} MoveHistory;

MoveHistory history[MAX_HISTORY];
int historyIndex = 0;  // Keeps track of the current history position

void displayGrid(int grid[SIZE][SIZE], int fixedCells[SIZE][SIZE]);

int isValidMove(int grid[SIZE][SIZE], int row, int col, int num, int showConflicts);
int solveSudoku(int grid[SIZE][SIZE]);
void startTimer();
void endTimer();
void resetGrid(int grid[SIZE][SIZE], int source[SIZE][SIZE]);
void displayElapsedTime();

int easyGrid[SIZE][SIZE] = {
    {5, 3, 0, 0, 7, 0, 0, 0, 0},
    {6, 0, 0, 1, 9, 5, 0, 0, 0},
    {0, 9, 8, 0, 0, 0, 0, 6, 0},
    {8, 0, 0, 0, 6, 0, 0, 0, 3},
    {4, 0, 0, 8, 0, 3, 0, 0, 1},
    {7, 0, 0, 0, 2, 0, 0, 0, 6},
    {0, 6, 0, 0, 0, 0, 2, 8, 0},
    {0, 0, 0, 4, 1, 9, 0, 0, 5},
    {0, 0, 0, 0, 8, 0, 0, 7, 9}
};

int mediumGrid[SIZE][SIZE] = {
    {0, 0, 0, 6, 0, 0, 4, 0, 0},
    {7, 0, 0, 0, 0, 3, 6, 0, 0},
    {0, 0, 0, 0, 9, 1, 0, 8, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 5, 0, 1, 8, 0, 0, 0, 3},
    {0, 0, 0, 3, 0, 6, 0, 4, 5},
    {0, 4, 0, 2, 0, 0, 0, 6, 0},
    {9, 0, 3, 0, 0, 0, 0, 0, 0},
    {0, 2, 0, 0, 0, 0, 1, 0, 0}
};

int hardGrid[SIZE][SIZE] = {
    {8, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 3, 6, 0, 0, 0, 0, 0},
    {0, 7, 0, 0, 9, 0, 2, 0, 0},
    {0, 5, 0, 0, 0, 7, 0, 0, 0},
    {0, 0, 0, 0, 4, 5, 7, 0, 0},
    {0, 0, 0, 1, 0, 0, 0, 3, 0},
    {0, 0, 1, 0, 0, 0, 0, 6, 8},
    {0, 0, 8, 5, 0, 0, 0, 1, 0},
    {0, 9, 0, 0, 0, 0, 4, 0, 0}
};

time_t startTime, endTime;


// Function to undo the last move
void undoLastMove(int grid[SIZE][SIZE], int fixedCells[SIZE][SIZE]) {
    if (historyIndex > 0) {
        historyIndex--;  // Move the index back to the previous move

        // Get the last move from the history
        int row = history[historyIndex].row;
        int col = history[historyIndex].col;
        int prevValue = history[historyIndex].prevValue;

        // Restore the previous value of the cell
        grid[row][col] = prevValue;
        fixedCells[row][col] = (prevValue != 0) ? 1 : 0;  // If value was 0, it’s not fixed

        printf("Undo successful: Reverted (%d, %d) to %d\n", row + 1, col + 1, prevValue);
    } else {
        printf("No move to undo.\n");
    }
}


// Display the Sudoku grid
// Enhanced function to display the Sudoku grid with row and column numbers

// Display the Sudoku grid
void displayGrid(int grid[SIZE][SIZE], int fixedCells[SIZE][SIZE]) {
    printf("\n    ");
    for (int i = 0; i < SIZE; i++) {
        printf(" %d ", i + 1); // Column numbers
    }
    printf("\n   -------------------------\n");

    for (int i = 0; i < SIZE; i++) {
        printf("%d | ", i + 1); // Row numbers
        for (int j = 0; j < SIZE; j++) {
            if (grid[i][j] == 0) {
                printf(". "); // Empty cells
            } else if (fixedCells[i][j]) {
                // Fixed cells in blue
                printf("\033[1;34m%d\033[0m ", grid[i][j]);
            } else {
                // User-entered cells in default (white) color
                printf("%d ", grid[i][j]);
            }
            if (j % 3 == 2 && j != SIZE - 1) {
                printf("| "); // Vertical grid separators
            }
        }
        printf("|\n");
        if (i % 3 == 2 && i != SIZE - 1) {
            printf("   -------------------------\n"); // Horizontal grid separators
        }
    }
    printf("   -------------------------\n");
}



// Check if placing a number is valid and highlight conflicts
int isValidMove(int grid[SIZE][SIZE], int row, int col, int num, int showConflicts) {
    int valid = 1;

    // Check row and column
    for (int i = 0; i < SIZE; i++) {
        if (grid[row][i] == num || grid[i][col] == num) {
            valid = 0;
            if (showConflicts)
                printf("Conflict: Number %d already exists in row or column.\n", num);
        }
    }

    // Check 3x3 subgrid
    int startRow = row - row % 3, startCol = col - col % 3;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (grid[startRow + i][startCol + j] == num) {
                valid = 0;
                if (showConflicts)
                    printf("Conflict: Number %d already exists in block.\n", num);
            }
        }
    }

    return valid;
}

// Solve the Sudoku puzzle using backtracking
int solveSudoku(int grid[SIZE][SIZE]) {
    for (int row = 0; row < SIZE; row++) {
        for (int col = 0; col < SIZE; col++) {
            if (grid[row][col] == 0) {
                for (int num = 1; num <= 9; num++) {
                    if (isValidMove(grid, row, col, num, 0)) {
                        grid[row][col] = num;
                        if (solveSudoku(grid))
                            return 1;
                        grid[row][col] = 0;
                    }
                }
                return 0;
            }
        }
    }
    return 1;
}

// Timer functions
void startTimer() {
    time(&startTime);
}

void endTimer() {
    time(&endTime);
}

void displayElapsedTime() {
    double elapsed = difftime(endTime, startTime);
    printf("Time elapsed: %.2f seconds.\n", elapsed);
}

// Reset grid to a source grid
void resetGrid(int grid[SIZE][SIZE], int source[SIZE][SIZE]) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            grid[i][j] = source[i][j];
        }
    }
}

// Main function
int main() {
    int grid[SIZE][SIZE];
    int choice, row, col, num, level;

    printf("Welcome to Sudoku!\n");
    printf("Choose difficulty level:\n1. Easy\n2. Medium\n3. Hard\n");
    scanf("%d", &level);

    // Reset history at the start of a new game
    historyIndex = 0;
    memset(history, 0, sizeof(history));  // Clear the history array

    int fixedCells[SIZE][SIZE] = {0};

    // Initialize fixedCells based on the starting grid
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fixedCells[i][j] = (grid[i][j] != 0) ? 1 : 0;
        }
    }

    switch (level) {
        case 1:
            resetGrid(grid, easyGrid);
            break;
        case 2:
            resetGrid(grid, mediumGrid);
            break;
        case 3:
            resetGrid(grid, hardGrid);
            break;
        default:
            printf("Invalid choice! Defaulting to Easy.\n");
            resetGrid(grid, easyGrid);
    }

    startTimer();

    while (1) {
        printf("\n1. Display Grid\n2. Make a Move\n3. Save Game\n4. Load Game\n5. Solve Puzzle\n6. Quit\n7. Undo Last Move\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                displayGrid(grid, fixedCells);
                break;
            case 2:
                // Inside the 'Make a Move' option:
                while (1) {
                    displayGrid(grid, fixedCells);

                    printf("Enter row (1-9), column (1-9), and number (1-9) to make a move: ");
                    scanf("%d %d %d", &row, &col, &num);

                    row -= 1;
                    col -= 1;

                    if (row < 0 || row >= SIZE || col < 0 || col >= SIZE || num < 1 || num > 9) {
                        printf("Invalid input! Please enter row and column between 1 and 9, and number between 1 and 9.\n");
                    } else if (grid[row][col] != 0) {
                        printf("This cell is already filled. Choose another cell.\n");
                    } else if (isValidMove(grid, row, col, num, 1)) {
                        // Save the move to history only if the move is valid
                        if (historyIndex < MAX_HISTORY) {
                            history[historyIndex].row = row;
                            history[historyIndex].col = col;
                            history[historyIndex].prevValue = grid[row][col];  // Save the previous value of the cell
                            historyIndex++;  // Increment historyIndex to point to the next available spot
                        }

                        grid[row][col] = num;  // Make the move
                        printf("Move accepted.\n");
                    } else {
                        printf("Invalid move. Try again.\n");
                    }

                    // Ask if the user wants to continue making moves
                    char continueMove;
                    printf("Do you want to make another move? (y/n): ");
                    scanf(" %c", &continueMove);

                    if (continueMove == 'n' || continueMove == 'N') {
                        printf("Exiting to the main menu.\n");
                        break;
                    }
                }
                break;

            case 3: {
                char saveFileName[100];
                printf("Enter the filename to save (e.g., game1.txt): ");
                scanf("%s", saveFileName);

                FILE *file = fopen(saveFileName, "w");
                if (!file) {
                    printf("Error: Unable to save the game.\n");
                    break;
                }

                for (int i = 0; i < SIZE; i++) {
                    for (int j = 0; j < SIZE; j++) {
                        fprintf(file, "%d ", grid[i][j]);
                    }
                    fprintf(file, "\n");
                }
                fclose(file);
                printf("Game saved successfully to %s!\n", saveFileName);
                break;
            }

            case 4: {
                char loadFileName[100];
                printf("Enter the filename to load (e.g., game1.txt): ");
                scanf("%s", loadFileName);

                FILE *file = fopen(loadFileName, "r");
                if (!file) {
                    printf("Error: Unable to load the game.\n");
                    break;
                }

                for (int i = 0; i < SIZE; i++) {
                    for (int j = 0; j < SIZE; j++) {
                        if (fscanf(file, "%d", &grid[i][j]) != 1) {
                            printf("Error: Invalid file format.\n");
                            fclose(file);
                            break;
                        }
                    }
                }
                fclose(file);
                printf("Game loaded successfully from %s!\n", loadFileName);
                break;
            }

            case 5:
                if (solveSudoku(grid)) {
                    displayGrid(grid, fixedCells);
                    printf("Sudoku solved!\n");
                } else {
                    printf("No solution exists.\n");
                }
                break;

            case 6:
                endTimer();
                displayElapsedTime();
                printf("Thank you for playing Sudoku!\n");
                exit(0);

            case 7:  // Undo the last move
                undoLastMove(grid, fixedCells);
                break;

            default:
                printf("Invalid choice. Try again.\n");
        }
    }

    return 0;
}
