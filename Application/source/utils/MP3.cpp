#include <filesystem>
#include "Log.hpp"
#include "utils/MP3.hpp"

// TagLib
#include <mpegfile.h>
#include <id3v2tag.h>
#include <id3v1tag.h>
#include <attachedpictureframe.h>

namespace Utils::MP3 {
    // Searches and returns an appropriate image
    std::vector<unsigned char> getArtFromID3(std::string path) {
        std::vector<unsigned char> v;

        // Open the file to read the metadata
        TagLib::MPEG::File audioFile(path.c_str(), false);
        if (audioFile.isValid()) {
            // Read the ID3v2 tag
            // NOTE: ID3v1 does not support album art
            if (audioFile.hasID3v2Tag()) {
                TagLib::ID3v2::Tag * tag = audioFile.ID3v2Tag();
                if (tag) {
                    // Extract the cover art tag frame
                    TagLib::ID3v2::FrameList coverFrameList = tag->frameListMap()["APIC"];
                    if (!coverFrameList.isEmpty()) {
                        TagLib::ID3v2::AttachedPictureFrame * coverFrame = (TagLib::ID3v2::AttachedPictureFrame *)coverFrameList.front();
                        if (coverFrame) {
                            // Check the image format and extract it if supported
                            TagLib::String mType = coverFrame->mimeType();
                            if (mType == "image/jpg" || mType == "image/jpeg" || mType == "image/png") {
                                TagLib::ByteVector byteVector = coverFrame->picture();
                                v.assign(byteVector.begin(), byteVector.end());
                            } else {
                                Log::writeInfo("[MP3] No suitable art found in: " + path);
                            }
                        } else {
                            Log::writeInfo("[MP3] No suitable art found in: " + path);
                        }
                    } else {
                        Log::writeInfo("[MP3] No suitable art found in: " + path);
                    }
                } else {
                    Log::writeError("[MP3] Failed to parse metadata for: " + path);
                }
            } else {
                Log::writeWarning("[MP3] No ID3v2 tags were found in: " + path);
            }
        } else {
            Log::writeError("[MP3] Unable to open file: " + path);
        }

        return v;
    }

    // Checks for tag type and reads the metadata
    Metadata::Song getInfoFromID3(std::string path) {
        // Default info to return
        Metadata::Song m;
        m.format = AudioFormat::MP3;
        m.ID = -3;
        m.title = std::filesystem::path(path).stem();      // Title defaults to file name
        m.artist = "Unknown Artist";                       // Artist defaults to unknown
        m.album = "Unknown Album";                         // Same for album
        m.trackNumber = 0;                                 // Initially 0 to indicate not set
        m.discNumber = 0;                                  // Initially 0 to indicate not set
        m.duration = 0;                                    // Initially 0 to indicate not set

        // Open the file to read the tags
        TagLib::MPEG::File audioFile(path.c_str(), true, TagLib::AudioProperties::Average);
        if (audioFile.isValid()) {
            // Read the audio properties to get the duration
            TagLib::MPEG::Properties * audioProperties = audioFile.audioProperties();
            if (audioProperties) {
                m.duration = (unsigned int)audioProperties->lengthInSeconds();
            } else {
                Log::writeError("[MP3] Failed to read audio properties of file: " + path);
            }

            // Confirm the tags exist and read them
            if (audioFile.hasID3v2Tag()) {
                TagLib::ID3v2::Tag * tag = audioFile.ID3v2Tag();
                if (tag) {
                    m.ID = -1;
                    TagLib::String title = tag->title();
                    if (!title.isEmpty()) m.title = title.to8Bit(true);
                    TagLib::String artist = tag->artist();
                    if (!artist.isEmpty()) m.artist = artist.to8Bit(true);
                    TagLib::String album = tag->album();
                    if (!album.isEmpty()) m.album = album.to8Bit(true);
                    m.trackNumber = tag->track();
                    TagLib::ID3v2::FrameListMap frameListMap = tag->frameListMap();
                    TagLib::ID3v2::FrameList discNumberList = frameListMap["TPOS"];
                    if (!discNumberList.isEmpty()) {
                        TagLib::String discNumber = discNumberList.front()->toString();
                        if (!discNumber.isEmpty()) {
                            // NOTE: This correctly handles cases where the format is "1/2" rather than "1".
                            //       In both cases, it will return 1.
                            m.discNumber = discNumber.toInt();
                        }
                    }
                } else {
                    m.ID = -2;
                    Log::writeWarning("[MP3] No tags were found in: " + path);
                }
            } else if (audioFile.hasID3v1Tag()) {
                // NOTE: ID3v1 does not have a disc number tag
                TagLib::ID3v1::Tag * tag = audioFile.ID3v1Tag();
                if (tag) {
                    m.ID = -1;
                    TagLib::String title = tag->title();
                    if (!title.isEmpty()) m.title = title.to8Bit(true);
                    TagLib::String artist = tag->artist();
                    if (!artist.isEmpty()) m.artist = artist.to8Bit(true);
                    TagLib::String album = tag->album();
                    if (!album.isEmpty()) m.album = album.to8Bit(true);
                    m.trackNumber = tag->track();
                } else {
                    m.ID = -2;
                    Log::writeWarning("[MP3] No tags were found in: " + path);
                }
            } else {
                m.ID = -2;
                Log::writeWarning("[MP3] No ID3 metadata present in: " + path);
            }
        } else {
            Log::writeError("[MP3] Unable to open file: " + path);
        }

        return m;
    }
};