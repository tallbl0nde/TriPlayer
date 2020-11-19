#include "Application.hpp"
#include <filesystem>
#include "ui/frame/settings/AppAdvanced.hpp"
#include "utils/FS.hpp"

namespace Frame::Settings {
    AppAdvanced::AppAdvanced(Main::Application * a) : Frame(a) {
        // Temporary variables
        Config * cfg = this->app->config();
        Aether::ListOption * opt;

        // Advanced::auto_launch_service
        this->addToggle("Settings.AppAdvanced.AutoLaunchSysmodule"_lang, [cfg]() -> bool {
            return cfg->autoLaunchService();
        }, [cfg](bool b) {
            cfg->setAutoLaunchService(b);
        });
        this->addComment("Settings.AppAdvanced.AutoLaunchSysmoduleText"_lang);
        this->list->addElement(new Aether::ListSeparator());

        // Remove redundant images
        this->addButton("Settings.AppAdvanced.RemoveUnneededImages"_lang, [this]() {
            this->removeImages();
        });
        this->addComment("Settings.AppAdvanced.RemoveUnneededImagesText"_lang);
        this->list->addElement(new Aether::ListSeparator());

        // Advanced::set_queue_max
        opt = new Aether::ListOption("Settings.AppAdvanced.InitialQueueSize"_lang, std::to_string(cfg->setQueueMax()), nullptr);
        opt->setCallback([this, cfg, opt]() {
            int val = cfg->setQueueMax();
            if (this->getNumberInput(val, "Settings.AppAdvanced.InitialQueueSize"_lang, "", true)) {
                val = (val < -1 ? -1 : (val > 65535 ? 65535 : val));
                if (cfg->setSetQueueMax(val)) {
                    opt->setValue(std::to_string(val));
                    this->app->sysmodule()->setQueueLimit(val);
                }
            }
        });
        opt->setColours(this->app->theme()->muted2(), this->app->theme()->FG(), this->app->theme()->accent());
        this->list->addElement(opt);
        this->addComment("Settings.AppAdvanced.InitialQueueSizeText"_lang);
        this->list->addElement(new Aether::ListSeparator());

        // Advanced::search_max_phrases
        opt = new Aether::ListOption("Settings.AppAdvanced.MaximumSearchPhrases"_lang, std::to_string(cfg->searchMaxPhrases()), nullptr);
        opt->setCallback([this, cfg, opt]() {
            int val = cfg->searchMaxPhrases();
            if (this->getNumberInput(val, "Settings.AppAdvanced.MaximumSearchPhrases"_lang, "", false)) {
                val = (val < 1 ? 1 : val);
                if (cfg->setSearchMaxPhrases(val)) {
                    opt->setValue(std::to_string(val));
                    this->app->database()->setSearchPhraseCount(val);
                }
            }
        });
        opt->setColours(this->app->theme()->muted2(), this->app->theme()->FG(), this->app->theme()->accent());
        this->list->addElement(opt);
        this->addComment("Settings.AppAdvanced.MaximumSearchPhrasesText"_lang);

        // Advanced::search_max_score
        opt = new Aether::ListOption("Settings.AppAdvanced.MaximumSearchScore"_lang, std::to_string(cfg->searchMaxScore()), nullptr);
        opt->setCallback([this, cfg, opt]() {
            int val = cfg->searchMaxScore();
            if (this->getNumberInput(val, "Settings.AppAdvanced.MaximumSearchScore"_lang, "", false)) {
                val = (val < 30 ? 30 : val);
                if (cfg->setSearchMaxScore(val)) {
                    opt->setValue(std::to_string(val));
                    this->app->database()->setSpellfixScore(val);
                }
            }
        });
        opt->setColours(this->app->theme()->muted2(), this->app->theme()->FG(), this->app->theme()->accent());
        this->list->addElement(opt);
        this->addComment("Settings.AppAdvanced.MaximumSearchScoreText"_lang);
    }

    void AppAdvanced::removeImages() {
        // Get list of all images referenced in database (returned in order)
        bool ok;
        std::vector<std::string> dbFiles = this->app->database()->getAllImagePaths(ok);
        if (!ok) {
            return;
        }

        // Get list of all files in folder
        std::vector<std::string> folderFiles;
        for (auto & entry: std::filesystem::recursive_directory_iterator("/switch/TriPlayer/images/")) {
            if (entry.is_directory()) {
                continue;
            }

            folderFiles.push_back(entry.path());
        }
        std::sort(folderFiles.begin(), folderFiles.end());

        // Remove from folder where not in DB
        for (size_t i = 0; i < folderFiles.size(); i++) {
            bool inDB = std::binary_search(dbFiles.begin(), dbFiles.end(), folderFiles[i]);
            if (!inDB) {
                Utils::Fs::deleteFile(folderFiles[i]);
            }
        }
    }
};