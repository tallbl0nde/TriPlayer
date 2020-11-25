#include "Log.hpp"
#include "meta/AudioDB.hpp"
#include "meta/Metadata.hpp"
#include "utils/FS.hpp"

// Taglib headers
#include <attachedpictureframe.h>
#include <flacfile.h>
#include <id3v1tag.h>
#include <id3v2tag.h>
#include <mpegfile.h>
#include <xiphcomment.h>
#include <wavfile.h>

namespace Metadata {
    DownloadResult downloadAlbumImage(const std::string & name, std::vector<unsigned char> & data, int & id) {
        // First search for album
        AudioDB::Entry entry = AudioDB::getAlbumInfo(name);
        if (entry.tadbID > 0) {
            id = entry.tadbID;
            // Now check if we have a URL
            if (entry.imageURL.empty()) {
                return DownloadResult::NoImage;
            }

            // Download image
            data = AudioDB::getEntryImage(entry);
            if (data.size() > 0) {
                return DownloadResult::Success;
            }

        } else if (entry.tadbID == -1) {
            // Parse error
            return DownloadResult::NotFound;
        }

        return DownloadResult::Error;
    }

    DownloadResult downloadArtistImage(const std::string & name, std::vector<unsigned char> & data, int & id) {
        // First search for artist
        AudioDB::Entry entry = AudioDB::getArtistInfo(name);
        if (entry.tadbID > 0) {
            id = entry.tadbID;
            // Now check if we have a URL
            if (entry.imageURL.empty()) {
                return DownloadResult::NoImage;
            }

            // Download image
            data = AudioDB::getEntryImage(entry);
            if (data.size() > 0) {
                return DownloadResult::Success;
            }

        } else if (entry.tadbID == -1) {
            // Parse error
            return DownloadResult::NotFound;
        }

        return DownloadResult::Error;
    }

    // Fill any blank values in the passed metadata with default values
    static void fillMissingValues(const std::string & path, Song & m) {
        // Default title to file name
        if (m.title.empty()) {
            m.title = Utils::Fs::getStem(path);
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

    // Returns a "blank" Song
    static Song getBlankMetadata() {
        Song m;
        m.ID = -3;
        m.title = "";
        m.artist = "";
        m.album = "";
        m.trackNumber = -1;
        m.discNumber = -1;
        m.duration = -1;
        m.plays = 0;
        m.favourite = false;
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
                    TagLib::ByteVector tmp = pic->data();
                    v.assign(tmp.begin(), tmp.end());
                    break;
                }
            }
        }
    }

    // Parse metadata stored in ID3v1 tags, only replacing empty values
    static void parseID3v1Tags(TagLib::ID3v1::Tag * tag, Song & m) {
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
                    TagLib::ID3v2::AttachedPictureFrame::Type type = image->type();
                    if (type == TagLib::ID3v2::AttachedPictureFrame::Type::FrontCover || type == TagLib::ID3v2::AttachedPictureFrame::Type::Other || type == TagLib::ID3v2::AttachedPictureFrame::Illustration) {
                        TagLib::ByteVector pic = image->picture();
                        v.assign(pic.begin(), pic.end());
                        break;
                    }
                }
            }
        }
    }

    // Parse metadata stored in ID3v2 tags, only replacing empty values
    static void parseID3v2Tags(TagLib::ID3v2::Tag * tag, Song & m) {
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

    // Parse metadata stored in a RIFF Info tag, only replacing empty values
    static void parseRIFFInfoTag(TagLib::RIFF::Info::Tag * info, Song & m) {
        if (m.title.empty() && !info->title().isEmpty()) {
            m.title = info->title().to8Bit(true);
        }

        if (m.artist.empty() && !info->artist().isEmpty()) {
            m.artist = info->artist().to8Bit(true);
        }

        if (m.album.empty() && !info->album().isEmpty()) {
            m.album = info->album().to8Bit(true);
        }

        if (m.trackNumber < 0 && info->track() != 0) {
            m.trackNumber = info->track();
        }

        // Note that RIFF Info chunks don't appear to store disc number
    }

    // Parse metadata stored in a XiphComment, only replacing empty values
    static void parseXiphComment(TagLib::Ogg::XiphComment * xiph, Song & m) {
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

    // Treats given file as FLAC and extracts art
    static std::vector<unsigned char> readArtFromFLAC(const std::string & path) {
        // Create empty vector to return
        std::vector<unsigned char> v;

        // Open the file
        TagLib::FLAC::File file(path.c_str(), false);
        if (!file.isValid()) {
            Log::writeError("[META] [FLAC] Unable to extract art from: " + path);
            return v;
        }

        // Check for image attached directly first
        parseFLACArt(file.pictureList(), v);

        // Then check Vorbis comment
        if (v.empty() && file.hasXiphComment()) {
            TagLib::Ogg::XiphComment * xiph = file.xiphComment();
            if (xiph != nullptr) {
                parseFLACArt(xiph->pictureList(), v);
            }
        }

        // And finally check ID3v2 tags
        if (v.empty() && file.hasID3v2Tag()) {
            TagLib::ID3v2::Tag * tag = file.ID3v2Tag();
            if (tag != nullptr) {
                parseID3v2Art(tag, v);
            }
        }

        if (v.empty()) {
            Log::writeWarning("[META] [FLAC] Couldn't find art in: " + path);
        }
        return v;
    }

    // Treats given file as MP3 and extracts art
    static std::vector<unsigned char> readArtFromMP3(const std::string & path) {
        // Create empty vector to return
        std::vector<unsigned char> v;

        // Open the file
        TagLib::MPEG::File file(path.c_str(), false);
        if (!file.isValid()) {
            Log::writeError("[META] [MP3] Unable to extract art from: " + path);
            return v;
        }

        // Check ID3v2 tags as they're the only thing that can contain an image
        if (file.hasID3v2Tag()) {
            TagLib::ID3v2::Tag * tag = file.ID3v2Tag();
            if (tag != nullptr) {
                parseID3v2Art(tag, v);
            }
        }

        if (v.empty()) {
            Log::writeWarning("[META] [MP3] Couldn't find art in: " + path);
        }
        return v;
    }

    // Treats given file as WAV and extracts art
    static std::vector<unsigned char> readArtFromWAV(const std::string & path) {
        // Create empty vector to return
        std::vector<unsigned char> v;

        // Open the file
        TagLib::RIFF::WAV::File file(path.c_str(), false);
        if (!file.isValid()) {
            Log::writeError("[META] [WAV] Unable to extract art from: " + path);
            return v;
        }

        // Check ID3v2 tags as they're the only thing that can contain an image
        if (file.hasID3v2Tag()) {
            TagLib::ID3v2::Tag * tag = file.ID3v2Tag();
            if (tag != nullptr) {
                parseID3v2Art(tag, v);
            }
        }

        if (v.empty()) {
            Log::writeWarning("[META] [WAV] Couldn't find art in: " + path);
        }
        return v;
    }

    std::vector<unsigned char> readArtFromFile(const std::string & path, const AudioFormat format) {
        // Call relevant function
        switch (format) {
            case AudioFormat::FLAC:
                return readArtFromFLAC(path);
                break;

            case AudioFormat::MP3:
                return readArtFromMP3(path);
                break;

            case AudioFormat::WAV:
                return readArtFromWAV(path);

            default:
                return std::vector<unsigned char>();
        }
    }

    // Treats given file as FLAC and extracts relevant metadata
    static Song readFromFLAC(const std::string & path) {
        // Initialize blank object first
        Song m = getBlankMetadata();
        m.format = AudioFormat::FLAC;
        m.path = path;

        // Open the file
        TagLib::FLAC::File file(path.c_str(), true, TagLib::AudioProperties::Accurate);
        if (!file.isValid()) {
            Log::writeError("[META] [FLAC] Unable to process file: " + path);
            m.ID = -3;
            return m;
        }

        // Read duration
        TagLib::FLAC::Properties * properties = file.audioProperties();
        if (properties != nullptr) {
            m.duration = static_cast<unsigned int>(properties->lengthInSeconds());
        } else {
            Log::writeWarning("[META] [FLAC] Couldn't read duration of file: " + path);
        }

        // Check if it has metadata stored in Vorbis format first
        if (file.hasXiphComment()) {
            TagLib::Ogg::XiphComment * xiph = file.xiphComment();
            if (xiph != nullptr) {
                parseXiphComment(xiph, m);
            }

        } else {
            Log::writeInfo("[META] [FLAC] No XiphComment found in: " + path);
        }

        // Check ID3v2 next
        if (file.hasID3v2Tag()) {
            TagLib::ID3v2::Tag * tag = file.ID3v2Tag();
            if (tag != nullptr) {
                parseID3v2Tags(tag, m);
            }

        } else {
            Log::writeInfo("[META] [FLAC] No ID3v2 tags found in: " + path);
        }

        // Finally check ID3v1
        if (file.hasID3v1Tag()) {
            TagLib::ID3v1::Tag * tag = file.ID3v1Tag();
            if (tag != nullptr) {
                parseID3v1Tags(tag, m);
            }

        } else {
            Log::writeInfo("[META] [FLAC] No ID3v1 tags found in: " + path);
        }

        // Fill in missing values with default values (does nothing if all filled)
        m.ID = -1;
        fillMissingValues(path, m);
        return m;
    }

    // Treats given file as MP3 and extracts relevant metadata
    static Song readFromMP3(const std::string & path) {
        // Initialize blank object first
        Song m = getBlankMetadata();
        m.format = AudioFormat::MP3;
        m.path = path;

        // Open the file
        TagLib::MPEG::File file(path.c_str(), true, TagLib::AudioProperties::Accurate);
        if (!file.isValid()) {
            Log::writeError("[META] [MP3] Unable to process file: " + path);
            m.ID = -3;
            return m;
        }

        // Read duration
        TagLib::MPEG::Properties * properties = file.audioProperties();
        if (properties != nullptr) {
            m.duration = static_cast<unsigned int>(properties->lengthInSeconds());
        } else {
            Log::writeWarning("[META] [MP3] Couldn't read duration of file: " + path);
        }

        // Check ID3v2 next
        if (file.hasID3v2Tag()) {
            TagLib::ID3v2::Tag * tag = file.ID3v2Tag();
            if (tag != nullptr) {
                parseID3v2Tags(tag, m);
            }

        } else {
            Log::writeInfo("[META] [MP3] No ID3v2 tags found in: " + path);
        }

        // Finally check ID3v1
        if (file.hasID3v1Tag()) {
            TagLib::ID3v1::Tag * tag = file.ID3v1Tag();
            if (tag != nullptr) {
                parseID3v1Tags(tag, m);
            }

        } else {
            Log::writeInfo("[META] [MP3] No ID3v1 tags found in: " + path);
        }

        // Fill in missing values with default values (does nothing if all filled)
        m.ID = -1;
        fillMissingValues(path, m);
        return m;
    }

    // Treats given file as WAV and extracts relevant metadata
    static Song readFromWAV(const std::string & path) {
        // Initialize blank object first
        Song m = getBlankMetadata();
        m.format = AudioFormat::WAV;
        m.path = path;

        // Open the file
        TagLib::RIFF::WAV::File file(path.c_str(), true, TagLib::AudioProperties::Accurate);
        if (!file.isValid()) {
            Log::writeError("[META] [WAV] Unable to process file: " + path);
            m.ID = -3;
            return m;
        }

        // Read duration
        TagLib::RIFF::WAV::Properties * properties = file.audioProperties();
        if (properties != nullptr) {
            m.duration = static_cast<unsigned int>(properties->lengthInSeconds());
        } else {
            Log::writeWarning("[META] [WAV] Couldn't read duration of file: " + path);
        }

        // Check for RIFF metadata first
        if (file.hasInfoTag()) {
            TagLib::RIFF::Info::Tag * tag = file.InfoTag();
            if (tag != nullptr) {
                parseRIFFInfoTag(tag, m);
            }

        } else {
            Log::writeInfo("[META] [WAV] No INFO tag found in: " + path);
        }

        // Then check ID3v2
        if (file.hasID3v2Tag()) {
            TagLib::ID3v2::Tag * tag = file.ID3v2Tag();
            if (tag != nullptr) {
                parseID3v2Tags(tag, m);
            }

        } else {
            Log::writeInfo("[META] [WAV] No ID3v2 tags found in: " + path);
        }

        // Fill in missing values with default values (does nothing if all filled)
        m.ID = -1;
        fillMissingValues(path, m);
        return m;
    }

    Song readFromFile(const std::string & path, const AudioFormat format) {
        // Call relevant function
        switch (format) {
            case AudioFormat::FLAC:
                return readFromFLAC(path);
                break;

            case AudioFormat::MP3:
                return readFromMP3(path);
                break;

            case AudioFormat::WAV:
                return readFromWAV(path);
                break;

            default:
                return getBlankMetadata();
        }
    }
};