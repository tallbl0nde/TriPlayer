#include <cstring>
#include "sources/Flac.hpp"
#include "Log.hpp"

Flac::Flac(const std::string &path)
	: Source()
{
	Log::writeInfo("[FLAC] Opening file: " + path);
	this->decoder = new FlacDecoder();

	auto initStatus = this->decoder->init(path);
	if (initStatus != FLAC__STREAM_DECODER_INIT_STATUS_OK)
	{
		this->valid_ = false;
		Log::writeError(FLAC__StreamDecoderInitStatusString[initStatus]);
		Log::writeError("[FLAC] Unable to open file");
	}
	this->decoder->process_until_end_of_metadata();
	this->sampleRate_ = this->decoder->get_sample_rate();
	this->channels_ = (int) this->decoder->get_channels();
	this->totalSamples_ = this->decoder->get_total_samples();
	this->valid_ = true;
	Log::writeInfo("[FLAC] File opened successfully");
}

Flac::~Flac()
{
	delete this->decoder;
}

size_t Flac::decode(unsigned char *buf, size_t buf_size) {
	this->decoder->set_decode_buffer(buf);
	size_t i = 0;
	while (i < buf_size)
	{
		auto blockSize = this->decoder->get_blocksize();
		if (!i + blockSize > buf_size) {
			this->decoder->process_single();
			if (this->decoder->get_state() == FLAC__STREAM_DECODER_END_OF_STREAM) {
				this->done_ = true;
				return i;
			}
		}
		else
			return i;
		i += blockSize;
	}
	return i;
}

void Flac::seek(size_t pos) {
	this->decoder->seek_absolute(pos);
}

size_t Flac::tell() {
	size_t pos = 0;
	this->decoder->get_decode_position(&pos);
	return pos;
}

FLAC__StreamDecoderWriteStatus
FlacDecoder::write_callback(const ::FLAC__Frame *frame,
							const FLAC__int32 *const *buffer) {
	if (!this->decodeBuffer)
		return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
	for(unsigned int i = 0; i < frame->header.blocksize; i++)
	{
		*(int16_t *)decodeBuffer = (FLAC__int16)buffer[0][i];
		decodeBuffer += 2;
		*(int16_t *)decodeBuffer = (FLAC__int16)buffer[1][i];
		decodeBuffer += 2;
	}
	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void FlacDecoder::error_callback(::FLAC__StreamDecoderErrorStatus status) {
	Log::writeError(FLAC__StreamDecoderErrorStatusString[status]);
}

void FlacDecoder::set_decode_buffer(unsigned char *decode_buffer) {
	decodeBuffer = decode_buffer;
}
