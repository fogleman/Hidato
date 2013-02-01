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

int rand_int(int n) {
    int result;
    while (n <= (result = rand() / (RAND_MAX / n)));
    return result;
}

double rand_double() {
    return (double)rand() / (double)RAND_MAX;
}

int random_neighbor(int width, int height, int index) {
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
            printf("--+");
        }
        printf("\n|");
        for (int x = 0; x < width; x++) {
            int i = y * width + x;
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
    for (int x = 0; x < width; x++) {
        printf("--+");
    }
    printf("\n\n");
}

void gen_init(Model *model, int width, int height) {
    int size = width * height;
    model->width = width;
    model->height = height;
    model->end = rand_int(size);
    model->next = calloc(size, sizeof(int));
    for (int i = 0; i < size; i++) {
        model->next[i] = random_neighbor(width, height, i);
    }
}

void gen_uninit(Model *model) {
    free(model->next);
}

int gen_extract(Model *model, int *grid) {
    int size = model->width * model->height;
    int lookup[size];
    for (int i = 0; i < size; i++) {
        grid[i] = 0;
        lookup[i] = -1;
    }
    for (int i = 0; i < size; i++) {
        lookup[model->next[i]] = i;
    }
    int index = model->end;
    int number = size;
    for (int i = 0; i < size; i++) {
        grid[index] = number;
        number--;
        index = lookup[index];
        if (index < 0) {
            break;
        }
    }
    int result = 0;
    for (int i = 0; i < size; i++) {
        if (grid[i]) {
            result++;
        }
    }
    return result;
}

int gen_energy(Model *model) {
    int size = model->width * model->height;
    int grid[size];
    int count = gen_extract(model, grid);
    return size - count;
}

int gen_do_move(Model *model) {
    int size = model->width * model->height;
    int index;
    do {
        index = rand_int(size);
    } while (index == model->end);
    int before = model->next[index];
    int after;
    do {
        after = random_neighbor(model->width, model->height, index);
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
    dst->end = src->end;
    for (int i = 0; i < src->width * src->height; i++) {
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

int main(int argc, char **argv) {
    srand(time(NULL));
    Model _model;
    Model *model = &_model;
    int width = 6;
    int height = 6;
    while (1) {
        gen_init(model, width, height);
        int energy = gen_anneal(model, width * height, 0.1, 100000);
        if (energy == 0) {
            int grid[width * height];
            gen_extract(model, grid);
            display(width, height, grid);
            break;
        }
    }
    gen_uninit(model);
    return 0;
}
