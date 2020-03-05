#pragma once
#include "Engine\Core\BytePacker.hpp"
#include "Game\GameCommon.hpp"

class Chunk;

//12 bytes
struct ChunkFileHeader
{
	uint8 m_4CC[4] = {'S', 'M', 'C', 'D'};
	uint8 m_version = CHUNK_SAVE_VERSION;
	uint8 m_chunkBitsX = BITS_WIDE_X;
	uint8 m_chunkBitsY = BITS_WIDE_Y;
	uint8 m_chunkBitsZ = BITS_TALL_Z;
	uint8 m_reserved1 = 0;
	uint8 m_reserved2 = 0;
	uint8 m_reserved3 = 0;
	uint8 m_blockdataFormat = 'R'; //stands for RLE - Compressed
};

bool ReadChunkDataFromFile(Chunk* outChunk);
bool WriteChunkDataToFile(Chunk* chunk);

void WriteChunkHeader(BytePacker& buffer);
ChunkFileHeader ExtractChunkHeader(BytePacker& buffer);
bool ValidateChunkFileHeader(const ChunkFileHeader& chunkFileHeader);

