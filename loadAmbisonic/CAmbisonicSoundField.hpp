#ifndef _C_AMBISONIC_SOUND_FIELD_HPP_
#define _C_AMBISONIC_SOUND_FIELD_HPP_

#include "ofxAudioFile.h"
#include "CLoudspeakerArray.hpp"

#include <cmath>
#include <string>
#include <vector>

class CAmbisonicSoundField {
public:
	// Constructor: initialize soundFieldArray structure based on order and channels
	CAmbisonicSoundField(int order, int numChannels, int sampleRate): ambisonicOrder(order), numChannels(numChannels), sampleRate(sampleRate) {
		// Resize for orders: from 0 to order (order + 1 levels)
		soundFieldArray.resize(ambisonicOrder + 1);

		// For each order level, resize to (2*order + 1) channels 
		for (int i = 0; i <= ambisonicOrder; ++i) {
			soundFieldArray[i].resize(2 * i + 1);

			// Initialize each channel's sample vector empty 
			for (auto & channelSamples : soundFieldArray[i]) {
				channelSamples.clear(); 
			}
		}
	}

	// Load WAV file and fill soundFieldArray with samples mapped by ambisonic order and channel
	bool LoadWav(const std::string & filePath) {
		ofxAudioFile audioFile;
		audioFile.load(filePath);

		if (!audioFile.loaded()) {
			std::cerr << "Error loading WAV file: " << filePath << std::endl;
			return false;
		}

		int fileChannels = audioFile.channels();
		if (fileChannels <= 0) {
			std::cerr << "Invalid number of channels in WAV file." << std::endl;
			return false;
		}

		// Calculate ambisonic order based on number of channels:
		// Number of channels = (order + 1)^2 for ambisonics
		int calculatedOrder = static_cast<int>(std::sqrt(fileChannels)) - 1;
		if (calculatedOrder < 0 || std::pow(calculatedOrder + 1, 2) != fileChannels) {
			std::cerr << "Number of channels does not correspond to a valid ambisonic order." << std::endl;
			return false;
		}

		size_t numSamples = audioFile.length();

		// Resize soundFieldArray for calculated order and channels
		soundFieldArray.resize(calculatedOrder + 1);
		for (int orderIdx = 0; orderIdx <= calculatedOrder; ++orderIdx) {
			int channelsInOrder = 2 * orderIdx + 1;
			soundFieldArray[orderIdx].resize(channelsInOrder);
			for (int ch = 0; ch < channelsInOrder; ++ch) {
				soundFieldArray[orderIdx][ch].clear();
				soundFieldArray[orderIdx][ch].reserve(numSamples);
			}
		}

		// Fill soundFieldArray with samples from the WAV file
		for (size_t i = 0; i < numSamples; ++i) {
			int channelCounter = 0;
			for (int orderIdx = 0; orderIdx <= calculatedOrder; ++orderIdx) {
				int channelsInOrder = 2 * orderIdx + 1;
				for (int ch = 0; ch < channelsInOrder; ++ch) {
					float sample = audioFile.sample(i, channelCounter);
					soundFieldArray[orderIdx][ch].push_back(sample);
					++channelCounter;
				}
			}
		}

		// Update class members to reflect loaded data
		numChannels = fileChannels;
		ambisonicOrder = calculatedOrder;
		sampleRate = audioFile.samplerate();

		return true;
	}

	//Rotar el campo dependiendo de como se hayan variado las coordenadas
	void RotateField(float yaw, float pitch, float roll); //A implementar

	//Delvuelve el orden del campo
	void GetOrder(); //A implementar

	//Permite guardar el campo en un archivo WAV
	void SaveToWav(); //A implementar

	//Devuelve un vector por altavoz
	std::vector<std::vector<float>> decodeToSpeakerArray(const CLoudSpeakerArray & array) const;

	CAmbisonicSoundField CAmbisonicSoundField::operator+(const CAmbisonicSoundField & other) const {
		// Verificamos que los campos tengan la misma estructura
		if (ambisonicOrder != other.ambisonicOrder || sampleRate != other.sampleRate) {
			throw std::runtime_error("Ambisonic fields must have the same order and sample rate to add.");
		}

		CAmbisonicSoundField result = *this; // Copiamos el campo actual

		for (int l = 0; l < ambisonicOrder + 1; ++l) {
			for (int ch = 0; ch < soundFieldArray[l].size(); ++ch) {
				for (int i = 0; i < soundFieldArray[l][ch].size(); ++i) {
					result.soundFieldArray[l][ch][i] += other.soundFieldArray[l][ch][i];
				}
			}
		}

		return result;
	}


private:
	int ambisonicOrder;
	size_t numChannels;
	size_t sampleRate; //Cambiar por FrameSize?

	// 3D vector to hold samples:
	// Dimensions: [order level][channel within order][samples over time]
	//Vector que almacena el numero de "ordenes", en cada orden sus correspondientes canales, y en cada canal sus muestras.
	std::vector<std::vector<std::vector<float>>> soundFieldArray;
};

#endif // _C_AMBISONIC_SOUND_FIELD_HPP_
