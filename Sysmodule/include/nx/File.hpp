#ifndef NX_FILE_HPP
#define NX_FILE_HPP

#include <atomic>
#include <cstddef>
#include <mutex>
#include <string>

// The File class represents a file on the SD Card. It uses libnx's fs* calls
// behind the scenes to actually read from the file. The file is opened using
// the constructor and is closed when the object is deleted. It also handles
// a read buffer behind the scenes in order to still be able to "read" when
// the SD Card is under heavy load.
namespace NX {
    class File {
        public:
            // Position offset is relative to when seeking
            enum class Position {
                Start,          // Start of file
                Current,        // Current position in file
                End             // End of file
            };

        private:
            // Forward declare file structures
            struct FFile;
            struct FFileSystem;

            // Read bytes into the file's buffer (below)
            // Run on a separate thread when the buffer is below a threshold
            static void fillBufferThread(void *);
            std::mutex bufferMutex;                 // Mutex protecting access to the buffer
            size_t id;                              // This file's thread ID
            std::atomic<bool> stopThread;           // Set true to exit the thread

            // Returns size of buffer
            size_t bufferSize();
            // Copy from read buffer into given buffer (handles wrapping around)
            size_t copyToBuffer(void *, const size_t);
            uint8_t * buffer;                       // Buffer of read data (circular buffer)
            std::atomic<size_t> bufferHead;         // Index marking 'front' of buffer
            std::atomic<size_t> bufferTail;         // Index marking 'back' of buffer
            off_t offset;                           // Relative file offset

            std::atomic<bool> error;                // Set true if an fs error occurred
            FFile * file;                           // File object
            std::mutex fileMutex;                   // Mutex protecting access to file metadata
            off_t fileOffset;                       // Current offset in file
            int64_t size;                           // Size of file in bytes

            static FFileSystem * filesystem;        // Filesystem to read files from
            static size_t fileID;                   // 'ID' of next file (used for identifying thread)

        public:
            // Constructor attempts to open file and create buffer
            File(const std::string &);

            // Read (copy) the requested number of bytes into the given buffer
            // Returns -1 on an error
            ssize_t read(void *, const size_t);

            // Seek to the given position in the file using the given relative position
            // Returns -1 on an error
            off_t seek(const off_t, const Position);

            // Destructor closes file handle
            ~File();

            // Initializes the required services
            static bool initializeService();
            // Closes the initialized services
            static void closeService();

            // Helper functions to operate on the provided file object using read() and
            // lseek() like function structure (as required by mpg123)
            static ssize_t readFile(void *, void *, const size_t);
            static off_t seekFile(void *, const off_t, const int);
    };
};

#endif