#include <algorithm>
#include "ui/element/listitem/File.hpp"
#include "ui/overlay/FileBrowser.hpp"
#include "utils/FS.hpp"
#include "utils/Utils.hpp"

// List padding
#define PADDING 50

namespace CustomOvl {
    FileBrowser::FileBrowser(int w, int h) : Overlay() {
        // Rectangle background
        this->rect = new Aether::Rectangle(640 - w/2, 360 - h/2, w, h, 8);
        this->addElement(this->rect);
        this->setTopLeft(this->rect->x(), this->rect->y());
        this->setBottomRight(this->rect->x() + this->rect->w(), this->rect->y() + this->rect->h());

        // Headings
        this->heading = new Aether::Text(this->rect->x() + PADDING, this->rect->y() + PADDING/2, "", 30);
        this->addElement(this->heading);
        this->path = new Aether::Text(this->heading->x(), this->heading->y() + 50, "", 20);
        this->addElement(this->path);
        this->topR = new Aether::Rectangle(this->rect->x(), this->path->y() + 45, this->rect->w(), 1);
        this->addElement(this->topR);

        // Close button
        this->cancelButton = new Aether::BorderButton(this->rect->x(), this->rect->y() + this->rect->h() - 70, this->rect->w(), 70, 2, "", 26, [this]() {
            this->close();
        });
        this->cancelButton->setBorderColour(Aether::Colour{255, 255, 255, 0});
        this->bottomR = new Aether::Rectangle(this->rect->x(), this->cancelButton->y(), this->rect->w(), 1);
        this->addElement(this->bottomR);
        this->addElement(this->cancelButton);

        // Create main list section
        this->list = new Aether::List(this->rect->x() + PADDING/2, this->topR->y() + 1, this->rect->w() - PADDING, this->bottomR->y() - this->topR->y() - 1);
        this->list->setShowScrollBar(true);
        this->addElement(this->list);
        this->setFocussed(this->list);
    }

    void FileBrowser::populateList() {
        // Make sure list is empty first
        this->list->removeAllElements();

        // Get list of files and filter based on extension
        auto contents = Utils::Fs::getDirectoryContents(this->path->string());
        contents.erase(std::remove_if(contents.begin(), contents.end(), [this](const std::pair<std::string, bool> e) {
            // Don't remove directories
            if (e.second || this->exts.empty()) {
                return false;
            }

            // Check if file extension is in list
            for (size_t i = 0; i < this->exts.size(); i++) {
                if (Utils::toLowercase(Utils::Fs::getExtension(e.first)) == Utils::toLowercase(this->exts[i])) {
                    return false;
                }
            }
            return true;
        }), contents.end());

        // Sort to have directories first
        std::sort(contents.begin(), contents.end(), [](const std::pair<std::string, bool> lhs, const std::pair<std::string, bool> rhs) {
            return lhs.second > rhs.second;
        });
        size_t splitOn = 0;
        for (size_t i = 0; i < contents.size(); i++) {
            if (!contents[i].second) {
                splitOn = i;
                break;
            }
        }
        std::sort(contents.begin(), contents.begin() + splitOn, [](const std::pair<std::string, bool> lhs, const std::pair<std::string, bool> rhs) {
            return lhs.first < rhs.first;
        });
        std::sort(contents.begin() + splitOn, contents.end(), [](const std::pair<std::string, bool> lhs, const std::pair<std::string, bool> rhs) {
            return lhs.first < rhs.first;
        });

        // Insert into list
        if (Utils::Fs::getParentDirectory(this->path->string()) != this->path->string()) {
            contents.insert(contents.begin(), std::make_pair("..", true));
        }
        for (size_t i = 0; i < contents.size(); i++) {
            std::string path;
            if (contents[i].first == "..") {
                path = Utils::Fs::getParentDirectory(this->path->string());
            } else {
                path = this->path->string() + (this->path->string() == "/" ? "" : "/") + contents[i].first;
            }
            CustomElm::ListItem::File * l;
            if (contents[i].second) {
                l = new CustomElm::ListItem::File(contents[i].first, true, [this, path]() {
                    this->setPath(path);
                });
            } else {
                l = new CustomElm::ListItem::File(contents[i].first, false, [this, path]() {
                    this->setFile(path);
                });
            }
            l->setIconColour(this->path->getColour());
            l->setTextColour(this->text);
            this->list->addElement(l);
        }
    }

    void FileBrowser::setFile(const std::string & s) {
        this->file = s;
        this->close();
    }

    std::string FileBrowser::chosenFile() {
        return this->file;
    }

    void FileBrowser::resetFile() {
        this->file.clear();
    }

    void FileBrowser::setExtensions(const std::vector<std::string> e) {
        this->exts = e;
    }

    void FileBrowser::setPath(const std::string & p) {
        this->path->setString(p);
        this->populateList();
    }

    void FileBrowser::setCancelText(std::string s) {
        this->cancelButton->setString(s);
    }

    void FileBrowser::setHeadingText(std::string s) {
        this->heading->setString(s);
    }

    void FileBrowser::setAccentColour(Aether::Colour c) {
        this->cancelButton->setTextColour(c);
    }

    void FileBrowser::setMutedLineColour(Aether::Colour c) {
        this->topR->setColour(c);
        this->bottomR->setColour(c);
    }

    void FileBrowser::setMutedTextColour(Aether::Colour c) {
        this->list->setScrollBarColour(c);
        this->path->setColour(c);
    }

    void FileBrowser::setTextColour(Aether::Colour c) {
        this->text = c;
    }

    void FileBrowser::setRectangleColour(Aether::Colour c) {
        this->rect->setColour(c);
    }

    bool FileBrowser::handleEvent(Aether::InputEvent * e) {
        if (e->type() == Aether::EventType::ButtonPressed) {
            if (e->button() == Aether::Button::B) {
                // If we can't move up a level just close the overlay
                std::string parent = Utils::Fs::getParentDirectory(this->path->string());
                if (parent == this->path->string()) {
                    this->close();

                // Otherwise move up a level
                } else {
                    this->setPath(parent);
                }

                return true;
            }
        }

        return Overlay::handleEvent(e);
    }
};