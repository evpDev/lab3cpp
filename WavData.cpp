#include "WavData.h"
#include <string>
#include <iostream>
#include <cstdio>
#include <cstring>

wav_errors_e WavData::CreateFromFile(const char* filename) {
    myfilename = filename;
    std::cout << ">>>> read_header( " << filename << " )\n" << std::endl;
    null_header(header_ptr); // Fill header with zeroes.
    std::cout << "Done!" << std::endl;
    FILE* f = fopen( myfilename, "rb" );
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

wav_errors_e WavData::extract_data_int16( const char* filename) {
    printf( ">>>> extract_data_int16( %s )\n", filename );
    wav_errors_e err;
    WavHeader header;
    err = read_header( filename, &header );
    if ( err != WAV_OK ) {
        // Problems with reading a header.
        return err;
    }

    if ( header.bitsPerSample != 16 ) {
        // Only 16-bit samples is supported.
        return UNSUPPORTED_FORMAT;
    }

    FILE* f = fopen( filename, "rb" );
    if ( !f ) {
        return IO_ERROR;
    }
    fseek( f, 44, SEEK_SET ); // Seek to the begining of PCM data.

    int chan_count = header.numChannels;
    int samples_per_chan = ( header.subchunk2Size / sizeof(short) ) / chan_count;

    // 1. Reading all PCM data from file to a single vector.
    std::vector<short> all_channels;
    all_channels.resize( chan_count * samples_per_chan );
    size_t read_bytes = fread( all_channels.data(), 1, header.subchunk2Size, f );
    if ( read_bytes != header.subchunk2Size ) {
        printf( "extract_data_int16() read only %zu of %u\n", read_bytes, header.subchunk2Size );
        return IO_ERROR;
    }
    fclose( f );


    // 2. Put all channels to its own vector.
    channels_data.resize( chan_count );
    for ( size_t ch = 0; ch < channels_data.size(); ch++ ) {
        channels_data[ ch ].resize( samples_per_chan );
    }

    for ( int ch = 0; ch < chan_count; ch++ ) {
        std::vector<short>& chdata = channels_data[ ch ];
        for ( size_t i = 0; i < samples_per_chan; i++ ) {
            chdata[ i ] = all_channels[ chan_count * i + ch ];
        }
    }
    return WAV_OK;
}

wav_errors_e WavData::ConvertStereoToMono() {
    extract_data_int16(myfilename);

    int chan_count = (int)channels_data.size();

    if ( chan_count != 2 ) {
        return BAD_PARAMS;
    }

    int samples_count_per_chan = (int)channels_data[0].size();

    // Verify that all channels have the same number of samples.
    for ( size_t ch = 0; ch < chan_count; ch++ ) {
        if ( channels_data[ ch ].size() != (size_t) samples_count_per_chan ) {
            return BAD_PARAMS;
        }
    }

    dest_mono.resize( 1 );
    std::vector<short>& mono = dest_mono[ 0 ];
    mono.resize( samples_count_per_chan );

    // Mono channel is an arithmetic mean of all (two) channels.
    for ( size_t i = 0; i < samples_count_per_chan; i++ ) {
        mono[ i ] = ( channels_data[0][i] + channels_data[1][i] ) / 2;
    }

    return WAV_OK;
}

wav_errors_e make_wav_file(const char* filename) {
    myfilenameOut = filename;
    printf( ">>>> make_wav_file( %s )\n", myfilenameOut );
    wav_errors_e err;
    wav_header_s header;

    int chan_count = (int)dest_mono.size();

    if ( chan_count < 1 ) {
        return BAD_PARAMS;
    }

    int samples_count_per_chan = (int)dest_mono[0].size();

    // Verify that all channels have the same number of samples.
    for ( size_t ch = 0; ch < chan_count; ch++ ) {
        if ( dest_mono[ ch ].size() != (size_t) samples_count_per_chan ) {
            return BAD_PARAMS;
        }
    }

    err = fill_header( &header, chan_count, 16, sample_rate, samples_count_per_chan );
    if ( err != WAV_OK ) {
        return err;
    }

    std::vector<short> all_channels;
    all_channels.resize( chan_count * samples_count_per_chan );

    for ( int ch = 0; ch < chan_count; ch++ ) {
        const std::vector<short>& chdata = dest_mono[ ch ];
        for ( size_t i = 0; i < samples_count_per_chan; i++ ) {
            all_channels[ chan_count * i + ch ] = chdata[ i ];
        }
    }

    FILE* f = fopen( myfilenameOut, "wb" );
    fwrite( &header, sizeof(wav_header_s), 1, f );
    fwrite( all_channels.data(), sizeof(short), all_channels.size(), f );
    if ( !f ) {
        return IO_ERROR;
    }

    fclose( f );

    return WAV_OK;
}
