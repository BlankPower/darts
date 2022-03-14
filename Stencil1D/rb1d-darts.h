//
// Created by Ember on 2022/3/11.
//

#ifndef DARTS_RB1D_DARTS_H
#define DARTS_RB1D_DARTS_H

#include <random>
#include <ctime>
#include <iostream>
#include <cstdlib>
#include "darts.h"
#include "utils.h"

using namespace darts;

class linkCD : public Codelet {
public:
    linkCD() = default;

    linkCD(uint32_t dep, uint32_t res, ThreadedProcedure *myTP, uint32_t stat) : Codelet(dep, res, myTP, stat) {};

    void fire() override;
};

class workCD : public Codelet {
public:
    workCD() = default;

    workCD(uint32_t dep, uint32_t res, ThreadedProcedure *myTP, uint32_t stat) : Codelet(dep, res, myTP, stat) {};

    void fire() override;
};

class getLCD : public Codelet {
public:
    getLCD() = default;

    getLCD(uint32_t dep, uint32_t res, ThreadedProcedure *myTP, uint32_t stat) : Codelet(dep, res, myTP, stat) {};

    void fire() override;
};

class getRCD : public Codelet {
public:
    getRCD() = default;

    getRCD(uint32_t dep, uint32_t res, ThreadedProcedure *myTP, uint32_t stat) : Codelet(dep, res, myTP, stat) {};

    void fire() override;
};

class procTP : public ThreadedProcedure {
public:
    double *u{};
    int rank{};
    int n_local{};
    int stepi{};
    double max_err{};
    procTP *lnbr{}, *rnbr{};
    linkCD link;
    getLCD getl;
    getRCD getr;
    workCD work;
    Codelet *toSignal{};

    procTP(int rank, int n_local, Codelet *toSig) : ThreadedProcedure(),
                                                    rank(rank),
                                                    n_local(n_local),
                                                    stepi(0),
                                                    max_err(0),
                                                    link(0, 0, this, SHORTWAIT),
                                                    getl(1, 1, this, LONGWAIT),
                                                    getr(1, 1, this, LONGWAIT),
                                                    work(1, 1, this, LONGWAIT),
                                                    toSignal(toSig) {
        // toSig : myMain::sync
        std::cout << "procTP::construction" << std::endl;
        u = new double[n_local];
        std::mt19937_64 rgen(1);
        for (int i = 1; i < n_local - 1; i++)
            u[i] = 0.5 + rgen() % MAX_VAL;

        // link lnbr and rnbr procTP
        add(&link);
    };

    procTP() = default;
};

class initCD : public Codelet {
public:
    initCD(uint32_t dep, uint32_t res, ThreadedProcedure *myTP, uint32_t stat) : Codelet(dep, res, myTP, stat) {};

    void fire() override;
};

class syncCD : public Codelet {
public:
    syncCD(uint32_t dep, uint32_t res, ThreadedProcedure *myTP, uint32_t stat) : Codelet(dep, res, myTP, stat) {};

    void fire() override;
};

class mainTP : public ThreadedProcedure {
public:
    int nprocs;
    uint64_t cur_time;
    initCD init;
    syncCD sync;
    procTP **procs;
    Codelet *toSignal;

    mainTP(int nprocs, Codelet *toSig) : ThreadedProcedure(),
                                         nprocs(nprocs),
                                         cur_time(0),
                                         init(nprocs, nprocs, this, LONGWAIT),
                                         sync(nprocs, nprocs, this, LONGWAIT),
                                         toSignal(toSig) {
        procs = new procTP *[nprocs];
        int block = N / nprocs;
        assert(block % 2 == 0);
        assert(block * procs == N);
        int n_local = block + 2;
        std::cout << "mainTP::constructed " << nprocs << " processes" << std::endl;
        for (int i = 0; i < nprocs; i++) {
//            procs[i] = procTP(n_local, &sync);
//TODO: cannot get the address of invoked TP
//      or send messages between TPs?
            invoke<procTP>(this, i, n_local, &sync);
        }
        add(&init);
    }
};


#endif //DARTS_RB1D_DARTS_H
