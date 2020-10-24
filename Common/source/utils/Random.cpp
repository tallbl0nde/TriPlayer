#include <ctime>
#include <random>
#include "utils/Random.hpp"

// Static random generator
static std::default_random_engine gen;
static bool seeded = false;

namespace Utils::Random {
    size_t getSizeT(const size_t min, const size_t max) {
        // Ensure it's seeded
        if (!seeded) {
            gen.seed(std::time(nullptr));
            gen.discard(3);
            seeded = true;
        }

        // Generate and return number
        std::uniform_int_distribution<unsigned long long> dist(min, max);
        return dist(gen);
    }
}