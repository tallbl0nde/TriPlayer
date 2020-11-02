#include "sources/MP3.hpp"
#include "sources/Flac.hpp"
#include "sources/SourceFactory.hpp"

/*
 * This is ugly as hell
 * todo: make the sourcefactory factory worthy
 */

Source *SourceFactory::make_source(const std::string &path) {
	if (path.ends_with(".mp3"))
		return new MP3(path);
	if (path.ends_with(".flac"))
		return new Flac(path);
	return nullptr;
}
