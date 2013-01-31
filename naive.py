from itertools import groupby

def find_ranges(data):
    result = []
    for _key, group in groupby(enumerate(data), lambda (i, x): i - x):
        result.append([x[1] for x in group])
    return result

def _find_paths(width, height, grid, index, target, length, path, seen):
    seen.add(index)
    path.append(index)
    if len(path) == length:
        if index == target:
            yield path[1:-1]
    else:
        x, y = index % width, index / width
        for dy in xrange(-1, 2):
            for dx in xrange(-1, 2):
                if dx == 0 and dy == 0:
                    continue
                nx, ny = x + dx, y + dy
                if nx < 0 or nx >= width:
                    continue
                if ny < 0 or ny >= height:
                    continue
                new_index = ny * width + nx
                if new_index != target and grid[new_index] != 0:
                    continue
                if new_index in seen:
                    continue
                for result in _find_paths(width, height, grid, new_index, target, length, path, seen):
                    yield result
    path.pop()
    seen.remove(index)

def search(width, height, grid, index, target):
    n1, n2 = grid[index], grid[target]
    length = n2 - n1 + 1
    all_choices = [set() for _ in xrange(length - 2)]
    for result in _find_paths(width, height, grid, index, target, length, [], set()):
        for index, value in enumerate(result):
            all_choices[index].add(value)
        if all(len(x) > 1 for x in all_choices):
            return False
    for index, number in enumerate(xrange(n1 + 1, n2)):
        choices = all_choices[index]
        if len(choices) == 1:
            choice = choices.pop()
            grid[choice] = number
    return True

def solve(width, height, grid):
    grid = list(grid)
    numbers = set(range(1, width * height + 1))
    while True:
        lookup = dict((x, i) for i, x in enumerate(grid) if x != 0)
        visible = set(lookup)
        hidden = numbers - visible
        groups = find_ranges(sorted(hidden))
        groups.sort(key=len)
        for group in groups:
            n1, n2 = group[0] - 1, group[-1] + 1
            i1, i2 = lookup[n1], lookup[n2]
            if search(width, height, grid, i1, i2):
                break
        else:
            break
    return grid

def display(width, height, numbers):
    for y in xrange(height):
        print ''.join(['+'] + ['--+' for _ in xrange(width)])
        row = ['|']
        for x in xrange(width):
            i = y * width + x
            row.append('%2d|' % numbers[i])
        print ''.join(row)
    print ''.join(['+'] + ['--+' for _ in xrange(width)])

def main():
    grid = [36, 0, 0, 0, 14, 0, 4, 3, 34, 10, 0, 0, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 18, 0, 30, 0, 0, 7, 20, 0, 0, 29, 26, 0, 23, 22]
    grid = solve(6, 6, grid)
    display(6, 6, grid)

if __name__ == '__main__':
    main()
