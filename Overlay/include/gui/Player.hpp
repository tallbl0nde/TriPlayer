#ifndef GUI_PLAYER_HPP
#define GUI_PLAYER_HPP

#include "tesla.hpp"

namespace Gui {
    class Player : public tsl::Gui {
        private:

        public:
            tsl::elm::Element * createUI();
    };
};

#endif