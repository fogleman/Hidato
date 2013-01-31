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

double random() {
    return (double)rand() / (double)RAND_MAX;
}

int random_neighbor(Model *model, int index) {
    int x = index % model->width;
    int y = index / model->width;
    while (1) {
        int dx = randint(2) - 1;
        int dy = randint(2) - 1;
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
        if (model->next[i] >= 0) {
            lookup[model->next[i]] = i;
        }
    }
    int index = model->end;
    int number = size;
    int result = 0;
    for (int i = 0; i < size; i++) {
        result++;
        if (path) {
            path[index] = number;
        }
        number--;
        index = lookup[index];
        if (index < 0) {
            break;
        }
    }
    free(lookup);
    return result;
}

double compute_energy(Model *model) {
    int size = model->width * model->height;
    return size - extract_path(model, 0);
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

double anneal(Model *model, double max_temp, double min_temp, int steps) {
    printf("here\n");
    double factor = -log(max_temp / min_temp);
    double energy = compute_energy(model);
    double previous_energy = energy;
    double best_energy = energy;
    // best_state
    for (int step = 0; step < steps; step++) {
        printf("%d\n", step);
        double temp = max_temp * exp(factor * step / steps);
        int undo_data = do_move(model);
        energy = compute_energy(model);
        double change = energy - previous_energy;
        if (change > 0 && exp(-change / temp) < random()) {
            undo_move(model, undo_data);
        }
        else {
            previous_energy = energy;
            if (energy < best_energy) {
                best_energy = energy;
            }
        }
    }
    return best_energy;
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

int main(int argc, char **argv) {
    srand(time(NULL));
    Model model;
    init(&model, 8, 8);
    double energy = anneal(&model, 64, 0.1, 100000);
    printf("%d\n", energy);
    uninit(&model);
    return 0;
}
