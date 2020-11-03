#ifndef FLAC_HPP
#define FLAC_HPP

#include <vector>
#include <filesystem>
#include "../Types.hpp"
#include "FLAC++/metadata.h"
#include "FLAC++/decoder.h"

namespace Utils
{
	class FlacDecoder : public FLAC::Decoder::File
	{
		Metadata::Song *metaPtr;
		std::vector<unsigned char> albumart;
		void addMeta(const std::string &key, const std::string &value);
	protected:
		FLAC__StreamDecoderWriteStatus
		write_callback(const ::FLAC__Frame *frame,
					   const FLAC__int32 *const *buffer) override;
		void error_callback(::FLAC__StreamDecoderErrorStatus status) override;
		void metadata_callback(const ::FLAC__StreamMetadata *metadata) override;

	public:
		FlacDecoder(Metadata::Song *dst);
		std::vector<unsigned char> getAlbumart();
	};
	class Flac
	{
		FlacDecoder *decoder;
		Metadata::Song meta;
	public:
		Flac(const std::string &path);
		~Flac();

		std::vector<unsigned char> getArt();
		Metadata::Song getInfo();
	};
}

#endif
