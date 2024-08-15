#include "filters.h"
#include <stdint.h>
#include <Arduino.h>
#include <FS.h>
#include <SD.h>

int MIC_SAMPLE_RATE = 44100;

typedef struct {
    double a0, a1, a2, b0, b1, b2, x1, x2, y1, y2;
} filter_coefficients_t;

// Wave header as struct
typedef struct {
    char riff_tag[4];
    uint32_t riff_length;
    char wave_tag[4];
    char fmt_tag[4];
    uint32_t fmt_length;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    char data_tag[4];
    uint32_t data_length;
} wave_header_t;

int16_t filter(int16_t& input, filter_coefficients_t& coefficients) {
    
    double output = coefficients.b0 * input + coefficients.b1 * coefficients.x1 + coefficients.b2 * coefficients.x2 - coefficients.a1 * coefficients.y1 - coefficients.a2 * coefficients.y2;
    coefficients.x2 = coefficients.x1;
    coefficients.x1 = input;
    coefficients.y2 = coefficients.y1;
    coefficients.y1 = output;

    if (output > INT16_MAX) {
        output = INT16_MAX;
    }
    return int16_t(output);
}
filter_coefficients_t calculate_filter_coefficients_butterworth(int cuttoff_freq, int sample_rate, bool high_pass) {
    filter_coefficients_t coefficients;
    coefficients.x1 = 0;
    coefficients.x2 = 0;
    coefficients.y1 = 0;
    coefficients.y2 = 0;
    double omega = 2.0 * PI * cuttoff_freq / sample_rate;
    double sn = sin(omega);
    double cs = cos(omega);
    double alpha = sn / (2.0 * sqrt(2.0)); // Q = sqrt(2)/2 for Butterworth
    double norm = 1.0 / (1.0 + alpha);
    coefficients.b0 = high_pass ? (1.0 + cs) / 2.0 * norm : (1.0 - cs) / 2.0 * norm;
    coefficients.b1 = high_pass ? -(1.0 + cs) * norm : (1.0 - cs) * norm;
    coefficients.b2 = coefficients.b0;
    coefficients.a1 = -2.0 * cs * norm;
    coefficients.a2 = (1.0 - alpha) * norm;
    return coefficients;
}

filter_coefficients_t calculate_filter_coefficients_biquad(int low_cutoff, int high_cutoff, int sample_rate) {
    filter_coefficients_t coefficients;
    coefficients.x1 = 0;
    coefficients.x2 = 0;
    coefficients.y1 = 0;
    coefficients.y2 = 0;

    int center_freq = (low_cutoff + high_cutoff) / 2;
    int bandwidth = high_cutoff - low_cutoff;

    double omega = 2.0 * PI * center_freq / sample_rate;
    double alpha = std::sin(omega) * std::sinh(std::log(2.0) / 2.0 * bandwidth * omega / std::sin(omega));
    alpha = 1;

    coefficients.b0 = 1.0;
    coefficients.b1 = -2.0 * std::cos(omega);
    coefficients.b2 = 1.0;
    coefficients.a0 = 1.0 + alpha;
    coefficients.a1 = -2.0 * std::cos(omega);
    coefficients.a2 = 1.0 - alpha;

    // Normalize the coefficients
    coefficients.b0 /= coefficients.a0;
    coefficients.b1 /= coefficients.a0;
    coefficients.b2 /= coefficients.a0;
    coefficients.a1 /= coefficients.a0;
    coefficients.a2 /= coefficients.a0;



    return coefficients;
}
bool processWAVFile(const std::string &inputFilePath, const std::string &outputFilePath,int cuttoff_freq,bool high_pass) {
    File inputFile = SD.open(inputFilePath.c_str(), FILE_READ);
    if (!inputFile) {
        Serial.println("Error opening input file");
        return false;
    }

    File outputFile = SD.open(outputFilePath.c_str(), FILE_WRITE);
    if (!outputFile) {
        Serial.println("Error opening output file");
        inputFile.close();
        return false;
    }

    //read and clone the header to the ouput file
    wave_header_t header;
    inputFile.read((uint8_t*)&header, sizeof(wave_header_t));
    outputFile.write((uint8_t*)&header, sizeof(wave_header_t));

    // Buffer for audio data
    const int bufferSize = 512;
    int16_t buffer[bufferSize];
    

    //calculate coefficients for 2nd order butterworth filter
    filter_coefficients_t filter_coefficients_1 = calculate_filter_coefficients_butterworth(cuttoff_freq, MIC_SAMPLE_RATE, high_pass);
    filter_coefficients_t filter_coefficients_2 = calculate_filter_coefficients_butterworth(cuttoff_freq, MIC_SAMPLE_RATE, high_pass);
    filter_coefficients_t filter_coefficients_3 = calculate_filter_coefficients_butterworth(cuttoff_freq, MIC_SAMPLE_RATE, high_pass);
    

    while (inputFile.available()) {
        int bytesRead = inputFile.read((uint8_t*)buffer, sizeof(buffer));
        int samplesRead = bytesRead / sizeof(int16_t);

        

        // Cascade filter 3 times for extreme effect
        for (int i = 0; i < samplesRead; i++) {
            buffer[i] = filter(buffer[i], filter_coefficients_1);
            buffer[i] = filter(buffer[i], filter_coefficients_2);
            buffer[i] = filter(buffer[i], filter_coefficients_3);
    
        }

        // Write filtered data to the output file
        outputFile.write((uint8_t*)buffer, bytesRead);
    }

    inputFile.close();
    outputFile.close();
    
    return true;
}

bool bandRejectFilter(const std::string &inputFilePath, const std::string &outputFilePath,int low_cuttoff, int high_cutoff) {
    File inputFile = SD.open(inputFilePath.c_str(), FILE_READ);
    if (!inputFile) {
        Serial.println("Error opening input file");
        return false;
    }

    File outputFile = SD.open(outputFilePath.c_str(), FILE_WRITE);
    if (!outputFile) {
        Serial.println("Error opening output file");
        inputFile.close();
        return false;
    }

    //read and clone the header to the ouput file
    wave_header_t header;
    inputFile.read((uint8_t*)&header, sizeof(wave_header_t));
    outputFile.write((uint8_t*)&header, sizeof(wave_header_t));

    // Buffer for audio data
    const int bufferSize = 512;
    int16_t buffer[bufferSize];
    

    //calculate coefficients for 2nd order butterworth filter
    filter_coefficients_t filter_coefficients_1 = calculate_filter_coefficients_biquad(low_cuttoff,high_cutoff, MIC_SAMPLE_RATE);
    filter_coefficients_t filter_coefficients_2 = calculate_filter_coefficients_biquad(low_cuttoff,high_cutoff, MIC_SAMPLE_RATE);

    
    int chuncks_read = 0;
    while (inputFile.available()) {
        
        int bytesRead = inputFile.read((uint8_t*)buffer, sizeof(buffer));
        int samplesRead = bytesRead / sizeof(int16_t);

        for (int i = 0; i < samplesRead; i++) {
            buffer[i] = filter(buffer[i], filter_coefficients_1);
            buffer[i] = filter(buffer[i], filter_coefficients_2);
            if (chuncks_read > 10) {
                buffer[i] = buffer[i] * 3;
            }
    
        }


        // Write filtered data to the output file
        outputFile.write((uint8_t*)buffer, bytesRead);
        chuncks_read++;
    }
    

    inputFile.close();
    outputFile.close();
    
    return true;
}

bool highPassFilter(const std::string &inputFilePath, const std::string &outputFilePath, int cuttoff_freq)
{
    return processWAVFile(inputFilePath, outputFilePath, cuttoff_freq, true);
}

bool lowPassFilter(const std::string &inputFilePath, const std::string &outputFilePath, int cuttoff_freq)
{
    return processWAVFile(inputFilePath, outputFilePath, cuttoff_freq, true);
}
