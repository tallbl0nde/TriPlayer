#ifndef MIGRATION_HPP
#define MIGRATION_HPP

// This header simply includes all the other migration headers
#include "db/migrations/1_CreateTables.hpp"
#include "db/migrations/2_AddArtistImage.hpp"
#include "db/migrations/3_AddAlbumMetadata.hpp"
#include "db/migrations/4_AddPlaylistImage.hpp"
#include "db/migrations/5_UpdateSearch.hpp"
#include "db/migrations/6_RemoveImages.hpp"
#include "db/migrations/7_AddAudioFormat.hpp"

#endif