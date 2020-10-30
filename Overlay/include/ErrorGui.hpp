#ifndef ERRORGUI_HPP
#define ERRORGUI_HPP

#include "tesla.hpp"

// The ErrorGui shows a brief error message indicating some part
// of the overlay didn't start as expected.
class ErrorGui : public tsl::Gui {
    public:
        // Creates all required UI elements
        tsl::elm::Element * createUI();
};

#endif