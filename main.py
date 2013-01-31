import anneal
import generator
import naive
import random
import solver

class Model(object):
    def __init__(self, width, height, grid, shown=None):
        self.width = width
        self.height = height
        self.size = width * height
        self.grid = grid
        self.shown = shown or ([True] * self.size)
    def energy(self):
        grid = self.get_grid()
        try:
            solver.solve(self.width, self.height, grid)
        except Exception:
            return self.size + 1
        return self.shown.count(True)
    def do_move(self):
        while True:
            index = random.randint(0, self.size - 1)
            if self.grid[index] == 1:
                continue
            self.shown[index] = not self.shown[index]
            return index
    def undo_move(self, index):
        self.shown[index] = not self.shown[index]
    def get_grid(self):
        return [x if self.shown[i] else 0 for i, x in enumerate(self.grid)]

def generate_puzzle(width, height, grid):
    def energy(state):
        return state.energy()
    def do_move(state):
        return state.do_move()
    def undo_move(state, undo_data):
        state.undo_move(undo_data)
    def make_copy(state):
        result = Model(state.width, state.height, state.grid)
        result.shown = list(state.shown)
        return result
    def listener(state, energy):
        print energy
        display(width, height, state.get_grid())
    state = Model(width, height, grid)
    annealer = anneal.Annealer(energy, do_move, undo_move, make_copy)
    annealer.listener = listener
    state, _e = annealer.anneal(state, width * height, 0.1, 200)
    return state.get_grid()

def display(width, height, grid):
    for y in xrange(height):
        print ''.join(['+'] + ['--+' for _ in xrange(width)])
        row = ['|']
        for x in xrange(width):
            i = y * width + x
            row.append('%2d|' % grid[i] if grid[i] else '  |')
        print ''.join(row)
    print ''.join(['+'] + ['--+' for _ in xrange(width)])
    print

def main():
    width = height = 6
    grid = generator.generate_puzzle(width, height)
    display(width, height, grid)
    grid = generate_puzzle(width, height, grid)
    display(width, height, grid)
    grid = naive.solve(width, height, grid)
    display(width, height, grid)
    grid = solver.solve(width, height, grid)
    display(width, height, grid)

if __name__ == '__main__':
    main()
