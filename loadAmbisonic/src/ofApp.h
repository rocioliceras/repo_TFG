#pragma once
#include "ofMain.h"
#include "ofxAudioFile.h"

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void audioOut(ofSoundBuffer & outBuffer);
	void keyPressed(int key);
	void LoadWav(const char * filePath);
	

	// Datos de audio
	std::vector<std::vector<float>> channels; // Almacena las muestras de cada canal
	size_t playbackIndex = 0;
	int playbackMode = 0;

	enum PlaybackOrder1 {
		PLAY_CH_W = 0, // Canal 0
		PLAY_CH_Y, // Canal 1
		PLAY_CH_Z, // Canal 2
		PLAY_CH_X, // Canal 3
		PLAY_CH_V, // Canal 4
		PLAY_CH_T, // Canal 5
		PLAY_CH_R, // Canal 6
		PLAY_CH_S, // Canal 7
		PLAY_CH_U, // Canal 8
		PLAY_ALL // Todos los canales
	};

	// Datos para representar las ondas
	std::vector<std::vector<float>> waveforms; // Almacena las ondas de cada canal

	size_t numChannels;
	size_t numSamples;

	// Variables para controlar las teclas
	bool keyWPressed = false;
	bool keyXPressed = false;
	bool keyYPressed = false;
	bool keyZPressed = false;
	bool keyVPressed = false;
	bool keyTPressed = false;
	bool keyRPressed = false;
	bool keySPressed = false;
	bool keyUPressed = false;
};
