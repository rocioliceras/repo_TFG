#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
	/// AUDIO setup
	AudioSetup();

	/// BRT Global Parametert setup
	globalParameters.SetSampleRate(SAMPLERATE); // Setting sample rate
	globalParameters.SetBufferSize(256); // Setting buffer size

	// //////////////////////////
	// // Listener Setup
	// //////////////////////////
	brtManager.BeginSetup();
	listener = brtManager.CreateListener<BRTBase::CListener>(LISTENER_ID);
	brtManager.EndSetup();
	Common::CTransform listenerPosition = Common::CTransform(); // Setting listener in (0,0,0)
	listenerPosition.SetPosition(Common::CVector3(0.0f, 0.0f, 0.0f));
	listener->SetListenerTransform(listenerPosition);

	/////////////////////////////////////
	// Create and connnect BRT modules
	/////////////////////////////////////
	configurationA.Setup(&brtManager, LISTENER_ID);
	configurationA.LoadResources(&brtManager, LISTENER_ID);

	/////////////////////
	// Create Sources
	/////////////////////
	vertices = {{ 1, 0, 0 }, { -1, 0, 0 }, { 0, 1, 0 }, { 0, -1, 0 }, { 0, 0, 1 }, { 0, 0, -1 }};
	speakers.resize(vertices.size());
	for (size_t i = 0; i < vertices.size(); ++i) {
		speakers[i] = CreateOmnidirectionalSoundSource(brtManager, "Speaker" + std::to_string(i));
		speakers[i]->SetSourceTransform(Common::CTransform(vertices[i]));
	}
	for (int i = 0; i < speakers.size(); i++) {
		configurationA.ConnectSoundSource(&brtManager, "Speaker" + std::to_string(i));
	}

	/////////////////////
	// Start AUDIO Render
	/////////////////////
	// Declaration and initialization of stereo buffer
	outputBufferStereo.left.resize(512);
	outputBufferStereo.right.resize(512);

	soundStream.start();
}

//--------------------------------------------------------------
void ofApp::update() {

}

//--------------------------------------------------------------
void ofApp::draw() {
	ofBackground(0);
	ofSetColor(255);

	ofDrawBitmapString("Press 's' to Start Audio, 'e' to Stop Audio", 20, 20);
	ofDrawBitmapString("'r' Start Recording, 't' Stop Recording", 20, 40);
	ofDrawBitmapString("'p' Start Playback, 'o' Stop Playback", 20, 60);

	std::string status = "Status: ";
	status += isRecording ? "[Recording] " : "";
	status += isPlaying ? "[Playing] " : "";
	ofDrawBitmapString(status, 20, 100);
}

//--------------------------------------------------------------
void ofApp::audioIn(ofSoundBuffer & input) {

	int frames = input.getNumFrames(); 
	int channels = input.getNumChannels();

	FLU.assign(frames, 0.0f);
	FRD.assign(frames, 0.0f);
	BLD.assign(frames, 0.0f);
	BRU.assign(frames, 0.0f);

	W.resize(frames);
	X.resize(frames);
	Y.resize(frames);
	Z.resize(frames);

	float norm = 0.25f;

	for (int i = 0; i < frames; i++) {
		/* FLU[i] = input[i * channels + 0]; // entrada 1
		FRD[i] = input[i * channels + 1]; // entrada 2
		BLD[i] = input[i * channels + 2]; // entrada 3
		BRU[i] = input[i * channels + 3]; // entrada 4

		W[i] = (FLU[i] + FRD[i] + BLD[i] + BRU[i]) * norm;
		X[i] = (FLU[i] + FRD[i] - BLD[i] - BRU[i]) * norm;
		Y[i] = (-FLU[i] + FRD[i] - BLD[i] + BRU[i]) * norm;
		Z[i] = (FLU[i] - FRD[i] - BLD[i] + BRU[i]) * norm;*/


		FLU[i] = input[i]; 
		FRD[i] = input[i];
		BLD[i] = input[i];
		BRU[i] = input[i];

		W[i] = (FLU[i] + FRD[i] + BLD[i] + BRU[i]) ;
		X[i] = 0.0f;
		Y[i] = 0.0f;
		Z[i] = 0.0f;

		if (isRecording) {
			recordBuffer.push_back(W[i]); 
		}
	}

}

void ofApp::audioOut(ofSoundBuffer & buffer) {
	unsigned int uiBufferSize = 256;
	// Setting the output buffer as float
	std::vector<float> & bufferData = buffer.getBuffer();

	//Pointer to the first element of the vector
	float * floatOutputBuffer = &bufferData[0];

	//Overflow/underflow
	if (buffer.getNumFrames() == 0) {
		std::cout << "Stream underflow detected!" << std::endl;
	}

	// Initializes buffer with zeros
	outputBufferStereo.left.Fill(uiBufferSize, 0.0f);
	outputBufferStereo.right.Fill(uiBufferSize, 0.0f);

	audioProcess(outputBufferStereo, uiBufferSize);

	// Declaration and initialization of interlaced audio vector for correct stereo output
	CStereoBuffer<float> iOutput;
	iOutput.Interlace(outputBufferStereo.left, outputBufferStereo.right);

	// Buffer filling loop
	for (auto it = iOutput.begin(); it != iOutput.end(); it++) {
		floatOutputBuffer[0] = *it; // Setting of value in actual buffer position
		floatOutputBuffer = &floatOutputBuffer[1]; // Updating pointer to next buffer position
	}
}

void ofApp::audioProcess(Common::CEarPair<CMonoBuffer<float>> & bufferOutput, int bufferSize) {

	if (isPlaying && !recordBuffer.empty()) {
		for (unsigned int i = 0; i < bufferSize; i++) {
			if (playPosition < recordBuffer.size()) {
				float sample = recordBuffer[playPosition++];
				outputBufferStereo.left[i] += sample;
				outputBufferStereo.right[i] += sample;
			}
		}
		// Si llegamos al final, paramos la reproducción
		if (playPosition >= recordBuffer.size()) {
			isPlaying = false;
			playPosition = 0;
		}
	}

	DecodeToSpeakerArray(W, X, Y, Z, speakers);

	Common::CEarPair<CMonoBuffer<float>> bufferProcessed;

	brtManager.ProcessAll();
	listener->GetBuffers(bufferProcessed.left, bufferProcessed.right);

	bufferOutput.left += bufferProcessed.left;
	bufferOutput.right += bufferProcessed.right;
}


void ofApp::AudioSetup() {

	ofSoundStreamSettings settings;
	settings.setApi(ofSoundDevice::Api::MS_ASIO); 


	// ------- SELECCIÓN DE DISPOSITIVO DE ENTRADA ------
	ofSoundDevice device = ShowSelectAudioDeviceMenu();
	settings.setInDevice(device);
	settings.setOutDevice(device); 

	// ----- SELECCIÓN DE DISPOSITIVO DE SALIDA ------
	/* auto outDevices = soundStream.getMatchingDevices("Default");
	if (!outDevices.empty()) {
		settings.setOutDevice(outDevices[0]);
	} else {
		ofLogError() << "No se encontro dispositivo de entrada";
	}*/

	//settings.numInputChannels = 4;

	settings.numOutputChannels = 2;
	settings.numInputChannels = 1; 

	// ----------- PARÁMETROS GENERALES --------------
	settings.sampleRate = SAMPLERATE; 
	settings.bufferSize = 256;
	settings.numBuffers = 2;

	settings.setInListener(this);
	settings.setOutListener(this);

	try {
		soundStream.setup(settings);
		soundStream.stop(); 
	} catch (const std::exception & e) {
		std::cout << "Error al iniciar audio: " << e.what() << std::endl;
	}
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	if (key == 's') soundStream.start();
	if (key == 'e') soundStream.stop();

	if (key == 'r') { // 'r' para grabar
		recordBuffer.clear();
		isRecording = true;
		std::cout << "Recording started\n";
	}
	if (key == 't') { // 't' para parar la grabación
		isRecording = false;
		std::cout << "Recording stopped\n";
	}
	if (key == 'p') { // 'p' para reproducir
		if (!recordBuffer.empty()) {
			isPlaying = true;
			playPosition = 0;
			std::cout << "Playback started\n";
		}
	}
	if (key == 'o') { // 'o' para parar la reproducción
		isPlaying = false;
		playPosition = 0;
		std::cout << "Playback stopped\n";
	}
}


//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {
}

	/// Function to show the user a menu to choose the audio output device
ofSoundDevice ofApp::ShowSelectAudioDeviceMenu() {

	auto devices = soundStream.getDeviceList();
	std::cout << "Number of audio devices found: " << devices.size() << std::endl;

	std::cout << std::endl
			  << std::endl;
	std::cout << "----------------------------------------" << std::endl;
	std::cout << "     List of available audio inputs     " << std::endl;
	std::cout << "----------------------------------------" << std::endl
			  << std::endl;

	for (int i = 0; i < devices.size(); ++i) {
		std::cout << "Device #" << i << ": " << devices[i].name << std::endl;
		std::cout << "Output channels: " << devices[i].outputChannels << " ";
		std::cout << "Input channels: " << devices[i].inputChannels << " ";
		std::cout << "Default output: " << (devices[i].isDefaultOutput ? "Yes" : "No") << std::endl;
		std::cout << std::endl;
	}

	std::cout << std::endl;
	int selectAudioDevice;
	do {
		std::cout << "Please choose which audio output you wish to use: ";
		std::cin >> selectAudioDevice;
		std::cin.clear();
		std::cin.ignore(INT_MAX, '\n');
	} while (!(selectAudioDevice > -1 && selectAudioDevice <= devices.size()));
	std::cout << std::endl;
	return devices[selectAudioDevice];
}

void ofApp::DecodeToSpeakerArray(const std::vector<float> & W,const std::vector<float> & X,const std::vector<float> & Y,const std::vector<float> & Z,std::vector<SourceModelPtr> & speakers) {

	size_t numSamples = W.size();
	float Wgain = 1.0f / std::sqrt(speakers.size());
	float XYZgain = std::sqrt(3.0f / 2.0f) / std::sqrt(speakers.size());

	for (size_t i = 0; i < speakers.size(); i++) {
		Common::CVector3 pos = speakers[i]->GetSourceTransform().GetPosition();
		CMonoBuffer<float> buffer(numSamples, 0.0f);

		for (size_t n = 0; n < numSamples; n++) {
			buffer[n] = Wgain * W[n] + XYZgain * (pos.x * X[n] + pos.y * Y[n] + pos.z * Z[n]);
		}

		speakers[i]->SetBuffer(buffer);
	}
}
