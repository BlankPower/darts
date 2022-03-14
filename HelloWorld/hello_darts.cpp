#include <iostream>
#include <cstdlib>
#include "darts.h"

using namespace darts;

class HelloCD : public Codelet
{
public:
    int rank;
    HelloCD(uint32_t dep, uint32_t res, ThreadedProcedure *myTP, uint32_t stat, int rank) : Codelet(dep, res, myTP, stat),
                                                                                            rank(rank) {}
    HelloCD(void) {}
    virtual void fire(void);
};

class SyncCD : public Codelet
{
public:
    SyncCD(uint32_t dep, uint32_t res, ThreadedProcedure *myTP, uint32_t stat) : Codelet(dep, res, myTP, stat) {}
    virtual void fire(void);
};

class HelloTP : public ThreadedProcedure
{
public:
    int procs;
    HelloCD *hello;
    SyncCD sync;
    Codelet *toSignal;
    HelloTP(int procs, Codelet *toSig) : ThreadedProcedure(),
                                         procs(procs),
                                         sync(procs, procs, this, LONGWAIT),
                                         toSignal(toSig)
    {
        hello = new HelloCD[procs];
        for (int i = 0; i < procs; i++)
        {
            hello[i] = HelloCD(0, 0, this, SHORTWAIT, i);
            add(&hello[i]);
        }
    }
};

void HelloCD::fire(void)
{
    HelloTP *myHello = static_cast<HelloTP *>(myTP_);
    printf("Hello from process %d out of %d processes\n", rank, myHello->procs);
    // std::cout << "Hello from codelet " << rank << " out of " << myHello->procs << " processes" << std::endl;
    myHello->sync.decDep();
}
void SyncCD::fire(void)
{
    HelloTP *myHello = static_cast<HelloTP *>(myTP_);
    std::cout << "Final sync." << std::endl;
    myHello->toSignal->decDep();
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cout << "enter number of TPs and CDs" << std::endl;
        return 0;
    }
    int tps = atoi(argv[1]);
    int cds = atoi(argv[2]);
    int procs = cds * tps;

    ThreadAffinity affin(cds, tps, SPREAD, TPDYNAMIC, MCDYNAMIC);
    if (affin.generateMask())
    {
        Runtime *rt = new Runtime(&affin);
        rt->run(launch<HelloTP>(procs, &Runtime::finalSignal));
        delete rt;
    }
    return 0;
}