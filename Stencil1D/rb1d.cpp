#include <iostream>
#include <random>
#include <cassert>
#include <ctime>
#include <upcxx/upcxx.hpp>

using namespace std;

//#include "rb1d-check.cpp"

bool check_convergence(double *u, long n_local, const double EXPECTED_VAL,
                       const double EPSILON, long stepi) {
    double err = 0;
    for (long i = 1; i < n_local - 1; i++)
        err = max(err, fabs(EXPECTED_VAL - u[i]));
    // upcxx collective to get max error over all processes
    double max_err = upcxx::reduce_all(err, upcxx::op_fast_max).wait();
    // check for convergence
    if (max_err / EXPECTED_VAL <= EPSILON) {
        if (!upcxx::rank_me())
            cout << "Converged at " << stepi << ", err " << max_err << endl;
        return true;
    }
    return false;
}

//SNIPPET
int main(int argc, char **argv) {
    upcxx::init();
    // initialize parameters - simple test case
    const long N = 8;
    const long MAX_ITER = N * N * 2;
    const double EPSILON = 0.1;
    const int MAX_VAL = 100;
    const double EXPECTED_VAL = MAX_VAL / 2.0;
    // get the bounds for the local panel, assuming num procs divides N into an even block size
    long block = N / upcxx::rank_n();
    assert(block % 2 == 0);
    assert(N == block * upcxx::rank_n());
    long n_local = block + 2; // plus two for ghost cells
    // set up the distributed object
    upcxx::dist_object<upcxx::global_ptr<double>> u_g(upcxx::new_array<double>(n_local));
    // downcast to a regular C++ pointer
    double *u = u_g->local();
    // init to uniform pseudo-random distribution, independent of job size
    mt19937_64 rgen(1);
    rgen.discard(upcxx::rank_me() * block);
    for (long i = 1; i < n_local - 1; i++)
        u[i] = 0.5 + rgen() % MAX_VAL;

    // fetch the left and right pointers for the ghost cells
    int l_nbr = (upcxx::rank_me() + upcxx::rank_n() - 1) % upcxx::rank_n();
    int r_nbr = (upcxx::rank_me() + 1) % upcxx::rank_n();
    upcxx::global_ptr<double> uL = u_g.fetch(l_nbr).wait();
    upcxx::global_ptr<double> uR = u_g.fetch(r_nbr).wait();
    upcxx::barrier(); // optional - wait for all ranks to finish init

    // iteratively solve
    bool converged = false;
    for (long stepi = 0; stepi < MAX_ITER; stepi++) {
        // alternate between red and black
        int phase = stepi % 2;
        // get the values for the ghost cells
        if (!phase) u[0] = upcxx::rget(uL + block).wait();
        else u[n_local - 1] = upcxx::rget(uR + 1).wait();
        // compute updates and error
        for (long i = phase + 1; i < n_local - 1; i += 2)
            u[i] = (u[i - 1] + u[i + 1]) / 2.0;

        for (int i = 0; i < n_local; i++) {
            std::cout << u[i] << "\t";
        }
        std::cout << "stepi = " << stepi << std::endl;

        upcxx::barrier(); // wait until all processes have finished calculations
        if (stepi % 10 == 0) { // periodically check convergence
            if (check_convergence(u, n_local, EXPECTED_VAL, EPSILON, stepi)) {
                converged = true;
                break;
            }
        }
    }
    if (!converged)
        if (!upcxx::rank_me())
            std::cout << "Iteration complete but still didn't convergence." << std::endl;
    upcxx::finalize();
    return 0;
}
//SNIPPET
