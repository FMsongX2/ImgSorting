#pragma once

#include <vector>

bool decodeAudioFile(const char* path, std::vector<float>& samples, int& channels, int& rate);
