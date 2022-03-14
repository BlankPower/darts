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
    const int N = 8;
    const int MAX_ITER = N * N * 2;
    const double EPSILON = 0.1;
    const int MAX_VAL = 100;
    const double EXPECTED_VAL = MAX_VAL / 2.0;

    int n_local = N + 2;
    auto *u = new double[n_local];
    auto *v = new double[N];
    std::mt19937_64 rgen(1);
    for (int i = 1; i < n_local - 1; i++) {
        u[i] = 0.5 + rgen() % MAX_VAL;
        v[i - 1] = u[i];
    }
    for (int i = 0; i < n_local; i++) {
        std::cout << u[i] << "\t";
    }
    std::cout << "Initial" << std::endl;
    bool converged = false;
    for (int stepi = 0; stepi < MAX_ITER; stepi++) {
        int phase = stepi % 2;
        if (!phase) u[0] = u[N];
        else u[n_local - 1] = u[1];
        for (int i = phase + 1; i < n_local - 1; i += 2) {
            u[i] = (u[i - 1] + u[i + 1]) / 2.0;
        }
        for (int i = 0; i < n_local; i++) {
            std::cout << u[i] << "\t";
        }
        std::cout << "stepi = " << stepi << std::endl;
        if (stepi % 10 == 0) {
            if (check_convergence(u, n_local, EXPECTED_VAL, EPSILON, stepi)) {
                converged = true;
                break;
            }
        }
    }
    if (!converged)
        std::cout << "Iteration complete but still didn't convergence." << std::endl;

    std::cout << "==========================" << std::endl;

    for (int i = 0; i < N; i++) {
        std::cout << v[i] << "\t";
    }
    std::cout << "Initial" << std::endl;
    for (int stepi = 0; stepi < MAX_ITER; stepi++) {
        auto temp = new double[N];
        for (int i = 0; i < N; i++)
            temp[i] = v[i];
        for (int i = 0; i < N; i++) {
            int l = (i + N - 1) % N;
            int r = (i + 1) % N;
            v[i] = (temp[l] + temp[r]) / 2.0;
        }
        for (int i = 0; i < N; i++) {
            std::cout << v[i] << "\t";
        }
        std::cout << "stepi = " << stepi << std::endl;
        if (stepi % 10 == 0) {
            if (check_convergence(v, N, EXPECTED_VAL, EPSILON, stepi)) {
                break;
            }
        }
    }
    return 0;
}

