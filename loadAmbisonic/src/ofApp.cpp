#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
	LoadWav("../resources/ambi_2OA_test.wav");

	// Configuración del stream de audio
	ofSoundStreamSettings settings;
	settings.setOutListener(this);
	settings.sampleRate = 48000;
	settings.numOutputChannels = numChannels;
	settings.numInputChannels = 0;
	settings.bufferSize = 1024;
	ofSoundStreamSetup(settings);
}

//--------------------------------------------------------------
void ofApp::update() {
	// En el update no necesitamos hacer nada específico para la visualización,
	// todo lo manejamos en la función draw().
}

//--------------------------------------------------------------
void ofApp::draw() {
	ofBackground(255);

	string keyPressedText = "Canal activo: ";

	switch (playbackMode) {
	case PLAY_CH_W:
		keyPressedText += "W";
		break;
	case PLAY_CH_X:
		keyPressedText += "X";
		break;
	case PLAY_CH_Y:
		keyPressedText += "Y";
		break;
	case PLAY_CH_Z:
		keyPressedText += "Z";
		break;
	case PLAY_CH_V:
		keyPressedText += "V";
		break;
	case PLAY_CH_T:
		keyPressedText += "T";
		break;
	case PLAY_CH_R:
		keyPressedText += "R";
		break;
	case PLAY_CH_S:
		keyPressedText += "S";
		break;
	case PLAY_CH_U:
		keyPressedText += "U";
		break;
	case PLAY_ALL:
		keyPressedText = "Reproduciendo todos los canales";
		break;
	}

	ofSetColor(0); // Color negro
	ofDrawBitmapString(keyPressedText, 20, 40); // Texto en (x=20, y=40)
}

//--------------------------------------------------------------
void ofApp::audioOut(ofSoundBuffer & outBuffer) {
	for (size_t i = 0; i < outBuffer.getNumFrames(); i++) {
		float sample = 0.0f;

		switch (playbackMode) {
		case PLAY_CH_W:
			if (playbackIndex < channels[0].size()) sample = channels[0][playbackIndex];
			break;
		case PLAY_CH_Y:
			if (playbackIndex < channels[1].size()) sample = channels[1][playbackIndex];
			break;
		case PLAY_CH_Z:
			if (playbackIndex < channels[2].size()) sample = channels[2][playbackIndex];
			break;
		case PLAY_CH_X:
			if (playbackIndex < channels[3].size()) sample = channels[3][playbackIndex];
			break;
		case PLAY_CH_V:
			if (playbackIndex < channels[4].size()) sample = channels[4][playbackIndex];
			break;
		case PLAY_CH_T:
			if (playbackIndex < channels[5].size()) sample = channels[5][playbackIndex];
			break;
		case PLAY_CH_R:
			if (playbackIndex < channels[6].size()) sample = channels[6][playbackIndex];
			break;
		case PLAY_CH_S:
			if (playbackIndex < channels[7].size()) sample = channels[7][playbackIndex];
			break;
		case PLAY_CH_U:
			if (playbackIndex < channels[8].size()) sample = channels[8][playbackIndex];
			break;
		case PLAY_ALL:
			for (int ch = 0; ch < numChannels; ch++) {
				if (playbackIndex < channels[ch].size()) sample += channels[ch][playbackIndex];
			}
			sample /= 4.0f;
			break;
		}

		// Reproducimos el audio
		for (int j = 0; j < numChannels; j++) {
			outBuffer[i * numChannels + j] = sample;
		}

		playbackIndex++;
		if (playbackIndex >= channels[0].size()) playbackIndex = 0;
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	switch (key) {
	case 'w':
		if (keyWPressed) {
			keyWPressed = false;
			playbackMode = PLAY_ALL; 
		} else {
			keyWPressed = true;
			playbackMode = PLAY_CH_W;
		}
		break;
	case 'x':
		if (keyXPressed) {
			keyXPressed = false;
			playbackMode = PLAY_ALL; 
		} else {
			keyXPressed = true;
			playbackMode = PLAY_CH_X;
		}
		break;
	case 'y':
		if (keyYPressed) {
			keyYPressed = false;
			playbackMode = PLAY_ALL; 
		} else {
			keyYPressed = true;
			playbackMode = PLAY_CH_Y;
		}
		break;
	case 'z':
		if (keyZPressed) {
			keyZPressed = false;
			playbackMode = PLAY_ALL; 
		} else {
			keyZPressed = true;
			playbackMode = PLAY_CH_Z;
		}
		break;
	case 'v':
		if (keyVPressed) {
			keyVPressed = false;
			playbackMode = PLAY_ALL;
		} else {
			keyVPressed = true;
			playbackMode = PLAY_CH_V;
		}
		break;
	case 't':
		if (keyTPressed) {
			keyTPressed = false;
			playbackMode = PLAY_ALL;
		} else {
			keyTPressed = true;
			playbackMode = PLAY_CH_T;
		}
		break;
	case 'r':
		if (keyRPressed) {
			keyRPressed = false;
			playbackMode = PLAY_ALL;
		} else {
			keyRPressed = true;
			playbackMode = PLAY_CH_R;
		}
		break;
	case 's':
		if (keySPressed) {
			keySPressed = false;
			playbackMode = PLAY_ALL;
		} else {
			keySPressed = true;
			playbackMode = PLAY_CH_S;
		}
		break;
	case 'u':
		if (keyUPressed) {
			keyUPressed = false;
			playbackMode = PLAY_ALL;
		} else {
			keyUPressed = true;
			playbackMode = PLAY_CH_U;
		}
		break;
	case 'a':
		keyWPressed = false;
		keyXPressed = false;
		keyYPressed = false;
		keyZPressed = false;
		keyVPressed = false;
		keyTPressed = false;
		keyRPressed = false;
		keySPressed = false;
		keyUPressed = false;
		playbackMode = PLAY_ALL; 
		break;
	}
}

//--------------------------------------------------------------
void ofApp::LoadWav(const char * filePath) {
	ofxAudioFile audioFile;
	audioFile.load(filePath);

	if (!audioFile.loaded()) {
		std::cerr << "Error loading WAV: " << filePath << std::endl;
		return;
	}

	numChannels = audioFile.channels();
	numSamples = audioFile.length();

	channels.clear();
	channels.resize(numChannels); 

	for (auto & ch : channels) {
		ch.clear();
		ch.reserve(numSamples);
	}

	for (size_t i = 0; i < numSamples; i++) {
		for (int ch = 0; ch < std::min((int)numChannels, (int)numChannels); ch++) {
			channels[ch].push_back(audioFile.sample(i, ch));
		}
	}

	// Prepara las ondas para su visualización
	waveforms.resize(numChannels);
	for (int i = 0; i < numChannels; i++) {
		waveforms[i].clear();
		for (size_t j = 0; j < channels[i].size(); j++) {
			waveforms[i].push_back(channels[i][j]);
		}
	}
}
