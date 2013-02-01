#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

typedef struct {
    int width;
    int height;
    int size;
    int start;
    int *next;
} Model;

typedef struct {
    int width;
    int height;
    int size;
    int *data;
} Grid;

typedef struct {
    int start;
    int end;
} Group;

int rand_int(int n) {
    int result;
    while (n <= (result = rand() / (RAND_MAX / n)));
    return result;
}

double rand_double() {
    return (double)rand() / (double)RAND_MAX;
}

int rand_neighbor(int width, int height, int index) {
    int x = index % width;
    int y = index / width;
    while (1) {
        int dx = rand_int(3) - 1;
        int dy = rand_int(3) - 1;
        if (dx == 0 && dy == 0) {
            continue;
        }
        int nx = x + dx;
        int ny = y + dy;
        if (nx < 0 || nx >= width) {
            continue;
        }
        if (ny < 0 || ny >= height) {
            continue;
        }
        return ny * width + nx;
    }
}

void display(int width, int height, int *grid) {
    int size = width * height;
    for (int y = 0; y < height; y++) {
        printf("+");
        for (int x = 0; x < width; x++) {
            printf("---+");
        }
        printf("\n|");
        for (int x = 0; x < width; x++) {
            int i = y * width + x;
            if (grid[i]) {
                printf("%3d|", grid[i]);
            }
            else {
                printf("   |");
            }
        }
        printf("\n");
    }
    printf("+");
    for (int x = 0; x < width; x++) {
        printf("---+");
    }
    printf("\n\n");
}

void gen_init(Model *model, int width, int height) {
    int size = width * height;
    model->width = width;
    model->height = height;
    model->size = size;
    model->start = rand_int(size);
    model->next = calloc(size, sizeof(int));
}

void gen_randomize(Model *model) {
    for (int i = 0; i < model->size; i++) {
        model->next[i] = rand_neighbor(model->width, model->height, i);
    }
}

void gen_uninit(Model *model) {
    free(model->next);
}

int gen_extract(Model *model, int *grid) {
    for (int i = 0; i < model->size; i++) {
        grid[i] = 0;
    }
    int index = model->start;
    int number = 1;
    int result = 0;
    while (grid[index] == 0) {
        result++;
        grid[index] = number++;
        index = model->next[index];
    }
    return result;
}

void gen_display(Model *model) {
    int grid[model->size];
    gen_extract(model, grid);
    display(model->width, model->height, grid);
}

int gen_energy(Model *model) {
    int grid[model->size];
    int count = gen_extract(model, grid);
    return model->size - count;
}

int gen_do_move(Model *model) {
    int index = rand_int(model->size);
    int before = model->next[index];
    int after;
    do {
        after = rand_neighbor(model->width, model->height, index);
    } while (after == before);
    model->next[index] = after;
    return (index << 16) | before;
}

void gen_undo_move(Model *model, int undo_data) {
    int index = (undo_data >> 16) & 0xffff;
    int value = undo_data & 0xffff;
    model->next[index] = value;
}

void gen_copy(Model *dst, Model *src) {
    dst->width = src->width;
    dst->height = src->height;
    dst->size = src->size;
    dst->start = src->start;
    for (int i = 0; i < src->size; i++) {
        dst->next[i] = src->next[i];
    }
}

int gen_anneal(Model *model, double max_temp, double min_temp, int steps) {
    Model _best;
    Model *best = &_best;
    gen_init(best, model->width, model->height);
    gen_copy(best, model);
    double factor = -log(max_temp / min_temp);
    int energy = gen_energy(model);
    int previous_energy = energy;
    int best_energy = energy;
    for (int step = 0; step < steps; step++) {
        double temp = max_temp * exp(factor * step / steps);
        int undo_data = gen_do_move(model);
        energy = gen_energy(model);
        double change = energy - previous_energy;
        if (change > 0 && exp(-change / temp) < rand_double()) {
            gen_undo_move(model, undo_data);
        }
        else {
            previous_energy = energy;
            if (energy < best_energy) {
                best_energy = energy;
                gen_copy(best, model);
                if (energy <= 0) {
                    break;
                }
            }
        }
    }
    gen_copy(model, best);
    gen_uninit(best);
    return best_energy;
}

void gen(int width, int height, int *output) {
    Model _model;
    Model *model = &_model;
    while (1) {
        gen_init(model, width, height);
        gen_randomize(model);
        int energy = gen_anneal(model, 10, 0.1, 100000);
        if (energy == 0) {
            gen_extract(model, output);
        }
        gen_uninit(model);
        if (energy == 0) {
            break;
        }
    }
}

void gen_test() {
    int width = 16;
    int height = width;
    int data[width * height];
    gen(width, height, data);
    display(width, height, data);
}

void solver_display(Grid *grid) {
    display(grid->width, grid->height, grid->data);
}

int solver_find_groups(Grid *grid, Group *groups) {
    int result = 0;
    int visible[grid->size + 1]; // 1-based
    for (int i = 0; i < grid->size + 1; i++) {
        visible[i] = 0;
    }
    for (int i = 0; i < grid->size; i++) {
        if (grid->data[i]) {
            visible[grid->data[i]] = 1;
        }
    }
    int start = 0;
    int end = 0;
    for (int i = 1; i < grid->size + 1; i++) {
        if (visible[i]) {
            if (end) {
                Group *group = groups + result++;
                group->start = start;
                group->end = end;
                end = 0;
            }
            start = i + 1;
        }
        else {
            end = i;
        }
    }
    if (end) {
        Group *group = groups + result++;
        group->start = start;
        group->end = end;
        end = 0;
    }
    return result;
}

void solver_lookup(Grid *grid, int *lookup) {
    for (int i = 0; i < grid->size + 1; i++) {
        lookup[i] = -1;
    }
    for (int i = 0; i < grid->size; i++) {
        lookup[grid->data[i]] = i;
    }
}

void solver_copy(Grid *dst, Grid *src) {
    dst->width = src->width;
    dst->height = src->height;
    dst->size = src->size;
    for (int i = 0; i < src->size; i++) {
        dst->data[i] = src->data[i];
    }
}

int solver_helper(
    Grid *grid, Grid *output, int *lookup, Group *groups,
    int n_groups, int index)
{
    if (index == n_groups) {
        int done = 1;
        for (int i = 0; i < n_groups; i++) {
            Group *group = groups + i;
            if (group->start <= group->end) {
                done = 0;
                break;
            }
        }
        if (done) {
            int count = grid->size;
            for (int i = 0; i < grid->size; i++) {
                if (grid->data[i]) {
                    count--;
                }
            }
            if (count) {
                return 0;
            }
            else {
                solver_copy(output, grid);
                return 1;
            }
        }
        else {
            int new_lookup[grid->size + 1];
            solver_lookup(grid, new_lookup);
            return solver_helper(grid, output, new_lookup, groups, n_groups, 0);
        }
    }
    Group *group = groups + index;
    if (group->start > group->end) {
        return solver_helper(grid, output, lookup, groups, n_groups, index + 1);
    }
    else {
        int result = 0;
        int i = lookup[group->start - 1];
        group->start++;
        int x = i % grid->width;
        int y = i / grid->width;
        int k = lookup[group->start];
        int kx = -1;
        int ky = -1;
        if (k >= 0) {
            kx = k % grid->width;
            ky = k / grid->height;
        }
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (dx == 0 && dy == 0) {
                    continue;
                }
                int nx = x + dx;
                int ny = y + dy;
                if (nx < 0 || nx >= grid->width) {
                    continue;
                }
                if (ny < 0 || ny >= grid->height) {
                    continue;
                }
                int j = ny * grid->width + nx;
                if (grid->data[j]) {
                    continue;
                }
                if (k >= 0) {
                    int kdx = abs(nx - kx);
                    int kdy = abs(ny - ky);
                    if (kdx > 1 || kdy > 1) {
                        continue;
                    }
                }
                grid->data[j] = group->start - 1;
                result += solver_helper(
                    grid, output, lookup, groups,n_groups, index + 1);
                grid->data[j] = 0;
                if (result > 1) {
                    break;
                }
            }
        }
        group->start--;
        return result;
    }
}

int solver(Grid *grid) {
    Grid _output;
    Grid *output = &_output;
    output->data = calloc(grid->size, sizeof(int));
    Group groups[grid->size];
    int n_groups = solver_find_groups(grid, groups);
    int result = solver_helper(grid, output, 0, groups, n_groups, n_groups);
    if (result == 1) {
        solver_copy(grid, output);
    }
    free(output->data);
    return result;
}

void solver_test() {
    int data[] = {
        31,0,0,0,11,9,0,0,0,0,0,8,27,0,33,0,0,0,0,
        25,36,17,0,0,0,23,0,19,4,0,0,0,0,0,0,1
    };
    Grid _grid;
    Grid *grid = &_grid;
    grid->width = 6;
    grid->height = 6;
    grid->size = 36;
    grid->data = data;
    solver_display(grid);
    int count = solver(grid);
    printf("%d\n", count);
    solver_display(grid);
}

int vis_energy(Grid *grid, int *visible) {
    int energy = 0;
    int data[grid->size];
    for (int i = 0; i < grid->size; i++) {
        if (visible[i]) {
            data[i] = grid->data[i];
            energy++;
        }
        else {
            data[i] = 0;
        }
    }
    int *temp = grid->data;
    grid->data = data;
    int count = solver(grid);
    grid->data = temp;
    if (count == 1) {
        return energy;
    }
    else {
        return grid->size;
    }
}

int vis_do_move(Grid *grid, int *visible) {
    int index;
    do {
        index = rand_int(grid->size);
    } while (grid->data[index] == 1 || grid->data[index] == grid->size);
    visible[index] = !visible[index];
    return index;
}

void vis_undo_move(Grid *grid, int *visible, int index) {
    visible[index] = !visible[index];
}

void vis_copy(int size, int *dst, int *src) {
    for (int i = 0; i < size; i++) {
        dst[i] = src[i];
    }
}

int vis_anneal(Grid *grid, double max_temp, double min_temp, int steps) {
    int visible[grid->size];
    int best[grid->size];
    for (int i = 0; i < grid->size; i++) {
        visible[i] = 1;
        best[i] = 1;
    }
    double factor = -log(max_temp / min_temp);
    int energy = vis_energy(grid, visible);
    int previous_energy = energy;
    int best_energy = energy;
    for (int step = 0; step < steps; step++) {
        double temp = max_temp * exp(factor * step / steps);
        int undo_data = vis_do_move(grid, visible);
        energy = vis_energy(grid, visible);
        double change = energy - previous_energy;
        if (change > 0 && exp(-change / temp) < rand_double()) {
            vis_undo_move(grid, visible, undo_data);
        }
        else {
            previous_energy = energy;
            if (energy < best_energy) {
                best_energy = energy;
                vis_copy(grid->size, best, visible);
            }
        }
        // printf("%d of %d => %d\n", step + 1, steps, best_energy);
    }
    for (int i = 0; i < grid->size; i++) {
        if (!best[i]) {
            grid->data[i] = 0;
        }
    }
    return best_energy;
}

int vis(int width, int height, int *output) {
    Grid _grid;
    Grid *grid = &_grid;
    grid->width = width;
    grid->height = height;
    grid->size = width * height;
    grid->data = output;
    return vis_anneal(grid, 10, 0.1, 100);
}

void hidato(int width, int height) {
    int output[width * height];
    gen(width, height, output);
    display(width, height, output);
    int energy = vis(width, height, output);
    display(width, height, output);
    printf("%d\n", energy);
}

int main(int argc, char **argv) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
    hidato(8, 8);
    return 0;
}
