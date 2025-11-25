#pragma once

#include "ConfigurationA.hpp"
#include "ofMain.h"
#include "ofxBRT.h"

#define SAMPLERATE 44100
#define BUFFER_SIZE 256
#define LISTENER_ID "listener1"

struct AmbiFrame {
	float W;
	float X;
	float Y;
	float Z;
};

class ofApp : public ofBaseApp {

public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

	void audioIn(ofSoundBuffer & input);
	void audioOut(ofSoundBuffer & buffer);
	void audioProcess(Common::CEarPair<CMonoBuffer<float>> & bufferOutput, int bufferSize);

	ofSoundDevice ShowSelectAudioDeviceMenu();
	void AudioSetup();

	vector<float> left;
	vector<float> right;
	vector<float> volHistory;

	int bufferCounter;
	int drawCounter;

	float smoothedVol;
	float scaledVol;

	std::vector<Common::CVector3> vertices;
	using SourceModelPtr = std::shared_ptr<BRTSourceModel::CSourceSimpleModel>;
	std::vector<SourceModelPtr> speakers;
	CConfigurationA configurationA;

	Common::CGlobalParameters globalParameters;
	BRTBase::CBRTManager brtManager;
	std::shared_ptr<BRTBase::CListener> listener;
	Common::CEarPair<CMonoBuffer<float>> outputBufferStereo;
	ofSoundStream soundStream;

	std::vector<AmbiFrame> recordBuffer;
	bool isRecording = false;
	bool isPlaying = false;
	size_t playPosition = 0;

private:
	void setRealTimePriority() {
		HANDLE hThread = GetCurrentThread();
		SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);
	}

	std::shared_ptr<BRTSourceModel::CSourceSimpleModel> CreateOmnidirectionalSoundSource(
		BRTBase::CBRTManager & brtManager, const std::string & _soundSourceID) {
		brtManager.BeginSetup();
		auto _brtSoundSource = brtManager.CreateSoundSource<BRTSourceModel::CSourceSimpleModel>(_soundSourceID);
		brtManager.EndSetup();
		if (!_brtSoundSource) std::cout << "Error creating sound source" << std::endl;
		return _brtSoundSource;
	}

	vector<float> FLU;
	vector<float> FRD;
	vector<float> BLD;
	vector<float> BRU;

	vector<float> W;
	vector<float> X;
	vector<float> Y;
	vector<float> Z;

	std::vector<float> audioInputBuffer;

	void DecodeToSpeakerArray(const std::vector<float> & W,
		const std::vector<float> & X,
		const std::vector<float> & Y,
		const std::vector<float> & Z,
		std::vector<SourceModelPtr> & speakers);
};


