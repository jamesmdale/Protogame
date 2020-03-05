#include "Game\Helpers\ChunkFileLoader.hpp"
#include "Engine\File\File.hpp"
#include "Engine\Core\StringUtils.hpp"
#include "Engine\Core\EngineCommon.hpp"
#include "Game\GameObjects\Chunk.hpp"
#include "Game\Definitions\BlockDefinition.hpp"

//  =========================================================================================
bool ReadChunkDataFromFile(Chunk* outChunk)
{
	IntVector2 outChunkCoordinates = outChunk->m_chunkCoords;

	std::string fileName = Stringf("Chunk_%i,%i.chunk", outChunkCoordinates.x, outChunkCoordinates.y);

	std::string filePath = Stringf("%s%s", "Saves\\", fileName.c_str());

	//read in the file
	FILE *fp = nullptr;
	errno_t errorVal = fopen_s( &fp, filePath.c_str(), "rb" );	
	UNUSED(errorVal); //useful when step-debugging just to check error code

	if (fp == nullptr) {
		return false;
	}

	size_t size = 0U;
	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	byte_t *buffer = (byte_t*) malloc(size); // space for NULL

	size_t readSize = fread( buffer, 1, size, fp );
	fclose(fp);

	if(size <= sizeof(ChunkFileHeader))
		return false;

	//store buffer into bytepacker
	BytePacker bytepacker;
	bytepacker.WriteBytes(size, buffer, false);

	//read from file
	ChunkFileHeader header = ExtractChunkHeader(bytepacker);
	if(!ValidateChunkFileHeader(header))
		return false;		

	//probably need good error checking here
	uint blockIndex = 0;
	bool isReadComplete = false;
	uint8 type;
	uint8 runLength;
	uint blockCount = 0;

	while(!isReadComplete)
	{
		bytepacker.ReadBytes(&type, 1, false);
		bytepacker.ReadBytes(&runLength, 1, false);		

		//used for checking the number of actual blocks that get set
		blockCount += runLength;

		BlockDefinition* blockDefinitionForType = BlockDefinition::GetDefinitionById(type);

		ASSERT_OR_DIE(blockDefinitionForType != nullptr, "INVALID BLOCK TYPE LOADED FROM DATA!!!");

		for (uint runIndex = 0; runIndex < runLength; ++runIndex)
		{		
			outChunk->m_blocks[blockIndex].m_type = blockDefinitionForType->m_type;
			outChunk->m_blocks[blockIndex].m_bits = blockDefinitionForType->m_defaultBits;
			++blockIndex;
		}

		if(bytepacker.IsFinishedReading())
			isReadComplete = true;
	}

	return true;
}

//  =========================================================================================
bool WriteChunkDataToFile(Chunk* chunk)
{
	IntVector2 outChunkCoordinates = chunk->m_chunkCoords;

	std::string fileName = Stringf("Chunk_%i,%i.chunk", outChunkCoordinates.x, outChunkCoordinates.y);

	std::string filePath = Stringf("%s%s", "Saves\\", fileName.c_str());

	//read in the file
	FILE *fp = nullptr;
	errno_t errorVal = fopen_s( &fp, filePath.c_str(), "wb" );	
	UNUSED(errorVal); //useful when step-debugging just to check error code

	if (fp == nullptr) {
		return false;
	}

	//store buffer into bytepacker
	BytePacker bytepacker;

	WriteChunkHeader(bytepacker);

	uint8 currentType = chunk->m_blocks[0].m_type;
	uint8 currentRunLength = 1;	
	for (int blockIndex = 1; blockIndex < BLOCKS_PER_CHUNK; ++blockIndex)
	{
		//probably need good error checking here
		if (chunk->m_blocks[blockIndex].m_type == currentType && currentRunLength != UINT8_MAX)
		{
			currentRunLength++;
		}
		else
		{
			bytepacker.WriteBytes(1, &currentType, false);
			bytepacker.WriteBytes(1, &currentRunLength, false);			

			currentRunLength = 1;
			currentType = chunk->m_blocks[blockIndex].m_type;
		}
	}

	//write last run of blocks
	bytepacker.WriteBytes(1, &currentType, false);
	bytepacker.WriteBytes(1, &currentRunLength, false);	

	size_t writeSize = fwrite( bytepacker.GetBuffer(), 1, bytepacker.GetWrittenByteCount(), fp );
	fclose(fp);
}
//  =========================================================================================
void WriteChunkHeader(BytePacker & buffer)
{
	ChunkFileHeader headerData;
	buffer.WriteBytes(12, &headerData, false);
}

//  =========================================================================================
ChunkFileHeader ExtractChunkHeader(BytePacker& buffer)
{
	ChunkFileHeader headerData;

	buffer.ReadBytes(&headerData, 12, false);

	return headerData;
}

//  =========================================================================================
bool ValidateChunkFileHeader(const ChunkFileHeader& chunkFileHeader)
{
	if (chunkFileHeader.m_4CC[0] != 'S' ||
		chunkFileHeader.m_4CC[1] != 'M' ||
		chunkFileHeader.m_4CC[2] != 'C' ||
		chunkFileHeader.m_4CC[3] != 'D' ||
		chunkFileHeader.m_version != CHUNK_SAVE_VERSION ||
		chunkFileHeader.m_chunkBitsX != BITS_WIDE_X ||
		chunkFileHeader.m_chunkBitsY != BITS_WIDE_Y ||
		chunkFileHeader.m_chunkBitsZ != BITS_TALL_Z ||
		chunkFileHeader.m_blockdataFormat != 'R')
	{
		return false;
	}

	return true;
}