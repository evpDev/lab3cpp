#include "WavData.h"
#include <string>
#include <iostream>
#include <cstdio>
#include <cstring>

wav_errors_e WavData::CreateFromFile(const char* filename) {
    std::cout << ">>>> read_header( " << filename << " )\n" << std::endl;
    null_header(header_ptr); // Fill header with zeroes.
    std::cout << "Done!" << std::endl;
    FILE* f = fopen( filename, "rb" );
    if ( !f ) {
        return IO_ERROR;
    }
    std::cout << "Done!" << std::endl;
    size_t blocks_read = fread( header_ptr, sizeof(WavHeader), 1, f);
    if ( blocks_read != 1 ) {
        // can't read header, because the file is too small.
        return BAD_FORMAT;
    }
    std::cout << "Done!" << std::endl;
    fseek( f, 0, SEEK_END ); // seek to the end of the file
    size_t file_size = ftell( f ); // current position is a file size!
    fclose( f );
    std::cout << "Done!" << std::endl;
    if ( check_header(file_size) != HEADER_OK ) {
        return BAD_FORMAT;
    } else {
        return WAV_OK;
    }
    std::cout << "Done!" << std::endl;
}

void WavData::GetDescription() {
    std::string mess;
    mess += "-------------------------\n";
    mess += " audioFormat   %u\n", header_ptr->audioFormat;
    mess += " numChannels   %u\n", header_ptr->numChannels;
    mess += " sampleRate    %u\n", header_ptr->sampleRate;
    mess += " bitsPerSample %u\n", header_ptr->bitsPerSample;
    mess += " byteRate      %u\n", header_ptr->byteRate;
    mess += " blockAlign    %u\n", header_ptr->blockAlign;
    mess += " chunkSize     %u\n", header_ptr->chunkSize;
    mess += " subchunk1Size %u\n", header_ptr->subchunk1Size;
    mess += " subchunk2Size %u\n", header_ptr->subchunk2Size;
    mess += "-------------------------\n" ;
}

void WavData::null_header(WavHeader* header_ptr) {
    memset( header_ptr, 0, sizeof(WavHeader) );
}

wav_headers_errors_e WavData::check_header(size_t file_size_bytes) {
    // Go to wav_header.h for details
    if ( header_ptr->chunkId[0] != 0x52 ||
         header_ptr->chunkId[1] != 0x49 ||
         header_ptr->chunkId[2] != 0x46 ||
         header_ptr->chunkId[3] != 0x46 )
    {
        printf( "HEADER_RIFF_ERROR\n" );
        return HEADER_RIFF_ERROR;
    }

    if ( header_ptr->chunkSize != file_size_bytes - 8 ) {
        printf( "HEADER_FILE_SIZE_ERROR\n" );
        return HEADER_FILE_SIZE_ERROR;
    }

    if ( header_ptr->format[0] != 0x57 ||
         header_ptr->format[1] != 0x41 ||
         header_ptr->format[2] != 0x56 ||
         header_ptr->format[3] != 0x45 )
    {
        printf( "HEADER_WAVE_ERROR\n" );
        return HEADER_WAVE_ERROR;
    }

    if ( header_ptr->subchunk1Id[0] != 0x66 ||
         header_ptr->subchunk1Id[1] != 0x6d ||
         header_ptr->subchunk1Id[2] != 0x74 ||
         header_ptr->subchunk1Id[3] != 0x20 )
    {
        printf( "HEADER_FMT_ERROR\n" );
        return HEADER_FMT_ERROR;
    }

    if ( header_ptr->audioFormat != 1 ) {
        printf( "HEADER_NOT_PCM\n" );
        return HEADER_NOT_PCM;
    }

    if ( header_ptr->subchunk1Size != 16 ) {
        printf( "HEADER_SUBCHUNK1_ERROR\n" );
        return HEADER_SUBCHUNK1_ERROR;
    }

    if ( header_ptr->byteRate != header_ptr->sampleRate * header_ptr->numChannels * header_ptr->bitsPerSample/8 ) {
        printf( "HEADER_BYTES_RATE_ERROR\n" );
        return HEADER_BYTES_RATE_ERROR;
    }

    if ( header_ptr->blockAlign != header_ptr->numChannels * header_ptr->bitsPerSample/8 ) {
        printf( "HEADER_BLOCK_ALIGN_ERROR\n" );
        return HEADER_BLOCK_ALIGN_ERROR;
    }

    if ( header_ptr->subchunk2Id[0] != 0x64 ||
         header_ptr->subchunk2Id[1] != 0x61 ||
         header_ptr->subchunk2Id[2] != 0x74 ||
         header_ptr->subchunk2Id[3] != 0x61 )
    {
        printf( "HEADER_FMT_ERROR\n" );
        return HEADER_FMT_ERROR;
    }

    if ( header_ptr->subchunk2Size != file_size_bytes - 44 )
    {
        printf( "HEADER_SUBCHUNK2_SIZE_ERROR\n" );
        return HEADER_SUBCHUNK2_SIZE_ERROR;
    }

    return HEADER_OK;
}
