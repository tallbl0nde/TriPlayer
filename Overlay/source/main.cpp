#define TESLA_INIT_IMPL

#include "tesla.hpp"
#include "TriOverlay.hpp"

// Main simply launches the overlay and loops
int main(int argc, char * argv[]) {
    return tsl::loop<TriOverlay>(argc, argv);
}
