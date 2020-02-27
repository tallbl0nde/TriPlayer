#ifndef AETHER_LISTHEADINGHELP_HPP
#define AETHER_LISTHEADINGHELP_HPP

#include "base/Container.hpp"
#include "horizon/button/HelpButton.hpp"
#include "horizon/list/ListHeading.hpp"

namespace Aether {
    // A ListHeading but with a help icon that's right aligned
    class ListHeadingHelp : public Container {
        private:
            ListHeading * heading;
            HelpButton * help;

            // Called on size change to reposition elements
            void positionElements();

        public:
            // Constructor creates + positions elements
            // String, callback function
            ListHeadingHelp(std::string, std::function<void()>);

            // Getter + setters for colours
            Colour getHelpColour();
            void setHelpColour(Colour);
            Colour getRectColour();
            void setRectColour(Colour);
            Colour getTextColour();
            void setTextColour(Colour);

            // Set callback
            void setHelpCallback(std::function<void()>);

            // Adjusting width/height requires repositioning
            void setW(int);
            void setH(int);
    };
};

#endif