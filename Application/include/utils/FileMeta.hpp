#ifndef FILEMETA_HPP
#define FILEMETA_HPP

#include <vector>
#include "Types.hpp"

namespace Utils {
	class FileMeta {
	public:
		static Metadata::Song getMetadata(const std::string &path);
		static std::vector<unsigned char> getArt(const std::string &path);
	};
}

#endif
