#include <filesystem>
#include "Log.hpp"
#include "utils/FLAC.hpp"

// Taglib headers
#include <attachedpictureframe.h>
#include <flacfile.h>
#include <id3v1tag.h>
#include <id3v2tag.h>
#include <xiphcomment.h>

namespace Utils::FLAC {
    // Fill any blank values in the passed metadata with default values
    static void fillMissingValues(const std::string & path, Metadata::Song & m) {
        // Default title to file name
        if (m.title.empty()) {
            m.title = std::filesystem::path(path).stem();
            m.ID = -2;
        }

        // Default artist and album to unknown
        if (m.artist.empty()) {
            m.artist = "Unknown Artist";
            m.ID = -2;
        }
        if (m.album.empty()) {
            m.album = "Unknown Album";
            m.ID = -2;
        }

        // Default disc and track numbers to zero
        if (m.discNumber < 0) {
            m.discNumber = 0;
            m.ID = -2;
        }
        if (m.duration < 0) {
            m.duration = 0;
            m.ID = -2;
        }
        if (m.trackNumber < 0) {
            m.trackNumber = 0;
            m.ID = -2;
        }
    }

    // Returns a "blank" Metadata::Song
    static Metadata::Song getBlankMetadata() {
        Metadata::Song m;
        m.ID = -3;
        m.title = "";
        m.artist = "";
        m.album = "";
        m.trackNumber = -1;
        m.discNumber = -1;
        m.duration = -1;
        return m;
    }

    // Return whether mime type is supported
    static inline bool mimeTypeSupported(const TagLib::String & mime) {
        const std::string str = mime.to8Bit(true);
        return (mime == "image/jpg" || mime == "image/jpeg" || mime == "image/png");
    }

    // Check for and get the first valid image stored in a list of pictures
    static void parseFLACArt(TagLib::List<TagLib::FLAC::Picture *> pics, std::vector<unsigned char> & v) {
        // Iterate over entire list until one is found
        for (TagLib::FLAC::Picture * pic : pics) {
            if (mimeTypeSupported(pic->mimeType())) {
                TagLib::FLAC::Picture::Type type = pic->type();
                if (type == TagLib::FLAC::Picture::Type::FrontCover || type == TagLib::FLAC::Picture::Type::Other || type == TagLib::FLAC::Picture::Illustration) {
                    v.assign(pic->data().begin(), pic->data().end());
                    break;
                }
            }
        }
    }

    // Parse metadata stored in ID3v1 tags, only replacing empty values
    static void parseID3v1Tags(TagLib::ID3v1::Tag * tag, Metadata::Song & m) {
        if (m.title.empty() && !tag->title().isEmpty()) {
            m.title = tag->title().to8Bit(true);
        }

        if (m.artist.empty() && !tag->artist().isEmpty()) {
            m.artist = tag->artist().to8Bit(true);
        }

        if (m.album.empty() && !tag->album().isEmpty()) {
            m.album = tag->album().to8Bit(true);
        }

        if (m.trackNumber < 0 && tag->track() != 0) {
            m.trackNumber = tag->track();
        }
    }

    // Check for and get the first valid image stored in the given ID3v2 tags
    static void parseID3v2Art(TagLib::ID3v2::Tag * tag, std::vector<unsigned char> & v) {
        TagLib::ID3v2::FrameList frameList = tag->frameListMap()["APIC"];

        for (TagLib::ID3v2::Frame * frame : frameList) {
            TagLib::ID3v2::AttachedPictureFrame * image = static_cast<TagLib::ID3v2::AttachedPictureFrame *>(frame);
            if (image) {
                if (mimeTypeSupported(image->mimeType())) {
                    v.assign(image->picture().begin(), image->picture().end());
                    break;
                }
            }
        }
    }

    // Parse metadata stored in ID3v2 tags, only replacing empty values
    static void parseID3v2Tags(TagLib::ID3v2::Tag * tag, Metadata::Song & m) {
        if (m.title.empty() && !tag->title().isEmpty()) {
            m.title = tag->title().to8Bit(true);
        }

        if (m.artist.empty() && !tag->artist().isEmpty()) {
            m.artist = tag->artist().to8Bit(true);
        }

        if (m.album.empty() && !tag->album().isEmpty()) {
            m.album = tag->album().to8Bit(true);
        }

        if (m.trackNumber < 0 && tag->track() != 0) {
            m.trackNumber = tag->track();
        }

        // Stop if we have a disc number already
        if (m.discNumber >= 0) {
            return;
        }

        // Check for TPOS frame in order to get disc number
        TagLib::ID3v2::FrameList discNumberList = tag->frameListMap()["TPOS"];
        if (!discNumberList.isEmpty()) {
            TagLib::String discNumber = discNumberList.front()->toString();
            if (!discNumber.isEmpty()) {
                // Note: This correctly handles 1/2, etc...
                m.discNumber = discNumber.toInt();
            }
        }
    }

    // Parse metadata stored in a XiphComment, only replacing empty values
    static void parseXiphComment(TagLib::Ogg::XiphComment * xiph, Metadata::Song & m) {
        if (m.title.empty() && !xiph->title().isEmpty()) {
            m.title = xiph->title().to8Bit(true);
        }

        if (m.artist.empty() && !xiph->artist().isEmpty()) {
            m.artist = xiph->artist().to8Bit(true);
        }

        if (m.album.empty() && !xiph->album().isEmpty()) {
            m.album = xiph->album().to8Bit(true);
        }

        if (m.trackNumber < 0 && xiph->track() != 0) {
            m.trackNumber = xiph->track();
        }

        // Stop if we have a disc number already
        if (m.discNumber >= 0) {
            return;
        }

        // Check for DISCNUMBER field name in order to get disc number
        TagLib::StringList discNumberList = xiph->fieldListMap()["DISCNUMBER"];
        if (!discNumberList.isEmpty()) {
            TagLib::String discNumber = discNumberList.front();
            if (!discNumber.isEmpty()) {
                // Note: This correctly handles 1/2, etc...
                m.discNumber = discNumber.toInt();
            }
        }
    }

    std::vector<unsigned char> getArt(std::string path) {
        // Create empty vector
        std::vector<unsigned char> v;

        // Open the file
        TagLib::FLAC::File audioFile(path.c_str(), false);
        if (!audioFile.isValid()) {
            Log::writeError("[FLAC] Unable to extract art from: " + path);
            return v;
        }

        // Check for image attached directly first
        parseFLACArt(audioFile.pictureList(), v);

        // Then check Vorbis comment
        if (v.empty() && audioFile.hasXiphComment()) {
            TagLib::Ogg::XiphComment * xiph = audioFile.xiphComment();
            if (xiph != nullptr) {
                parseFLACArt(xiph->pictureList(), v);
            }
        }

        // And finally check ID3v2 tags
        if (v.empty() && audioFile.hasID3v2Tag()) {
            TagLib::ID3v2::Tag * tag = audioFile.ID3v2Tag();
            if (tag != nullptr) {
                parseID3v2Art(tag, v);
            }
        }

        if (v.empty()) {
            Log::writeWarning("[FLAC] Couldn't find art in: " + path);
        }
        return v;
    }

    Metadata::Song getInfo(std::string path) {
        // Initialize blank object first
        Metadata::Song m = getBlankMetadata();
        m.format = AudioFormat::FLAC;
        m.path = path;

        // Open the file
        TagLib::FLAC::File audioFile(path.c_str(), true, TagLib::AudioProperties::Average); // Change to accurate and see speed?
        if (!audioFile.isValid()) {
            Log::writeError("[FLAC] Unable to process file: " + path);
            m.ID = -3;
            return m;
        }

        // Read duration
        TagLib::FLAC::Properties * properties = audioFile.audioProperties();
        if (properties != nullptr) {
            m.duration = static_cast<unsigned int>(properties->lengthInSeconds());
        } else {
            Log::writeWarning("[FLAC] Couldn't read duration of file: " + path);
        }

        // Check if it has metadata stored in Vorbis format first
        if (audioFile.hasXiphComment()) {
            TagLib::Ogg::XiphComment * xiph = audioFile.xiphComment();
            if (xiph != nullptr) {
                parseXiphComment(xiph, m);
            }

        } else {
            Log::writeInfo("[FLAC] No XiphComment found in: " + path);
        }

        // Check ID3v2 next
        if (audioFile.hasID3v2Tag()) {
            TagLib::ID3v2::Tag * tag = audioFile.ID3v2Tag();
            if (tag != nullptr) {
                parseID3v2Tags(tag, m);
            }

        } else {
            Log::writeInfo("[FLAC] No ID3v2 tags found in: " + path);
        }

        // Finally check ID3v1
        if (audioFile.hasID3v1Tag()) {
            TagLib::ID3v1::Tag * tag = audioFile.ID3v1Tag();
            if (tag != nullptr) {
                parseID3v1Tags(tag, m);
            }

        } else {
            Log::writeInfo("[FLAC] No ID3v1 tags found in: " + path);
        }

        // Fill in missing values with default values (does nothing if all filled)
        m.ID = -1;
        fillMissingValues(path, m);
        return m;
    }
};