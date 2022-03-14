#include <iostream>
#include <upcxx/upcxx.hpp>

using namespace std;

int main() {
    upcxx::init();
    cout << "Hello world from process " << upcxx::rank_me()
        << " out of " << upcxx::rank_n() << " processes" << endl;
    upcxx::finalize();
}