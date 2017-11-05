#include "WavHeader.h"
#include <vector>

enum wav_errors_e {
    WAV_OK = 0,
    IO_ERROR,
    BAD_FORMAT,
    UNSUPPORTED_FORMAT,
    BAD_PARAMS,
    DATA_SIZE_ERROR
};

// Possible header's errors
enum wav_headers_errors_e {
    HEADER_OK = 0,
    HEADER_RIFF_ERROR,
    HEADER_FILE_SIZE_ERROR,
    HEADER_WAVE_ERROR,
    HEADER_FMT_ERROR,
    HEADER_NOT_PCM,
    HEADER_SUBCHUNK1_ERROR,
    HEADER_BYTES_RATE_ERROR,
    HEADER_BLOCK_ALIGN_ERROR,
    HEADER_SUBCHUNK2_SIZE_ERROR
};

class WavData {
    char* myfilename;
    char* myfilenameOut;
    int sample_rate = 44100;
    vector< vector<short> > channels_data;
    std::vector< std::vector<short> > dest_mono;
    WavHeader* header_ptr;
    void null_header(WavHeader *header_ptr);
    wav_headers_errors_e check_header(size_t file_size_bytes);
    wav_errors_e extract_data_int16(const char* filename);
public:
    wav_errors_e CreateFromFile(const char* filename);
    void GetDescription();
    void SaveToFile(const char* filename);
    wav_errors_e ConvertStereoToMono();
};
