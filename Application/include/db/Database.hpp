#ifndef DATABASE_HPP
#define DATABASE_HPP

#include "SQLite.hpp"
#include "Types.hpp"
#include <vector>

// The Database class interacts with the database stored on the sd card
// to read/write data. All queries have a way of detecting if they failed.
// If so, error() will return a non-empty string describing the error.
class Database {
    public:
        // Column and method to sort results by
        // Note that a default order will be used if the requested data
        // does not support the requested sort type
        enum class SortBy {
            TitleAsc,       // Title/name (A-Z)
            TitleDsc,       // Title/name (Z-A)
            ArtistAsc,      // Artist name (A-Z)
            ArtistDsc,      // Artist name (Z-A)
            AlbumAsc,       // Album name (A-Z)
            AlbumDsc,       // Album name (Z-A)
            AlbumsAsc,      // Album count (least first)
            AlbumsDsc,      // Album count (most first)
            LengthAsc,      // Length (shortest first)
            LengthDsc,      // Length (longest first)
            SongsAsc,       // Song count (least first)
            SongsDsc        // Song count (most first)
        };

    private:
        // Interface to database
        SQLite * db;
        // String describing last error
        std::string error_;
        // Number of phrases to generate when searching
        unsigned int searchPhrases;
        // Maximum 'spellfix score' to permit for searches
        unsigned int searchScore;

        // Indicates whether search_update has been set to 1
        // Used to avoid repeated UPDATE queries
        bool updateMarked;

        // Update the stored error message
        void setErrorMsg(const std::string &);

        // ===== Private Queries ===== //
        bool addArtist(std::string &);
        bool addAlbum(std::string &);
        bool getVersion(int &);
        bool setSearchUpdate(int);
        std::vector<std::string> getSearchPhrases(const std::string &, std::string &);

    public:
        // ===== Housekeeping ===== //
        // Constructor creates + 'migrates' the database to a newer version if needed
        Database();
        // Migrates the database to bring it to the latest version
        bool migrate();
        // Returns the last error that occurred (blank if no error has occurred)
        std::string error();
        // Set the maximum number of phrases to search with (higher means a 'broader' search)
        void setSearchPhraseCount(const unsigned int);
        // Set the maximum 'spellfix score' to use for searches (higher means less accurate)
        void setSpellfixScore(const unsigned int);

        // ===== Connection Management ===== //
        // Open the database read-write (will block until available)
        bool openReadOnly();
        // Open the database read-only (will block until available)
        bool openReadWrite();
        // Close a open connection (if there is one)
        void close();

        // ===== Album Metadata ===== //
        // Update an album's metadata (grabs ID from struct)
        bool updateAlbum(Metadata::Album);
        // Returns metadata for all stored albums
        // Empty if no albums or an error occurred
        std::vector<Metadata::Album> getAllAlbumMetadata(SortBy);
        // Return metadata for the given AlbumID
        // ID will be negative if not found
        Metadata::Album getAlbumMetadataForID(AlbumID);
        // Return metadata for all albums an artist is a part of
        // Empty if none found or an error occurred
        std::vector<Metadata::Album> getAlbumMetadataForArtist(ArtistID, SortBy);

        // ===== Artist Metadata ===== //
        // Update an artist's metadata (grabs ID from struct)
        bool updateArtist(Metadata::Artist);
        // Returns metadata for all stored artists
        // Empty if no artists or an error occurred
        std::vector<Metadata::Artist> getAllArtistMetadata(SortBy);
        // Returns a list of artists for a given AlbumID
        // Empty if no artists or an error occurred
        std::vector<Metadata::Artist> getArtistMetadataForAlbum(AlbumID);
        // Return metadata for the given ArtistID
        // ID will be negative if not found
        Metadata::Artist getArtistMetadataForID(ArtistID);

        // ===== Playlist Metadata ===== //
        // Add a blank playlist to the database
        // Returns true if successful, false otherwise
        bool addPlaylist(Metadata::Playlist);
        // Update a playlist's metadata
        // Returns true if successful, false otherwise
        bool updatePlaylist(Metadata::Playlist);
        // Remove a playlist from the database
        // Returns true if successful, false otherwise
        bool removePlaylist(PlaylistID);
        // Returns metadata for all stored playlists
        // Empty if no playlists or an error occurred
        std::vector<Metadata::Playlist> getAllPlaylistMetadata(SortBy);
        // Returns metadata for given ID (id will be -1 if not found!)
        Metadata::Playlist getPlaylistMetadataForID(PlaylistID);
        // Returns a playlist's songs
        // Empty if there are none or an error occurred
        std::vector<Metadata::PlaylistSong> getSongMetadataForPlaylist(PlaylistID, SortBy);
        // Add a song to a playlist
        // Return true if successful, false otherwise
        bool addSongToPlaylist(PlaylistID, SongID);
        // Remove a song from a playlist
        // Return true if successful, false otherwise
        bool removeSongFromPlaylist(PlaylistSongID);

        // ===== Song Metadata ===== //
        // Add a song (and associated artists, etc) into database
        // Returns true if successful, false otherwise
        bool addSong(Metadata::Song);
        // Updates the matching song in the database
        // Returns true if successful, false otherwise
        bool updateSong(Metadata::Song);
        // Remove song from database with ID
        // Returns true if successful, false otherwise
        bool removeSong(SongID);
        // Returns metadata for all stored songs
        // Empty if no songs or an error occurred
        std::vector<Metadata::Song> getAllSongMetadata(SortBy);
        // Returns an album's songs
        // Empty if there are none or an error occurred
        std::vector<Metadata::Song> getSongMetadataForAlbum(AlbumID);
        // Returns an artist's songs
        // Empty if there are none or an error occurred
        std::vector<Metadata::Song> getSongMetadataForArtist(ArtistID);
        // Returns SongInfo for given ID (id will be -1 if not found!)
        Metadata::Song getSongMetadataForID(SongID);

        // ===== Search Queries ===== //
        // Returns if the database needs to be updated before searching
        bool needsSearchUpdate();
        // Indexes terms and prepares database for searching
        bool prepareSearch();
        // Search for records matching given text
        // The number of returned records can also be optionally limited
        // Empty if no matching songs or an error occurred
        std::vector<Metadata::Album> searchAlbums(const std::string, int = -1);
        std::vector<Metadata::Artist> searchArtists(const std::string, int = -1);
        std::vector<Metadata::Playlist> searchPlaylists(const std::string, int = -1);
        std::vector<Metadata::Song> searchSongs(const std::string, int = -1);

        // ===== Misc. Queries ===== //
        // Returns a vector of strings containing all referenced images
        // Empty if no image paths stored or an error occurred (bool set false on error, true on success)
        std::vector<std::string> getAllImagePaths(bool &);
        // Returns a vector of pairs (file path, modified time) for all songs
        // Empty if no songs or error occurred (bool set false on error, true on success)
        std::vector< std::pair<std::string, unsigned int> > getAllSongFileInfo(bool &);
        // Returns the id of the artist with the given name (-1 if not found)
        ArtistID getArtistIDForName(const std::string &);
        // Return the id of a song's album
        // ID will be negative on an error
        AlbumID getAlbumIDForSong(SongID);
        // Return the id of a song's artist
        // ID will be negative on an error
        ArtistID getArtistIDForSong(SongID);
        // Return ID of song with given path (-1 if not found)
        SongID getSongIDForPath(std::string &);

        // Destructor closes handle
        ~Database();
};

#endif