#ifndef PLAYERGUI_HPP
#define PLAYERGUI_HPP

#include "tesla.hpp"

class PlayerGui : public tsl::Gui {
    private:

    public:
        tsl::elm::Element * createUI();
};

#endif