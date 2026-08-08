#include "FlacSoundFileSource.h"
#include <cstring>
#include <Core/Log.h>
#include <Core/File/BitStreamReader.h>

using namespace engine::sound;

engine::sound::FlacSoundFileSource::FlacSoundFileSource()
{
}

engine::sound::FlacSoundFileSource::~FlacSoundFileSource()
{
}

SoundData* engine::sound::FlacSoundFileSource::ParseSoundFile(IBinaryStream* Stream)
{
	try
	{
		BitStreamReader Reader{ Stream };
		FlacFileLoader Loader{ &Reader };

		return nullptr;
	}
	catch (FlacLoadException& e)
	{
		Log::Info(str::Format("Stream filed to parse as a FLAC file: %s", e.ErrorCode));
		return nullptr;
	}
}

engine::sound::FlacFileLoader::FlacFileLoader(BitStreamReader* Stream)
{
	char Header[4]{};
	Stream->ReadBytes((uByte*)Header, 4);

	if (strncmp(Header, "fLaC", 4) != 0)
	{
		throw FlacLoadException("Invalid header bytes");
	}

	bool ReadHeaders = true;

	while (ReadHeaders)
	{
		//uint32 MetadataHeader = Stream->Get<uint32>();

		ReadHeaders = !Stream->ReadBit();
		uByte InfoByte = Stream->ReadBits<7>();

		uint32 HeaderLength = Stream->ReadBits<24>();

		switch (InfoByte)
		{
		case 0: {
			// General metadata
			this->MinBlockSize = BitStreamReader::SwapEndian<sizeof(uint16)>(Stream->Get<uint16>());
			this->MaxBlockSize = BitStreamReader::SwapEndian<sizeof(uint16)>(Stream->Get<uint16>());
			this->MaxFrameSize = Stream->ReadBits<24>();
			this->MaxBlockSize = Stream->ReadBits<24>();
			this->SampleRate = Stream->ReadBits<20>();
			this->NumChannels = Stream->ReadBits<3>();
			this->BitsPerSample = Stream->ReadBits<5>() + 1;
			this->SampleCount = Stream->ReadBits<36>();
			// MD5 Checksum here, but we don't really care.
			Stream->Get<uint64>();
			Stream->Get<uint64>();
			break;
		}
		case 4: {
			// Vorbis comment metadata (values are little endian here I guess)

			uint32 VendorStringLength = Stream->Get<uint32>();

			FlacVendor.resize(VendorStringLength);

			Stream->ReadBytes(reinterpret_cast<uByte*>(FlacVendor.data()), VendorStringLength);

			uint32 NumFields = Stream->Get<uint32>();

			uint32 RemainingChunk = HeaderLength - sizeof(uint32) * 2 - VendorStringLength;

			for (uint32 i = 0; i < NumFields; i++)
			{
				uint32 FieldLength = Stream->Get<uint32>();

				auto& h = Headers.emplace_back();

				h.resize(FieldLength);
				Stream->ReadBytes(reinterpret_cast<uByte*>(h.data()), FieldLength);

				RemainingChunk -= sizeof(uint32) + FieldLength;
			}

			Stream->ReadBytes(nullptr, RemainingChunk);
			break;
		}
		default:
			Stream->ReadBytes(nullptr, HeaderLength);
			break;
		}
	}

	ReadBody(Stream);
}

void engine::sound::FlacFileLoader::ReadBody(BitStreamReader* Stream)
{
	while (DecodeFrame(Stream))
	{
		ReadFrameFooter(Stream);
	}
}

bool engine::sound::FlacFileLoader::DecodeFrame(BitStreamReader* Stream)
{
	auto SyncFrame = Stream->ReadBits<15>();

	if (SyncFrame != 0b111111111111100)
	{
		throw FlacLoadException("Invalid FLAC frame header");
	}
	bool BlockStrategy = Stream->ReadBit();

	auto BlockSize = ConvertBlockSize(Stream->ReadBits<4>());
	auto SampleRate = Stream->ReadBits<4>();
	auto StereoMode = Stream->ReadBits<4>();
	auto BitDepth = Stream->ReadBits<3>();
	auto Reserved = Stream->ReadBit();

	auto FrameId = Stream->Get<uByte>();
	auto Crc = Stream->Get<uByte>();

	Log::Info(str::Format("Frame: %x %i %i", SyncFrame, BlockSize, SampleRate));

	if (FrameId > 127)
	{
		abort();
	}

	for (uint8 i = 0; i < this->NumChannels + 1; i++)
	{
		DecodeSubFrame(Stream, BitDepth, BlockSize);
	}
	return true;
}

void engine::sound::FlacFileLoader::DecodeSubFrame(BitStreamReader* Stream, uint64 BitDepth, uint64 BlockSize)
{
	bool ZeroBit = Stream->ReadBit();
	auto FrameType = Stream->ReadBits<6>();
	bool WastedSpace = Stream->ReadBit();
	std::vector<int16> Samples;

	if (FrameType == 0)
	{
		DecodeConstantFrame(Stream, BitDepth, BlockSize, Samples);
		return;
	}

	int8 FixedPredictor = FrameType - 0b001000;
	int8 LinearPredictOrder = FrameType - 31;

	int16 FirstSample = 0;

	if (BitDepth == 0)
	{
		BitDepth = this->BitsPerSample;
	}

	for (int8 i = 0; i < LinearPredictOrder; i++)
	{
		FirstSample = DecodeSample(Stream, BitDepth);

		Samples.push_back(FirstSample);
	}

	auto PredictorPrecision = Stream->ReadBits<4>() + 1;
	auto PredictorShift = Stream->ReadBits<5>();

	std::vector<int16> PredictorCoefficients;

	for (int8 i = 0; i < LinearPredictOrder; i++)
	{
		PredictorCoefficients.push_back(Stream->ReadNBits(PredictorPrecision));
	}

	DecodeResiduals(Stream);
	throw FlacLoadException("Not implemented");
}

void engine::sound::FlacFileLoader::DecodeResiduals(BitStreamReader* Stream)
{
	auto CodingMethod = Stream->ReadBits<4>();
}

void engine::sound::FlacFileLoader::DecodeConstantFrame(BitStreamReader* Stream, uint64 BitDepth, uint64 BlockSize,
	std::vector<int16>& Samples)
{
	int16 FirstSample = DecodeSample(Stream, BitDepth);

	for (size_t i = 0; i < BlockSize; i++)
	{
		Samples.push_back(FirstSample);
	}
}

uint64 engine::sound::FlacFileLoader::ConvertBlockSize(uint8 BlockSizeType)
{
	switch (BlockSizeType)
	{
	case 0b0001:
		return 192;
	case 0b0010:
	case 0b0011:
	case 0b0100:
	case 0b0101:
		return 144 * std::pow(2.0, double(BlockSizeType));
	case 0b0110:
		abort();
	case 0b0111:
		abort();
	case 0b1000:
	case 0b1001:
	case 0b1010:
	case 0b1011:
	case 0b1100:
	case 0b1101:
	case 0b1110:
	case 0b1111:
		return std::pow(2.0, double(BlockSizeType));
	}
	return uint64();
}

int16 engine::sound::FlacFileLoader::DecodeSample(BitStreamReader* Stream, uint8 Type)
{
	switch (Type)
	{
	case 0b001:
		return Stream->ReadBits<8>();
	case 0b010:
		return Stream->ReadBits<12>();
	case 0b100:
		return Stream->ReadBits<16>();
	case 0b101:
		return Stream->ReadBits<20>() >> 4;
	case 0b110:
		return Stream->ReadBits<24>() >> 8;
		break;
	case 0b111:
		return Stream->ReadBits<32>() >> 16;
	}
	throw FlacLoadException("Invalid bit depth");
}

void engine::sound::FlacFileLoader::ReadFrameFooter(BitStreamReader* Stream)
{
	Stream->AlignToByte();
	auto crc = Stream->ReadBits<16>(); // CRC (ignore for now)
}
