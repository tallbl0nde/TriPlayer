#include <filesystem>

#include "utils/FileMeta.hpp"
#include "utils/MP3.hpp"
#include "utils/Flac.hpp"

Metadata::Song Utils::FileMeta::getMetadata(const std::string &path) {
	auto ext = std::filesystem::path(path).extension();
	if (ext == ".mp3")
		return MP3::getInfoFromID3(path);
	if (ext == ".flac")
	{
		Flac flac(path);
		return flac.getInfo();
	}
	Metadata::Song song;
	song.ID = -3;
	return song;
}

std::vector<unsigned char> Utils::FileMeta::getArt(const std::string &path) {
	auto ext = std::filesystem::path(path).extension();
	if (ext == ".mp3")
		return MP3::getArtFromID3(path);
	if (ext == ".flac")
	{
		Flac flac(path);
		return flac.getArt();
	}
	return std::vector<unsigned char>();
}
