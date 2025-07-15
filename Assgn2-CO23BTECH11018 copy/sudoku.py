import numpy as np

def generate_sudoku(n):
    """
    Generate a filled n x n Sudoku grid where each sub-grid size is sqrt(n) x sqrt(n).
    """
    side = int(np.sqrt(n))
    if side ** 2 != n:
        return f"Cannot create a proper Sudoku grid for size {n}. It must be a perfect square."

    def pattern(r, c):
        return (side * (r % side) + r // side + c) % n

    def shuffle(s):
        return np.random.permutation(s)

    r_base = range(side)
    rows = [g * side + r for g in shuffle(r_base) for r in shuffle(r_base)]
    cols = [g * side + c for g in shuffle(r_base) for c in shuffle(r_base)]
    nums = shuffle(range(1, n + 1))

    # Create the grid
    board = [[nums[pattern(r, c)] for c in cols] for r in rows]
    return np.array(board)

# Generate 64x64 Sudoku grid
sudoku_64x64 = generate_sudoku(8100)

# Write the grid to input.txt without brackets
with open("input.txt", "w") as file:
    # Write each row of the grid as space-separated numbers
    for row in sudoku_64x64:
        file.write(' '.join(map(str, row)) + '\n')