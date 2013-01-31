from itertools import groupby
import naive

def find_groups(data):
    result = []
    for _key, group in groupby(enumerate(data), lambda (i, x): i - x):
        result.append([x[1] for x in group])
    return result

def empty_neighbors(width, height, grid, index):
    result = set()
    x = index % width
    y = index / width
    for dy in xrange(-1, 2):
        for dx in xrange(-1, 2):
            nx = x + dx
            ny = y + dy
            if nx == x and ny == y:
                continue
            if nx < 0 or nx >= width:
                continue
            if ny < 0 or ny >= height:
                continue
            new_index = ny * width + nx
            if grid[new_index]:
                continue
            result.add(new_index)
    return result

def _solve3(width, height, grid, state, lookup):
    if not state:
        yield list(grid)
        return
    number = state.pop()
    a, b = number - 1, number + 1
    valid = empty_neighbors(width, height, grid, lookup[a])
    if b in lookup:
        valid &= empty_neighbors(width, height, grid, lookup[b])
    for index in sorted(valid):
        grid[index] = number
        for result in _solve3(width, height, grid, state, lookup):
            yield result
        grid[index] = 0
    state.append(number)

def _solve2(width, height, grid, states):
    if not states:
        yield list(grid)
        return
    state = states.pop()
    lookup = dict((x, i) for i, x in enumerate(grid) if x)
    for new_grid in _solve3(width, height, grid, state, lookup):
        for result in _solve2(width, height, new_grid, states):
            yield result
    states.append(state)

def _solve1(width, height, grid):
    grid = list(grid)
    numbers = set(range(1, width * height + 1))
    visible = set(x for x in grid if x)
    hidden = numbers - visible
    groups = find_groups(sorted(hidden))
    states = []
    while groups:
        state = sorted(group.pop(0) for group in groups)
        groups = filter(None, groups)
        states.append(state)
    states.reverse()
    for result in _solve2(width, height, grid, states):
        yield result

def solve(width, height, grid):
    grid = naive.solve(width, height, grid)
    if grid.count(0) == 0:
        return grid
    generator = _solve1(width, height, grid)
    try:
        result = generator.next()
    except StopIteration:
        raise Exception('No solutions exist.')
    try:
        generator.next()
    except StopIteration:
        return result
    raise Exception('Multiple solutions exist.')

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
    #grid = [36, 0, 0, 0, 14, 0, 4, 3, 34, 10, 0, 0, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 18, 0, 30, 0, 0, 7, 20, 0, 0, 29, 26, 0, 23, 22]
    grid = [31,0,0,0,11,9,0,0,0,0,0,8,27,0,33,0,0,0,0,25,36,17,0,0,0,23,0,19,4,0,0,0,0,0,0,1]
    grid = solve(6, 6, grid)
    display(6, 6, grid)

if __name__ == '__main__':
    main()
