#include <cstring>
#include <future>
#include "Log.hpp"
#include "nx/File.hpp"
#include "nx/NX.hpp"
#include <switch.h>

namespace NX {
    // Inherit proper structs for forward declared ones
    struct File::FFile : public FsFile {};
    struct File::FFileSystem : public FsFileSystem {};

    File::FFileSystem * File::filesystem = nullptr;             // FsFileSystem object used to open FsFiles with
    size_t File::fileID = 0;                                    // ID of the next file
    constexpr size_t readBufferDelay = 10;                      // Number of milliseconds between checking if buffer needs topping up
    constexpr size_t readBufferSize = 0x19000;                  // Size of read buffer (100kB)
    constexpr size_t readBufferThreshold = readBufferSize/2;    // Read in new data when the buffer hits this level

    File::File(const std::string & path) {
        // Initialize variables in case error occurrs
        this->buffer = nullptr;
        this->error = true;

        // Check the fs is ready
        if (this->filesystem == nullptr) {
            Log::writeError("[FS] Couldn't open file as fs not initialized!");
            return;
        }
        // Try to open file and set nullptr if unable to
        this->file = new FFile;
        Result rc = fsFsOpenFile(this->filesystem, path.c_str(), FsOpenMode_Read, this->file);
        if (R_FAILED(rc)) {
            Log::writeError("[FS] Failed to open file: " + path);
            delete file;
            file = nullptr;
            return;
        }

        // Get file size in order to seek
        rc = fsFileGetSize(this->file, &this->size);
        if (R_FAILED(rc)) {
            Log::writeError("[FS] Couldn't get file size for: " + path);
            delete file;
            file = nullptr;
            return;
        }

        // Properly initialize variables and buffer
        this->buffer = new uint8_t[readBufferSize];
        this->bufferHead = 0;
        this->bufferTail = 0;
        this->error = false;
        this->fileOffset = 0;
        this->id = this->fileID++;
        this->offset = 0;
        this->stopThread = false;

        // Start fill thread
        Thread::create("file" + std::to_string(this->id), fillBufferThread, this, 0x4000);
    }

    // Note: We can freely _read_ this->bufferTail in this thread (producer)
    void File::fillBufferThread(void * arg) {
        File * file = static_cast<File *>(arg);

        // Loop until we're signalled to exit
        while (!file->stopThread) {
            // Check if the buffer is getting low (subtract one as it's reserved)
            size_t emptyBytes = (readBufferSize - 1) - file->bufferSize();
            if (emptyBytes >= readBufferThreshold) {
                // Also check if we're not at the end
                std::scoped_lock<std::mutex> mtx(file->fileMutex);
                if (!(file->fileOffset >= file->size)) {
                    // Read from file into buffer
                    Result rc;
                    uint64_t actualRead = 0;


                    // If we have to wrap around then read in two goes
                    if (file->bufferTail + emptyBytes > readBufferSize) {
                        size_t firstPart = readBufferSize - file->bufferTail;
                        uint64_t read = 0;

                        rc = fsFileRead(file->file, file->fileOffset, file->buffer + file->bufferTail, firstPart, FsReadOption_None, &read);
                        actualRead += read;

                        // Stop if we're at the end
                        if (read == firstPart) {
                            rc = fsFileRead(file->file, file->fileOffset + read, file->buffer, emptyBytes - firstPart, FsReadOption_None, &read);
                            actualRead += read;
                        }

                    // Otherwise just read as normal
                    } else {
                        rc = fsFileRead(file->file, file->fileOffset, file->buffer + file->bufferTail, emptyBytes, FsReadOption_None, &actualRead);
                    }

                    // If an error occurred break out of loop
                    if (R_FAILED(rc)) {
                        Log::writeError("[FS] I/O error when reading file: " + std::to_string(rc));
                        file->error = true;
                        break;
                    }

                    // Lock mutex so we can safely update the end of the buffer
                    file->fileOffset += actualRead;
                    std::scoped_lock<std::mutex> mtx2(file->bufferMutex);
                    file->bufferTail += actualRead;
                    file->bufferTail = file->bufferTail % readBufferSize;
                }
            }

            // Pause before checking again
            Thread::sleepMilli(readBufferDelay);
        }
    }

    size_t File::bufferSize() {
        std::scoped_lock<std::mutex> mtx(this->bufferMutex);
        if (this->bufferHead > this->bufferTail) {
            return (readBufferSize - (this->bufferHead - this->bufferTail));
        } else {
            return this->bufferTail - this->bufferHead;
        }
    }

    size_t File::copyToBuffer(void * outBuffer, const size_t count) {
        // If we have to wrap around then read in two goes
        if (this->bufferHead + count > readBufferSize) {
            size_t firstPart = readBufferSize - this->bufferHead;
            std::memcpy(outBuffer, this->buffer + this->bufferHead, firstPart);
            std::memcpy(static_cast<uint8_t *>(outBuffer) + firstPart, this->buffer, count - firstPart);

        } else {
            std::memcpy(outBuffer, this->buffer + this->bufferHead, count);
        }

        // Move start index
        this->bufferHead += count;
        this->bufferHead = this->bufferHead % readBufferSize;
        this->offset += count;

        return count;
    }

    // Note: We can freely access this->bufferHead in this thread (consumer)
    ssize_t File::read(void * outBuffer, const size_t count) {
        // Edge case
        if (count == 0) {
            return 0;
        }

        // Check we haven't already encountered an error
        if (this->error) {
            return -1;
        }

        // Check how many bytes we have in the buffer
        size_t size = this->bufferSize();

        // Simply copy and return if we have enough
        if (size >= count) {
            return this->copyToBuffer(outBuffer, count);
        }

        // Otherwise wait until we have more bytes or reach EOF
        while (size < count) {
            // Stop if an I/O error is reported
            if (this->error) {
                return -1;
            }

            // Return remaining bytes if EOF
            std::unique_lock<std::mutex> mtx(this->fileMutex);
            if (this->fileOffset >= this->size) {
                return this->copyToBuffer(outBuffer, this->bufferSize());
            }
            mtx.unlock();

            // Return what we have if requested more than we can buffer
            if (count >= readBufferSize && size > 0) {
                return this->copyToBuffer(outBuffer, size);
            }

            // Sleep for a while before checking again
            Thread::sleepMilli(readBufferDelay);

            // Check buffer size again
            size = this->bufferSize();
        }

        // Copy as usual now that we have enough bytes
        return this->copyToBuffer(outBuffer, count);
    }

    off_t File::seek(const off_t offset, const Position position) {
        // Return if an error has occurred
        if (this->error) {
            return -1;
        }

        // Seek in file first (we need to lock for the whole operation)
        std::scoped_lock<std::mutex> mtx(this->fileMutex);
        switch (position) {
            case Position::Start:
                this->fileOffset = offset;
                break;

            case Position::Current:
                this->fileOffset = this->offset + offset;
                break;

            case Position::End:
                this->fileOffset = this->size + offset;
                break;
        }

        // Purge buffer, which will cause a read operation as soon as this function returns
        // This is safe as read() and seek() are called in a single-threaded context,
        // so we can assume that this->bufferTail won't be accessed
        std::scoped_lock<std::mutex> mtx2(this->bufferMutex);
        this->bufferHead = 0;
        this->bufferTail = 0;
        this->offset = this->fileOffset;

        return this->offset;
    }

    File::~File() {
        // Join fill thread
        this->stopThread = true;
        Thread::join("file" + std::to_string(this->id));

        // Close file and free buffer
        if (this->file != nullptr) {
            fsFileClose(this->file);
        }
        delete this->buffer;
    }

    bool File::initializeService() {
        // Prevent opening twice
        if (File::filesystem != nullptr) {
            return true;
        }

        // Open SD Card filesystem
        File::filesystem = new FFileSystem;
        Result rc = fsOpenSdCardFileSystem(File::filesystem);
        if (R_FAILED(rc)) {
            Log::writeError("[FS] Couldn't open SD Card fs");
            delete File::filesystem;
            File::filesystem = nullptr;
            return false;
        }

        return true;
    }

    void File::closeService() {
        delete File::filesystem;
        File::filesystem = nullptr;
    }


    ssize_t File::readFile(void * file, void * buffer, size_t count) {
        return static_cast<File *>(file)->read(buffer, count);
    }

    off_t File::seekFile(void * file, const off_t offset, const int type) {
        Position position;
        switch (type) {
            case SEEK_SET:
                position = Position::Start;
                break;

            case SEEK_CUR:
                position = Position::Current;
                break;

            case SEEK_END:
                position = Position::End;
                break;

            // If we can't handle the given type return -1 as it's an error
            default:
                Log::writeError("[FS] Unknown seek type");
                return -1;
                break;
        }

        return static_cast<File *>(file)->seek(offset, position);
    }
};