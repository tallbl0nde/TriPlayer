#include "sqlite3.h"
#include <switch.h>

// Important points:
// - There is no file truncation
//   -> journal_mode=truncate won't work
// - There is no file locking
//   -> as far as I know Horizon doesn't support it?
// - Doesn't support temp files
// - Doesn't support dynamic libraries (not my fault)

// FsFileSystem used to interact with files on SD Card
static FsFileSystem fs;

// Size of write buffer (in bytes)
#define SQLITE_NXVFS_BUFFERSZ 8192

// sqlite3_file * actually points to this structure
typedef struct nxFile nxFile;
struct nxFile {
    sqlite3_file base;          // Base class
    FsFile file;                // NX (Horizon) file object
    char * buf;                 // Buffer for writes
    int bufSize;                // Number of bytes in buffer
    sqlite3_int64 bufOffset;    // Offset of bytes in buffer from buf[0]
};

// Close a file
static int nxClose(sqlite3_file * pFile) {
    nxFile * file = (nxFile *) pFile;
    fsFileClose(&file->file);
    return SQLITE_OK;
}

// Read data from a file
static int nxRead(sqlite3_file * pFile, void * buf, int bytes, sqlite_int64 offset) {
    // Bytes read and result code
    u64 read = 0;
    Result rc;

    // Read from file
    nxFile * file = (nxFile *) pFile;
    rc = fsFileRead(&file->file, offset, buf, bytes, FsReadOption_None, &read);

    // Return IO error if result isn't good
    if (R_FAILED(rc)) {
        return SQLITE_IOERR_READ;
    }

    // Check if we read the right amount of bytes
    if (read == bytes) {
        return SQLITE_OK;

    // Zero-pad the remaining buffer if not enough bytes were read
    } else if (read >= 0) {
        if (read < bytes) {
            memset(&((char *) buf)[read], 0, bytes-read);
        }
        return SQLITE_IOERR_SHORT_READ;
    }

    // Don't think this should be reached?
    return SQLITE_IOERR_READ;
}

// Write to a file (and flush immediately)
static int nxDirectWrite(nxFile * file, const void * buf, int bytes, sqlite_int64 offset) {
    Result rc = fsFileWrite(&file->file, offset, buf, bytes, FsWriteOption_Flush);

    // Return IO error if result is not good
    if (R_FAILED(rc)) {
        return SQLITE_IOERR_WRITE;
    }

    return SQLITE_OK;
}

// Flush file's buffer to disk (no-op if buffer is empty)
static int nxFlushBuffer(nxFile * file) {
    int rc = SQLITE_OK;
    if (file->buf) {
        rc = nxDirectWrite(file, file->buf, file->bufSize, file->bufOffset);
        file->buf = NULL;
    }
    return rc;
}

// Write to a file (without flushing)
static int nxWrite(sqlite3_file * pFile, const void * buf, int bytes, sqlite_int64 offset) {
    nxFile * file = (nxFile *) pFile;

    // If the buffer exists
    if (file->buf) {
        char * buf2 = (char *) buf;         // Pointer to remaining data in write buffer
        int bytes2 = bytes;                 // Remaining number of bytes in write buffer
        sqlite3_int64 offset2 = offset;     // File offset to write to

        // While there's still bytes to write
        while (bytes2 > 0) {
            int copy;       // Number of bytes to copy into file buffer

            // If the buffer is full or not being used - flush the buffer
            if (file->bufSize == SQLITE_NXVFS_BUFFERSZ || file->bufOffset + file->bufSize != offset2) {
                int rc = nxFlushBuffer(file);
                if (rc != SQLITE_OK) {
                    return rc;
                }
            }
            file->bufOffset = offset2 - file->bufSize;

            // Copy as much data as possible into the buffer
            copy = SQLITE_NXVFS_BUFFERSZ - file->bufSize;
            if (copy > bytes2) {
                copy = bytes2;
            }
            memcpy(&file->buf[file->bufSize], buf2, copy);
            file->bufSize += copy;

            // Update variables
            bytes2 -= copy;
            offset2 += copy;
            buf2 += copy;
        }

    // Otherwise if there's no buffer just write to file
    } else {
        return nxDirectWrite(file, buf, bytes, offset);
    }

    return SQLITE_OK;
}

// This is meant to truncate a file (maybe I'll get to it later)
// This means that journal_mode=truncate is not supported
static int nxTruncate(sqlite3_file * pFile, sqlite_int64 size) {
    return SQLITE_OK;
}

// Sync contents of file to the disk
static int nxSync(sqlite3_file * pFile, int flags) {
    nxFile * file = (nxFile *) pFile;

    // Flush buffer to disk
    int tmp = nxFlushBuffer(file);
    if (tmp != SQLITE_OK) {
        return tmp;
    }

    // Call system to flush it's cache
    Result rc = fsFileFlush(&file->file);
    return (R_SUCCEEDED(rc) ? SQLITE_OK : SQLITE_IOERR_FSYNC);
}

// Get the size of the file and write to pointer
static int nxFileSize(sqlite3_file * pFile, sqlite_int64 * size) {
    nxFile * file = (nxFile *) pFile;

    // Flush buffer to disk first
    int tmp = nxFlushBuffer(file);
    if (tmp != SQLITE_OK) {
        return tmp;
    }

    // Query using system call
    s64 sz;
    Result rc = fsFileGetSize(&file->file, &sz);
    if (R_FAILED(rc)) {
        return SQLITE_IOERR_FSTAT;
    }
    *(size) = sz;
    return SQLITE_OK;
}

// All locking functions do nothing
static int nxLock(sqlite3_file * pFile, int lock) {
    return SQLITE_OK;
}
static int nxUnlock(sqlite3_file * pFile, int lock) {
    return SQLITE_OK;
}
static int nxCheckReservedLock(sqlite3_file * pFile, int lock) {
    return SQLITE_OK;
}

// File control also does nothing
static int nxFileControl(sqlite3_file * pFile, int op, void * arg) {
    return SQLITE_NOTFOUND;
}

// Don't return any info about device
static int nxSectorSize(sqlite3_file * pFile) {
    return 0;
}
static int nxDeviceCharacteristics(sqlite3_file * pFile) {
    return 0;
}

// Open a file
static int nxOpen(sqlite3_vfs * vfs, const char * path, sqlite3_file * pFile, int flags, int * outFlags) {
    // Set file's IO methods to the ones above
    static const sqlite3_io_methods nxIO = {
        1,                          // iVersion
        nxClose,                    // xClose
        nxRead,                     // xRead
        nxWrite,                    // xWrite
        nxTruncate,                 // xTruncate
        nxSync,                     // xSync
        nxFileSize,                 // xFileSize
        nxLock,                     // xLock
        nxUnlock,                   // xUnlock
        nxCheckReservedLock,        // xCheckReservedLock
        nxFileControl,              // xFileControl
        nxSectorSize,               // xSectorSize
        nxDeviceCharacteristics     // xDeviceCharacteristics
    };

    nxFile * file = (nxFile *) pFile;
    Result rc;
    char * tmpBuf = NULL;           // Temporary pointer to potential file buffer

    // Don't support temp files
    if (path == NULL) {
        return SQLITE_IOERR;
    }

    // Create file buffer if it's a journal file
    if (flags & SQLITE_OPEN_MAIN_JOURNAL) {
        tmpBuf = (char *) sqlite3_malloc(SQLITE_NXVFS_BUFFERSZ);
        if (!tmpBuf) {
            return SQLITE_NOMEM;
        }
    }

    // Create file if flag is set
    if (flags & SQLITE_OPEN_CREATE) {
        rc = fsFsCreateFile(&fs, path, 0, 0);
    }

    // Choose mode based on flags
    u32 mode = 0;
    if (flags & SQLITE_OPEN_READONLY) {
        mode |= FsOpenMode_Read;
    } else if (flags & SQLITE_OPEN_READWRITE) {
        mode |= FsOpenMode_Read;
        mode |= FsOpenMode_Write;
    }

    // Allocate memory for file object and open
    memset(pFile, 0, sizeof(nxFile));
    rc = fsFsOpenFile(&fs, path, mode, &file->file);
    printf("%s: %i %i\n", path, R_MODULE(rc), R_DESCRIPTION(rc));
    if (R_FAILED(rc)) {
        file->base.pMethods = NULL;     // Prevents nxClose being called
        sqlite3_free(tmpBuf);
        return SQLITE_CANTOPEN;
    }
    file->buf = tmpBuf;

    // Set output flags
    if (outFlags) {
        *(outFlags) = flags;
    }
    file->base.pMethods = &nxIO;
    return SQLITE_OK;
}

// Delete the given file
static int nxDelete(sqlite3_vfs * vfs, const char * path, int sync) {
    Result rc = fsFsDeleteFile(&fs, path);

    // Commit changes if flag set
    if (R_SUCCEEDED(rc) && sync) {
        fsFsCommit(&fs);
    }

    return (R_SUCCEEDED(rc) ? SQLITE_OK : SQLITE_IOERR_DELETE);
}

// Check if the file exists
static int nxAccess(sqlite3_vfs * vfs, const char * path, int flags, int * out) {
    // Only check exists flag, fake the other ones
    if (flags & SQLITE_ACCESS_EXISTS) {
        FsDirEntryType type = FsDirEntryType_Dir;
        Result rc = fsFsGetEntryType(&fs, path, &type);
        if (R_FAILED(rc) || type != FsDirEntryType_File) {
            return SQLITE_IOERR_ACCESS;
        }
    }

    return SQLITE_OK;
}

// Simply returns the given path (should return full path though)
static int nxFullPathname(sqlite3_vfs * vfs, const char * path, int outBytes, char * outPath) {
    int num = strlen(path);
    if (outBytes > num) {
        num = outBytes;
    }
    memcpy(outPath, path, num);

    return SQLITE_OK;
}

// All dynamic library related functions do nothing due to no support
static void * nxDlOpen(sqlite3_vfs * vfs, const char * path){
  return NULL;
}
static void nxDlError(sqlite3_vfs * vfs, int bytes, char * err){
  sqlite3_snprintf(bytes, err, "Loadable extensions are not supported");
  err[bytes-1] = '\0';
}
static void (*nxDlSym(sqlite3_vfs * vfs, void * handle, const char * z))(void){
  return NULL;
}
static void nxDlClose(sqlite3_vfs * vfs, void * handle){
  return;
}

// Fill the provided buffer with pseudo-random bytes
static int nxRandomness(sqlite3_vfs * vfs, int bytes, char * buf) {
    randomGet((void *) buf, bytes);
    return SQLITE_OK;
}

// Sleep for the given number of microseconds
static int nxSleep(sqlite3_vfs * vfs, int mSecs) {
    svcSleepThread(mSecs * 1000);
    return mSecs;
}

// Returns the current time as UTC in Julian days
static int nxCurrentTime(sqlite3_vfs * vfs, double * time) {
    u64 ts;
    Result rc = timeGetCurrentTime(TimeType_Default, &ts);
    if (R_FAILED(rc)) {
        return SQLITE_ERROR;
    }

    *(time) = ts/86400.0 + 2440587.5;
    return SQLITE_OK;
}

// Returns a pointer to this VFS so it can be used
sqlite3_vfs * sqlite3_nxvfs() {
    static sqlite3_vfs nxvfs = {
        1,                            // iVersion
        sizeof(nxFile),               // szOsFile
        FS_MAX_PATH,                  // mxPathname
        0,                            // pNext
        "nx",                         // zName
        0,                            // pAppData
        nxOpen,                       // xOpen
        nxDelete,                     // xDelete
        nxAccess,                     // xAccess
        nxFullPathname,               // xFullPathname
        nxDlOpen,                     // xDlOpen
        nxDlError,                    // xDlError
        nxDlSym,                      // xDlSym
        nxDlClose,                    // xDlClose
        nxRandomness,                 // xRandomness
        nxSleep,                      // xSleep
        nxCurrentTime,                // xCurrentTime
    };
    return &nxvfs;
}

// Opens the FsFileSystem and registers the VFS
SQLITE_API int sqlite3_os_init() {
    Result rc = fsOpenImageDirectoryFileSystem(&fs, FsImageDirectoryId_Sd);
    if (R_FAILED(rc)) {
        return SQLITE_ERROR;
    }
    sqlite3_vfs_register(sqlite3_nxvfs(), 1);
    return SQLITE_OK;
}

// Closes the FsFileSystem
SQLITE_API int sqlite3_os_end() {
    fsFsClose(&fs);
    return SQLITE_OK;
}