#include "utils/Flac.hpp"
#include "Log.hpp"

namespace Utils
{
	Flac::Flac(const std::string &path) {
		this->decoder = new FlacDecoder(&this->meta);
		this->decoder->set_metadata_respond(FLAC__METADATA_TYPE_PICTURE);
		this->decoder->set_metadata_respond
		(FLAC__METADATA_TYPE_VORBIS_COMMENT);
		this->meta.ID = -3;
		if (this->decoder->init(path))
		{
			Log::writeError("[FLAC] Could not open " + path);
			return;
		}
		this->meta.title = std::filesystem::path(path).stem();
		this->meta.artist = "Unknown Artist";
		this->meta.album = "Unknown Album";
		this->meta.trackNumber = 0;
		this->meta.discNumber = 0;
		this->decoder->process_until_end_of_metadata();
		if (this->meta.ID == -3)
		{
			Log::writeInfo("[FLAC] No metadata found in " + path);
			this->meta.ID = -2;
		}
	}

	FlacDecoder::FlacDecoder(Metadata::Song *dst)
		: FLAC::Decoder::File(), metaPtr(dst)
	{}

	Flac::~Flac() {
		delete this->decoder;
	}

	std::vector<unsigned char> Flac::getArt() {
		return this->decoder->getAlbumart();
	}

	Metadata::Song Flac::getInfo() {
		return this->meta;
	}

	FLAC__StreamDecoderWriteStatus
	FlacDecoder::write_callback(const ::FLAC__Frame *frame,
								const FLAC__int32 *const *buffer) {
		return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
	}

	void FlacDecoder::error_callback(::FLAC__StreamDecoderErrorStatus status) {
	}

	void
	FlacDecoder::addMeta(const std::string &key, const std::string &value) {
		if (key == "ALBUM")
			this->metaPtr->album = value;
		if (key == "ARTIST")
			this->metaPtr->artist = value;
		if (key == "TITLE")
			this->metaPtr->title = value;
		if (key == "TRACKNUMBER")
			this->metaPtr->trackNumber = std::stoi(value);
		if (key == "DISCNUMBER")
			this->metaPtr->discNumber = std::stoi(value);
	}

	void
	FlacDecoder::metadata_callback(const ::FLAC__StreamMetadata *metadata) {
		Stream::metadata_callback(metadata);
		if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO)
		{
			auto *tag = (FLAC__StreamMetadata_StreamInfo *) &metadata->data;
			if (tag->total_samples)
				this->metaPtr->duration = tag->total_samples / tag->sample_rate;
			else
				this->metaPtr->duration = 0;
		}
		if (metadata->type == FLAC__METADATA_TYPE_VORBIS_COMMENT)
		{
			this->metaPtr->ID = -1;
			auto *tag = (FLAC__StreamMetadata_VorbisComment *) &metadata->data;
			for (unsigned int i = 0; i < tag->num_comments; i++)
			{
				std::string entry((char*)tag->comments[i].entry);
				auto key = entry.substr(0, entry.find("="));
				auto value = entry.substr(entry.find("=") + 1);
				this->addMeta(key, value);
			}
		}
		if (metadata->type == FLAC__METADATA_TYPE_PICTURE)
		{
			auto *tag = (FLAC__StreamMetadata_Picture *) &metadata->data;
			if (tag->type == FLAC__STREAM_METADATA_PICTURE_TYPE_FRONT_COVER)
			{
				std::string mimeType(tag->mime_type);
				if (mimeType == "image/jpg" ||
					mimeType == "image/jpeg" ||
					mimeType == "image/png")
				{
					for (size_t i = 0; i < tag->data_length; i++)
						this->albumart.push_back(*(tag->data + 1));
				}
				else
					Log::writeInfo("[FLAC] Embedded image unsupported");
			}
		}
	}

	std::vector<unsigned char> FlacDecoder::getAlbumart() {
		return this->albumart;
	}
}