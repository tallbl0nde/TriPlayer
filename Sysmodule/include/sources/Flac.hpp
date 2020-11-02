#ifndef FLAC_HPP
#define FLAC_HPP

#include <string>
#include "FLAC++/decoder.h"
#include "sources/Source.hpp"

namespace NX {
	class File;
};

class FlacDecoder : public FLAC::Decoder::File
{
	unsigned char *decodeBuffer;
protected:
	FLAC__StreamDecoderWriteStatus write_callback(const ::FLAC__Frame *frame,
												  const FLAC__int32 *const *buffer) override;
	void error_callback(::FLAC__StreamDecoderErrorStatus status) override;
public:
	void set_decode_buffer(unsigned char *decode_buffer);
};

class Flac : public Source {
private:
	FlacDecoder *decoder;
	NX::File *file;
public:
	Flac(const std::string &);
	~Flac();

	size_t decode(unsigned char *, size_t);
	void seek(size_t);
	size_t tell();
};

#endif
