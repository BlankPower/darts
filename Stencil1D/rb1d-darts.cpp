//
// Created by Ember on 2022/3/11.
//
#include <iostream>
#include <cstdlib>
#include "darts.h"
#include "rb1d-darts.h"

#define DEBUG 0

void linkCD::fire() {
    // only enter once
#ifdef DEBUG
    std::cout << "procTP::linkCD " << this << std::endl;
#endif
    auto myProc = static_cast<procTP *>(myTP_);
    if (myProc == nullptr) {
        std::cout << "Error: failed to fetch the pointer of procTP" << std::endl;
    } else {
        auto myMain = static_cast<mainTP *>(myProc->parentTP_);
        myMain->procs[myProc->rank] = myProc;
        myMain->init.decDep();
    }
}

void initCD::fire() {
    // only enter once
#ifdef DEBUG
    std::cout << "mainTP::initCD" << std::endl;
#endif
    auto myMain = static_cast<mainTP *>(myTP_);
    for (int i = 1; i < myMain->nprocs; i++)
        myMain->procs[i]->lnbr = myMain->procs[i - 1];
    myMain->procs[0]->lnbr = myMain->procs[myMain->nprocs - 1];
    for (int i = 0; i < myMain->nprocs - 1; i++)
        myMain->procs[i]->rnbr = myMain->procs[i + 1];
    myMain->procs[myMain->nprocs - 1]->rnbr = myMain->procs[0];

    for (int i = 0; i < myMain->nprocs; i++) {
        // Started at step 0, only need to get left neighbor and save to Proc->u[0]
        myMain->procs[i]->getl.decDep();
    }
}

void getLCD::fire() {
    auto myProc = static_cast<procTP *>(myTP_);
#if DEBUG
    std::cout << "procTP::getLCD (" << myProc << ")" << std::endl;
#endif
    myProc->u[0] = myProc->lnbr->u[myProc->n_local - 2];
    resetCodelet();
    myProc->work.decDep();
}

void getRCD::fire() {
    auto myProc = static_cast<procTP *>(myTP_);
#if DEBUG
    std::cout << "procTP::getRCD (" << myProc << ")" << std::endl;
#endif
    myProc->u[myProc->n_local - 1] = myProc->rnbr->u[1];
    resetCodelet();
    myProc->work.decDep();
}

void workCD::fire() {
    // assume u[0] or u[n_local - 1] is ready when workCD invoked
    auto myProc = static_cast<procTP *>(myTP_);
#if DEBUG
    std::cout << "procTP::workCD (" << myProc << ") stepi = " << std::dec << myProc->stepi << std::endl;
#endif
    int phase = myProc->stepi % 2;
    // step: 0 2 4... operate on u[1 3 5...]
    // step: 1 3 5... operate on u[2 4 6...]
    for (int i = phase + 1; i < myProc->n_local - 1; i += 2)
        myProc->u[i] = (myProc->u[i - 1] + myProc->u[i + 1]) / 2.0;

    if (myProc->stepi % 10 == 0) {
        myProc->max_err = 0;
        for (int i = 1; i < myProc->n_local - 1; i++)
            myProc->max_err = std::max(myProc->max_err, fabs(EXPECTED_VAL - myProc->u[i]));
    }
    myProc->stepi += 1;

    resetCodelet();
    myProc->toSignal->decDep();
}

void syncCD::fire() {
    // sync with steps
    auto myMain = static_cast<mainTP *>(myTP_);
    int stepi = myMain->procs[0]->stepi - 1;
#if DEBUG
    std::cout << "mainTP::syncCD (" << myMain << ") stepi = " << std::dec << stepi << std::endl;
#endif
    if (stepi % 10 == 0) {
#ifdef  DEBUG
        if (stepi % 10000 == 0) {
            uint64_t temp = getTime();
            std::cout << "stepi = " << stepi << ", Elapsed time = " << temp - myMain->cur_time << std::endl;
            myMain->cur_time = temp;
        }
#endif
        double max_err = 0;
        for (int i = 0; i < myMain->nprocs; i++)
            max_err = std::max(max_err, myMain->procs[i]->max_err);
        if (max_err / EXPECTED_VAL <= EPSILON) {
            std::cout << "Converged at " << stepi << ", err " << max_err << std::endl;
            myMain->toSignal->decDep();
            return;
        }
    }

    if (stepi + 1 >= MAX_ITER) {
        std::cout << "Iteration complete but still didn't convergence." << std::endl;
        myMain->toSignal->decDep();
        return;
        std::cout << "Should never be here" << std::endl;
    }

    resetCodelet();
    int phase = stepi % 2;
    if (phase) {
        // step: 1 3 5... operate on u[2 4 6...]
        for (int i = 0; i < myMain->nprocs; i++) {
            myMain->procs[i]->getl.decDep();
        }
    } else {
        // step: 0 2 4... operate on u[1 3 5...]
        for (int i = 0; i < myMain->nprocs; i++) {
            myMain->procs[i]->getr.decDep();
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cout << "enter number of TPs and CDs" << std::endl;
        return 0;
    }
    int tps = atoi(argv[1]);
    int cds = atoi(argv[2]);
    int nprocs = tps - 1;

    if (tps < 2) {
        std::cout << "tps should be larger than 1" << std::endl;
        return 0;
    }

    ThreadAffinity affin(cds, tps, SPREAD, TPDYNAMIC, MCDYNAMIC);
    if (affin.generateMask()) {
        Runtime *rt = new Runtime(&affin);
        rt->run(launch<mainTP>(nprocs, &Runtime::finalSignal));
        delete rt;
    }
    return 0;
}