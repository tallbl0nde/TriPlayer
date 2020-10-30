#ifndef GUI_ERROR_HPP
#define GUI_ERROR_HPP

#include "tesla.hpp"

// The ErrorGui shows a brief error message indicating some part
// of the overlay didn't start as expected.
namespace Gui {
    class Error : public tsl::Gui {
        public:
            // Creates all required UI elements
            tsl::elm::Element * createUI();
    };
};

#endif