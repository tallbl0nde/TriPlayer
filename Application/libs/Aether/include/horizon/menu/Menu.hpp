#ifndef AETHER_MENU_HPP
#define AETHER_MENU_HPP

#include "base/Scrollable.hpp"
#include "horizon/menu/MenuOption.hpp"

namespace Aether {
    // A menu is simply a scrollable object that sets some values
    // on creation in order to appear like Horizon's menu.
    class Menu : public Scrollable {
        private:
            // Pointer to currently "active" element
            MenuOption * active;

        public:
            // Hides scrollbar + adjusts scroll 'catchup'
            Menu(int, int, int, int);

            // Keep the selected item centred
            void update(uint32_t);

            // Set the given option as the highlighted option
            // Unsets previously highlighted
            void setActiveOption(MenuOption *);
    };
};

#endif