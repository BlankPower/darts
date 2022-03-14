//
// Created by Ember on 2022/3/11.
//

#include <ctime>
#include <iostream>
#include <random>

bool check_convergence(double *u, const int N, const double EXPECTED_VAL, const double EPSILON, int stepi) {
    double err = 0;
    for (int i = 0; i < N; i++)
        err = std::max(err, fabs(EXPECTED_VAL - u[i]));
    if (err / EXPECTED_VAL <= EPSILON) {
        std::cout << "Converged at " << stepi << ", err " << err << std::endl;
        return true;
    }
    return false;
}

int main() {
    const int N = 1024;
    const int MAX_ITER = N * N * 2;
    const double EPSILON = 0.1;
    const int MAX_VAL = 100;
    const double EXPECTED_VAL = MAX_VAL / 2;

    double *u = new double[N];
    std::mt19937_64 rgen(time(nullptr));
    for (int i = 0; i < N; i++) {
        u[i]  = 0.5 + rgen() % MAX_VAL;
    }
    bool converged = false;
    for (int stepi = 0; stepi < MAX_ITER; stepi++) {
        int phase = stepi % 2;
        if (!phase) u[0] = (u[N - 1] + u[1]) / 2.0;
        else u[N - 1] = (u[0] + u[N - 2]) / 2.0;
        for (int i = phase + 1; i < N - 1; i += 2) {
            u[i] = (u[i - 1] + u[i + 1]) / 2.0;
        }
        if (stepi % 10 == 0) {
            if (check_convergence(u, N, EXPECTED_VAL, EPSILON, stepi)) {
                converged = true;
                break;
            }
        }
    }
    if (!converged)
        std::cout << "Iteration complete but still didn't convergence." << std::endl;
    return 0;
}

