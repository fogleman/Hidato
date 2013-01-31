import anneal
import random

def compute_neighbors(width, height):
    neighbors = {}
    for y in xrange(height):
        for x in xrange(width):
            i = y * width + x
            neighbors[i] = []
            for dy in xrange(-1, 2):
                for dx in xrange(-1, 2):
                    nx, ny = x + dx, y + dy
                    if nx == x and ny == y:
                        continue
                    if nx < 0 or nx >= width:
                        continue
                    if ny < 0 or ny >= height:
                        continue
                    j = ny * width + nx
                    neighbors[i].append(j)
    return neighbors

def random_neighbor(width, height, index, cache={}):
    key = (width, height)
    if key not in cache:
        cache[key] = compute_neighbors(width, height)
    neighbors = cache[key]
    return random.choice(neighbors[index])

class Model(object):
    def __init__(self, width, height, _reset=True):
        self.width = width
        self.height = height
        self.size = width * height
        if _reset:
            self.reset()
    def reset(self):
        self.end = random.randint(0, self.size - 1)
        self.next = [-1] * self.size
        for i in xrange(self.size):
            if i == self.end:
                continue
            self.next[i] = random_neighbor(self.width, self.height, i)
    def energy(self):
        return self.get_grid().count(-1)
    def do_move(self):
        while True:
            i = random.randint(0, self.size - 1)
            if i == self.end:
                continue
            result = (i, self.next[i])
            self.next[i] = random_neighbor(self.width, self.height, i)
            return result
    def undo_move(self, (index, value)):
        self.next[index] = value
    def get_grid(self):
        result = [-1] * self.size
        lookup = dict((self.next[i], i) for i in xrange(self.size))
        index = self.end
        number = self.size
        for _ in xrange(self.size):
            result[index] = number
            number -= 1
            if index not in lookup:
                break
            index = lookup[index]
        return result

def generate_puzzle(width, height):
    def energy(state):
        return state.energy()
    def do_move(state):
        return state.do_move()
    def undo_move(state, undo_data):
        state.undo_move(undo_data)
    def make_copy(state):
        result = Model(state.width, state.height, False)
        result.end = state.end
        result.next = list(state.next)
        return result
    state = Model(width, height)
    annealer = anneal.Annealer(energy, do_move, undo_move, make_copy)
    state, e = annealer.anneal(state, width * height, 0.1, 100000)
    if e:
        raise Exception('Failed to generate a valid puzzle.')
    return state.get_grid()

def display(width, height, grid):
    for y in xrange(height):
        print ''.join(['+'] + ['--+' for _ in xrange(width)])
        row = ['|']
        for x in xrange(width):
            i = y * width + x
            row.append('%2d|' % grid[i])
        print ''.join(row)
    print ''.join(['+'] + ['--+' for _ in xrange(width)])

def main():
    width = height = 8
    grid = generate_puzzle(width, height)
    display(width, height, grid)

if __name__ == '__main__':
    main()
