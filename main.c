/*
 Code framework for solving the Travelling Salesman Problem with local search
 Copyright (C) 2023 Christine Solnon
 Ce programme est un logiciel libre ; vous pouvez le redistribuer et/ou le modifier au titre des clauses de la Licence Publique Générale GNU, telle que publiée par la Free Software Foundation. Ce programme est distribué dans l'espoir qu'il sera utile, mais SANS AUCUNE GARANTIE ; sans même une garantie implicite de COMMERCIABILITE ou DE CONFORMITE A UNE UTILISATION PARTICULIERE. Voir la Licence Publique Générale GNU pour plus de détails.
 */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

int iseed = 1;

double duration_seconds(clock_t since) {
    return ((double) (clock() - since)) / CLOCKS_PER_SEC;
}

// Return an integer value in [0, n-1], according to a pseudo-random sequence
int nextRand(int n) {
    int i = 16807 * (iseed % 127773) - 2836 * (iseed / 127773);
    if (i > 0) iseed = i;
    else iseed = 2147483647 + i;
    return iseed % n;
}

// input: the number n of vertices and a file descriptor fd
// Return a symmetrical cost matrix such that, for each i,j in [0, n-1], cost[i][j] = cost of arc (i,j)
// side effect: print in fd a Python script for defining turtle coordinates associated with vertices
int **createCost(size_t n, FILE *fd) {
    int x[n], y[n];
    int max = 1000;
    int **cost;
    cost = (int **) malloc(n * sizeof(int *));
    fprintf(fd, "import turtle\n");
    fprintf(fd, "turtle.setworldcoordinates(0, 0, %d, %d)\n", max, max + 100);
    for (size_t i = 0; i < n; i++) {
        x[i] = nextRand(max);
        y[i] = nextRand(max);
        fprintf(fd, "p%zu=(%d,%d)\n", i, x[i], y[i]);
        cost[i] = (int *) malloc(n * sizeof(int));
    }
    for (size_t i = 0; i < n; i++) {
        cost[i][i] = max * max;
        for (size_t j = i + 1; j < n; j++) {
            cost[i][j] = (int) sqrt((x[i] - x[j]) * (x[i] - x[j]) + (y[i] - y[j]) * (y[i] - y[j]));
            cost[j][i] = cost[i][j];
        }
    }
    return cost;
}

// Input: the number of vertices n, the cost matrix such that for all i,j in [0,n-1], cost[i][j] = cost of arc (i,j), the seed for the random number generator
// Output: sol[0..n-1] is a random permutation of [0..n-1]
// Return cost[n-1][0] + the sum of cost[i][i+1] for i in [0,n-2]
int generateRandomTour(size_t n, const int **cost, size_t *sol) {
    size_t cand[n];
    for (size_t i = 0; i < n; i++) cand[i] = i;
    sol[0] = nextRand((int) n);
    cand[sol[0]] = n - 1;
    int total = 0;
    size_t nbCand = n - 1;
    for (size_t i = 1; i < n; i++) {
        int j = nextRand((int) nbCand);
        sol[i] = cand[j];
        cand[j] = cand[--nbCand];
        total += cost[sol[i - 1]][sol[i]];
    }
    total += cost[sol[n - 1]][sol[0]];
    return total;
}

int length(size_t n, const size_t *solution, const int **cost) {
    int total = 0;
    for (size_t i = 1; i < n; i++) {
        total += cost[solution[i - 1]][solution[i]];
    }
    total += cost[solution[n - 1]][solution[0]];
    return total;
}

// Input: n = number n of vertices; sol[0..n-1] = permutation of [0,n-1]; fd = file descriptor
// Side effect: print in fd the Python script for displaying the tour associated with sol
void print(size_t *sol, size_t n, int totalLength, FILE *fd) {
    fprintf(fd, "turtle.clear()\n");
    fprintf(fd, "turtle.tracer(0,0)\n");
    fprintf(fd, "turtle.penup()\n");
    fprintf(fd, "turtle.goto(0,%d)\n", 1050);
    fprintf(fd, "turtle.write(\"Total length = %d\")\n", totalLength);
    fprintf(fd, "turtle.speed(0)\n");
    fprintf(fd, "turtle.goto(p%zu)\n", sol[0]);
    fprintf(fd, "turtle.pendown()\n");
    for (int i = 1; i < n; i++) fprintf(fd, "turtle.goto(p%zu)\n", sol[i]);
    fprintf(fd, "turtle.goto(p%zu)\n", sol[0]);
    fprintf(fd, "turtle.update()\n");
    fprintf(fd, "wait = input(\"Enter return to continue\")\n");
}


void swap(size_t *array, size_t x, size_t y) {
    size_t tmp = array[x];
    array[x] = array[y];
    array[y] = tmp;
}

typedef struct {
    size_t i;
    size_t j;
} Pair;

int greedyLS(int total, size_t n, size_t *solution, const int **cost) {
    // Insert your code for greedily improving solution here!
    while(true) {
        int best = 0;
        Pair swap_indices;
        for (size_t i = 0; i < n; i++) {
            for (size_t j = i + 2; j <= n; j++) {
                int benefit = cost[solution[i]][solution[(i + 1) % n]] + cost[solution[j % n]][solution[(j + 1) % n]] -
                              cost[solution[i]][solution[j % n]] - cost[solution[(i + 1) % n]][solution[(j + 1) % n]];
                if (benefit > best) {
                    best = benefit;
                    swap_indices.i = i + 1;
                    swap_indices.j = j;
                }
            }
        }
        if (best == 0) break;
        total -= best;
        while (swap_indices.i < swap_indices.j) {
            swap(solution, swap_indices.i % n, swap_indices.j % n);
            swap_indices.i++;
            swap_indices.j--;
        }
    }
    return total;
}


void ils(size_t k, size_t l, size_t n, size_t *sol_opt, const int **cost, FILE* fd) {
    int opt_length = generateRandomTour(n, cost, sol_opt);
    printf("Initial tour length = %d; ", opt_length);
    clock_t start = clock();
    opt_length = greedyLS(opt_length, n, sol_opt, cost);
    printf("Tour length after GreedyLS = %d; ", opt_length);
    printf("Time = %lfs;\n", duration_seconds(start));

    size_t curr[n];
    for (size_t i = 0; i < k; i++) {
        memcpy(curr, sol_opt, n * sizeof(size_t));
        for (size_t j = 0; j < l; j++) {
            swap(curr, (size_t) nextRand((int) n), (size_t) nextRand((int) n));
        }
        int curr_length = length(n, curr, cost);
        start = clock();
        curr_length = greedyLS(curr_length, n, curr, cost);
        if (curr_length < opt_length) {
            opt_length = curr_length;
            memcpy(sol_opt, curr, n * sizeof(size_t));
            printf("New best found at iteration %zu; Total length = %d; Time = %lf\n", i, curr_length,
                   duration_seconds(start));
            print(sol_opt, n, opt_length, fd);
        }
    }
}

size_t input(const char *text) {
    size_t value;
    printf("%s", text);
    fflush(stdout);
    int _ = scanf("%zu", &value);
    return value;
}

int main() {
    size_t k = input("Number of iterations of ILS (k): ");
    size_t l = input("Perturbation strength (l): ");
    size_t n = input("Number of vertices: ");
    FILE *fd = fopen("script.py", "w");
    int **cost = createCost(n, fd);
    size_t sol[n];
    ils(k, l, n, sol, (const int **) cost, fd);
    return 0;
}

