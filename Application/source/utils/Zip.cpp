#include "Log.hpp"
#include "utils/FS.hpp"
#include "utils/Zip.hpp"
#include <zzip/lib.h>

namespace Utils::Zip {
    bool extract(const std::string & zipFile, const std::string & outPath) {
        // Attempt to open zip file
        Log::writeInfo("[ZIP] Extracting to: " + outPath);
        ZZIP_DIR * zipDir = zzip_dir_open(zipFile.c_str(), nullptr);
        if (!zipDir) {
            Log::writeError("[ZIP] Couldn't open file: " + zipFile + " (" + std::string(zzip_strerror_of(zipDir)) + ")");
            return false;
        }

        // Create directory if needed
        if (!Utils::Fs::createPath(outPath)) {
            Log::writeError("[ZIP] Failed to create output directory: " + outPath);
            return false;
        }

        // If we opened it successfully, extract everything!
        ZZIP_DIRENT entry;
        bool ok = true;
        while (zzip_dir_read(zipDir, &entry)) {
            // Get zip-file string
            std::string path = std::string(entry.d_name);

            // If the zip-file is a directory (signified by "/" at the end), create it
            if (path[path.length() - 1] == '/') {
                Log::writeInfo("[ZIP] Ensuring directory exists: " + outPath + path);
                if (!Utils::Fs::createPath(outPath + path)) {
                    Log::writeError("[ZIP] Failed to create output directory: " + outPath + path);
                    ok = false;
                    break;
                }

            // Otherwise if it's a file, copy in 10MB chunks
            } else {
                // Open file for reading
                ZZIP_FILE * file = zzip_file_open(zipDir, path.c_str(), 0);
                if (!file) {
                    Log::writeError("[ZIP] Failed to open: " + path + " (" + std::string(zzip_strerror_of(zipDir)) + ")");
                    ok = false;
                    break;
                }

                // Delete original file
                Utils::Fs::deleteFile(outPath + path);

                // Read and write until entire file is processed
                std::vector<unsigned char> buf;
                zzip_size_t count = 0;
                buf.resize(10 * 1024 * 1024);
                while ((count = zzip_file_read(file, &buf[0], buf.size()))) {
                    buf.resize(count);
                    if (!Utils::Fs::appendFile(outPath + path, buf)) {
                        Log::writeError("[ZIP] Failed to extract: " + path);
                        zzip_file_close(file);
                        ok = false;
                        break;
                    }
                    buf.resize(10 * 1024 * 1024);
                }
                if (!ok) {
                    break;
                }
                zzip_file_close(file);

                Log::writeInfo("[ZIP] Processed file: " + path + ", compressed size: " + std::to_string(entry.d_csize) + ", decompressed size: " + std::to_string(entry.st_size));
            }
        }

        // Close object now that we're finished
        zzip_dir_close(zipDir);
        if (ok) {
            Log::writeSuccess("[ZIP] Extracted " + zipFile + " successfully!");
        }
        return ok;
    }
}