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
        model->next[i] = random_neighbor(model->width, model->height, i);
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

int main(int argc, char **argv) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
    Model _model;
    Model *model = &_model;
    int width = 16;
    int height = width;
    while (1) {
        gen_init(model, width, height);
        gen_randomize(model);
        int energy = gen_anneal(model, 10, 0.1, 100000);
        if (energy == 0) {
            gen_display(model);
        }
        gen_uninit(model);
        if (energy == 0) {
            break;
        }
    }
    return 0;
}
