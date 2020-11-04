#include <filesystem>
#include "sources/MP3.hpp"
#include "sources/Flac.hpp"
#include "sources/SourceFactory.hpp"

/*
 * This is ugly as hell
 * todo: make the sourcefactory factory worthy
 */

Source *SourceFactory::make_source(const std::string &path) {
	auto extension = std::filesystem::path(path).extension();
	if (extension == ".mp3")
		return new MP3(path);
	if (extension == ".flac")
		return new Flac(path);
	return nullptr;
}
