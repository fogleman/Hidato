#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    int width;
    int height;
    int end;
    int *next;
} Model;

int randint(int n) {
    int result;
    while (n <= (result = rand() / (RAND_MAX / n)));
    return result;
}

double randdouble() {
    return (double)rand() / (double)RAND_MAX;
}

int random_neighbor(Model *model, int index) {
    int x = index % model->width;
    int y = index / model->width;
    while (1) {
        int dx = randint(3) - 1;
        int dy = randint(3) - 1;
        if (dx == 0 && dy == 0) {
            continue;
        }
        int nx = x + dx;
        int ny = y + dy;
        if (nx < 0 || nx >= model->width) {
            continue;
        }
        if (ny < 0 || ny >= model->height) {
            continue;
        }
        return ny * model->width + nx;
    }
}

int extract_path(Model *model, int *path) {
    int size = model->width * model->height;
    int *lookup = calloc(size, sizeof(int));
    for (int i = 0; i < size; i++) {
        lookup[i] = -1;
    }
    for (int i = 0; i < size; i++) {
        lookup[model->next[i]] = i;
    }
    int index = model->end;
    int number = size;
    for (int i = 0; i < size; i++) {
        path[index] = number;
        number--;
        index = lookup[index];
        if (index < 0) {
            break;
        }
    }
    free(lookup);
    int result = 0;
    for (int i = 0; i < size; i++) {
        if (path[i]) {
            result++;
        }
    }
    return result;
}

int compute_energy(Model *model) {
    int size = model->width * model->height;
    int *grid = calloc(size, sizeof(int));
    int count = extract_path(model, grid);
    free(grid);
    return size - count;
}

int do_move(Model *model) {
    int size = model->width * model->height;
    int index;
    do {
        index = randint(size);
    } while (index == model->end);
    int before = model->next[index];
    int result = (index << 16) | before;
    int after;
    do {
        after = random_neighbor(model, index);
    } while (after == before);
    model->next[index] = after;
    return result;
}

void undo_move(Model *model, int undo_data) {
    int index = (undo_data >> 16) & 0xffff;
    int value = undo_data & 0xffff;
    model->next[index] = value;
}

void make_copy(Model *dst, Model *src) {
    dst->width = src->width;
    dst->height = src->height;
    dst->end = src->end;
    for (int i = 0; i < src->width * src->height; i++) {
        dst->next[i] = src->next[i];
    }
}

void init(Model *model, int width, int height) {
    int size = width * height;
    model->width = width;
    model->height = height;
    model->end = randint(size);
    model->next = calloc(size, sizeof(int));
    for (int i = 0; i < size; i++) {
        model->next[i] = random_neighbor(model, i);
    }
}

void uninit(Model *model) {
    free(model->next);
}

int anneal(Model *model, double max_temp, double min_temp, int steps) {
    Model _best;
    Model *best = &_best;
    init(best, model->width, model->height);
    make_copy(best, model);
    double factor = -log(max_temp / min_temp);
    int energy = compute_energy(model);
    int previous_energy = energy;
    int best_energy = energy;
    for (int step = 0; step < steps; step++) {
        double temp = max_temp * exp(factor * step / steps);
        int undo_data = do_move(model);
        energy = compute_energy(model);
        double change = energy - previous_energy;
        if (change > 0 && exp(-change / temp) < randdouble()) {
            undo_move(model, undo_data);
        }
        else {
            previous_energy = energy;
            if (energy < best_energy) {
                best_energy = energy;
                make_copy(best, model);
                if (energy <= 0) {
                    break;
                }
            }
        }
    }
    make_copy(model, best);
    uninit(best);
    return best_energy;
}

void display(Model *model) {
    int size = model->width * model->height;
    int *grid = calloc(size, sizeof(int));
    extract_path(model, grid);
    for (int y = 0; y < model->height; y++) {
        printf("+");
        for (int x = 0; x < model->width; x++) {
            printf("--+");
        }
        printf("\n|");
        for (int x = 0; x < model->width; x++) {
            int i = y * model->width + x;
            if (grid[i]) {
                printf("%02d|", grid[i]);
            }
            else {
                printf("  |");
            }
        }
        printf("\n");
    }
    printf("+");
    for (int x = 0; x < model->width; x++) {
        printf("--+");
    }
    printf("\n\n");
    free(grid);
}

int main(int argc, char **argv) {
    srand(time(NULL));
    Model _model;
    Model *model = &_model;
    int width = 6;
    int height = 6;
    while (1) {
        init(model, width, height);
        int energy = anneal(model, width * height, 0.1, 100000);
        if (energy == 0) {
            display(model);
            break;
        }
    }
    uninit(model);
    return 0;
}
