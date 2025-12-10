#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
	/// AUDIO setup
	AudioSetup();

	/// BRT Global Parametert setup
	globalParameters.SetSampleRate(SAMPLERATE); // Setting sample rate
	globalParameters.SetBufferSize(BUFFER_SIZE); // Setting buffer size

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
	vertices = { { 1, 0, 0 }, { -1, 0, 0 }, { 0, 1, 0 }, { 0, -1, 0 }, { 0, 0, 1 }, { 0, 0, -1 } };
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
	outputBufferStereo.left.resize(BUFFER_SIZE);
	outputBufferStereo.right.resize(BUFFER_SIZE);

	// Ensure ambisonic internal buffers have initial size
	W.resize(BUFFER_SIZE);
	X.resize(BUFFER_SIZE);
	Y.resize(BUFFER_SIZE);
	Z.resize(BUFFER_SIZE);

	// Inicializar botones (posiciones y tamaño)
	btnPlay.set(32, 52, 140, 40);
	btnStop.set(204, 52, 140, 40);

	btnRec.set(376, 52, 140, 40);
	btnStopRec.set(548, 52, 140, 40);

	btnPlayback.set(32, 112, 140, 40);
	btnStopPlayback.set(204, 112, 140, 40);

	// Checkbox MONITOR
	chkMonitor.set(376, 112, 20, 20);

	// Inicializar contadores
	bufferCounter = 0;
	drawCounter = 0;

	soundStream.start();
}

//--------------------------------------------------------------
void ofApp::update() {
}

//--------------------------------------------------------------
void ofApp::draw() {

	// Información superior
	ofFill();
	ofSetColor(225);
	ofDrawBitmapString("AMBI-ALICE", 32, 32);

	// -------------------------------------------
	// BOTONES ROSAS (rellenos)
	// -------------------------------------------
	auto drawButton = [&](ofRectangle & r, string label, bool hover, bool pressed, ofColor baseColor) {
		ofColor c = baseColor;

		if (hover && !pressed) {
			c += ofColor(40, 40, 40); 
		}

		if (pressed) {
			c = ofColor(255, 182, 193) ; 
		}

		ofNoFill();
		ofSetColor(c);
		ofDrawRectangle(r);

		ofSetColor(255); 
		ofDrawBitmapString(label, r.x + 15, r.y + 25);
	};

	ofColor pink(255, 20, 147);

	drawButton(btnPlay, "START AUDIO", hoverPlay, pressedPlay, pink);
	drawButton(btnStop, "STOP AUDIO", hoverStop, pressedStop, pink);

	drawButton(btnRec, "REC", hoverRec, pressedRec, pink);
	drawButton(btnStopRec, "STOP REC", hoverStopRec, pressedStopRec, pink);

	drawButton(btnPlayback, "PLAYBACK", hoverPlayback, pressedPlayback, pink);
	drawButton(btnStopPlayback, "STOP PLAY", hoverStopPlayback, pressedStopPlayback, pink);

	//--------------------------------------
	// CHECKBOX MONITOR
	//--------------------------------------
	auto drawCheckbox = [&](ofRectangle & r, bool hover, bool pressed, bool state) {
		ofColor base(255, 20, 147);
		ofColor c = base;
		if (hover && !pressed) c += ofColor(40, 40, 40);
		if (pressed) c = ofColor(255, 182, 193);

		ofFill();
		ofSetColor(c);
		ofDrawRectangle(r);

		if (state) {
			ofSetColor(255);
			ofDrawBitmapString("X", r.x + 4, r.y + 16);
		}

		ofSetColor(255);
		ofDrawBitmapString("B-FORMAT", r.x + 28, r.y + 16);
	};

	drawCheckbox(chkMonitor, chkMonitorHover, chkMonitorPressed, chkMonitorState);
	ofNoFill();

	if (chkMonitorState) {
		// draw the W channel:
		ofPushStyle();
		ofPushMatrix();
		ofTranslate(392 , 220, 0);

		ofSetColor(225);
		ofDrawBitmapString("W", 4, 18);

		ofSetLineWidth(1);
		ofDrawRectangle(0, 0,320 , 100);

		ofSetColor(245, 58, 135);
		ofSetLineWidth(3);

		ofBeginShape();
		for (unsigned int i = 0; i < W.size(); i++) {
			ofVertex(i * 1.25, 75 - W[i] * 180.0f);
		}
		ofEndShape(false);

		ofPopMatrix();
		ofPopStyle();

		// draw the X channel:
		ofPushStyle();
		ofPushMatrix();
		ofTranslate(32, 370, 0);

		ofSetColor(225);
		ofDrawBitmapString("X", 4, 18);

		ofSetLineWidth(1);
		ofDrawRectangle(0, 0, 320, 100);

		ofSetColor(245, 58, 135);
		ofSetLineWidth(3);

		ofBeginShape();
		for (unsigned int i = 0; i < X.size(); i++) {
			ofVertex(i *1.25, 75 - X[i] * 180.0f);
		}
		ofEndShape(false);

		ofPopMatrix();
		ofPopStyle();

		// draw the Y channel:
		ofPushStyle();
		ofPushMatrix();
		ofTranslate(392, 370, 0);

		ofSetColor(225);
		ofDrawBitmapString("Y", 4, 18);

		ofSetLineWidth(1);
		ofDrawRectangle(0, 0, 320, 100);

		ofSetColor(245, 58, 135);
		ofSetLineWidth(3);

		ofBeginShape();
		for (unsigned int i = 0; i < Y.size(); i++) {
			ofVertex(i *1.25, 75 - Y[i] * 180.0f);
		}
		ofEndShape(false);

		ofPopMatrix();
		ofPopStyle();

		// draw the Z channel:
		ofPushStyle();
		ofPushMatrix();
		ofTranslate(752, 370, 0);

		ofSetColor(225);
		ofDrawBitmapString("Z", 4, 18);

		ofSetLineWidth(1);
		ofDrawRectangle(0, 0, 320, 100);

		ofSetColor(245, 58, 135);
		ofSetLineWidth(3);

		ofBeginShape();
		for (unsigned int i = 0; i < Z.size(); i++) {
			ofVertex(i*1.25 , 75 - Z[i] * 180.0f);
		}
		ofEndShape(false);

		ofPopMatrix();
		ofPopStyle();
	} else {
		// draw the FRONT-LEFT-UP channel:
		ofPushStyle();
		ofPushMatrix();
		ofTranslate(32, 170, 0);

		ofSetColor(225);
		ofDrawBitmapString("FLU Channel", 4, 18);

		ofSetLineWidth(1);
		ofDrawRectangle(0, 0, 512, 150);

		ofSetColor(245, 58, 135);
		ofSetLineWidth(3);

		ofBeginShape();
		for (unsigned int i = 0; i < FLU.size(); i++) {
			ofVertex(i * 2, 100 - FLU[i] * 180.0f);
		}
		ofEndShape(false);

		ofPopMatrix();
		ofPopStyle();

		// draw the BACK-LEFT-DOWN channel:
		ofPushStyle();
		ofPushMatrix();
		ofTranslate(32, 370, 0);

		ofSetColor(225);
		ofDrawBitmapString("BLD Channel", 4, 18);

		ofSetLineWidth(1);
		ofDrawRectangle(0, 0, 512, 150);

		ofSetColor(245, 58, 135);
		ofSetLineWidth(3);

		ofBeginShape();
		for (unsigned int i = 0; i < BLD.size(); i++) {
			ofVertex(i * 2, 100 - BLD[i] * 180.0f);
		}
		ofEndShape(false);

		ofPopMatrix();
		ofPopStyle();

		// draw the BACK-RIGHT-UP channel:
		ofPushStyle();
		ofPushMatrix();
		ofTranslate(578, 170, 0);

		ofSetColor(225);
		ofDrawBitmapString("BRU Channel", 4, 18);

		ofSetLineWidth(1);
		ofDrawRectangle(0, 0, 512, 150);

		ofSetColor(245, 58, 135);
		ofSetLineWidth(3);

		ofBeginShape();
		for (unsigned int i = 0; i < BRU.size(); i++) {
			ofVertex(i * 2, 100 - BRU[i] * 180.0f);
		}
		ofEndShape(false);

		ofPopMatrix();
		ofPopStyle();

		// draw the FRONT-RIGHT-DOWN channel:
		ofPushStyle();
		ofPushMatrix();
		ofTranslate(578, 370, 0);

		ofSetColor(225);
		ofDrawBitmapString("FRD Channel", 4, 18);

		ofSetLineWidth(1);
		ofDrawRectangle(0, 0, 512, 150);

		ofSetColor(245, 58, 135);
		ofSetLineWidth(3);

		ofBeginShape();
		for (unsigned int i = 0; i < FRD.size(); i++) {
			ofVertex(i * 2, 100 - FRD[i] * 180.0f);
		}
		ofEndShape(false);

		ofPopMatrix();
		ofPopStyle();
	}

	// draw the RIGHT-STEREO channel:
	ofPushStyle();
	ofPushMatrix();
	ofTranslate(478, 570, 0);

	ofSetColor(225);
	ofDrawBitmapString("Right Channel", 4, 18);

	ofSetLineWidth(1);
	ofDrawRectangle(0, 0, 264, 75);

	ofSetColor(245, 58, 135);
	ofSetLineWidth(3);

	ofBeginShape();
	for (unsigned int i = 0; i < outputBufferStereo.right.size(); i++) {
		ofVertex(i , 50 - outputBufferStereo.right[i] * 180.0f);
	}
	ofEndShape(false);

	ofPopMatrix();
	ofPopStyle();

	// draw the LEFT-STEREO channel:
	ofPushStyle();
	ofPushMatrix();
	ofTranslate(760, 570, 0);

	ofSetColor(225);
	ofDrawBitmapString("Left Channel", 4, 18);

	ofSetLineWidth(1);
	ofDrawRectangle(0, 0, 264, 75);

	ofSetColor(245, 58, 135);
	ofSetLineWidth(3);

	ofBeginShape();
	for (unsigned int i = 0; i < outputBufferStereo.left.size(); i++) {
		ofVertex(i , 50 - outputBufferStereo.left[i] * 180.0f);
	}
	ofEndShape(false);

	ofPopMatrix();
	ofPopStyle();

	drawCounter++;

	ofSetColor(225);
	string reportString = "buffers received: " + ofToString(bufferCounter) + "\ndraw routines called: " + ofToString(drawCounter) + "\nticks: " + ofToString(soundStream.getTickCount());
	ofDrawBitmapString(reportString, 32, 589);
}

//--------------------------------------------------------------
void ofApp::audioIn(ofSoundBuffer & input) {

	int frames = input.getNumFrames();
	int channels = input.getNumChannels();

	// resize local arrays to frames
	FLU.assign(frames, 0.0f);
	FRD.assign(frames, 0.0f);
	BLD.assign(frames, 0.0f);
	BRU.assign(frames, 0.0f);

	W.resize(frames);
	X.resize(frames);
	Y.resize(frames);
	Z.resize(frames);

	if (channels == 1) {

		for (int i = 0; i < frames; i++) {

			float m = input[i] * 5;
			FLU[i] = m; // Front-Left-Up
			FRD[i] = m * 0.8f; // Front-Right-Down
			BLD[i] = m * 0.6f; // Back-Left-Down
			BRU[i] = m * 0.4f; // Back-Right-Up
		}

	} else {
		for (size_t i = 0; i < frames; i++) {
			FLU[i] = input[i * channels + 0] * 5;
			FRD[i] = input[i * channels + 1] * 5;
			BLD[i] = input[i * channels + 2] * 5;
			BRU[i] = input[i * channels + 3] * 5;
		}
	}

	for (int i = 0; i < frames; i++) {

		W[i] = (FLU[i] + FRD[i] + BLD[i] + BRU[i]);
		X[i] = (FLU[i] + FRD[i] - BLD[i] - BRU[i]);
		Y[i] = (FLU[i] - FRD[i] + BLD[i] - BRU[i]);
		Z[i] = (FLU[i] - FRD[i] - BLD[i] + BRU[i]);

		if (isRecording) {
			recordBuffer.push_back({ W[i], X[i], Y[i], Z[i] });
		}
	}
}

//--------------------------------------------------------------
void ofApp::audioOut(ofSoundBuffer & buffer) {
	unsigned int uiBufferSize = BUFFER_SIZE;
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
		for (int i = 0; i < bufferSize; ++i) {
			if (playPosition < recordBuffer.size()) {
				const AmbiFrame & f = recordBuffer[playPosition++];
				W[i] += f.W;
				X[i] += f.X;
				Y[i] += f.Y;
				Z[i] += f.Z;
			} else {
				isPlaying = false;
				playPosition = 0;
				break;
			}
		}
	}

	// Decodificar a speakers
	DecodeToSpeakerArray(W, X, Y, Z, speakers);

	// Procesar Binaural
	Common::CEarPair<CMonoBuffer<float>> bufferProcessed;
	brtManager.ProcessAll();
	listener->GetBuffers(bufferProcessed.left, bufferProcessed.right);

	bufferOutput.left += bufferProcessed.left;
	bufferOutput.right += bufferProcessed.right;
}

//--------------------------------------------------------------
void ofApp::AudioSetup() {

	ofSoundStreamSettings settings;
	settings.setApi(ofSoundDevice::Api::DEFAULT);

	// ------- SELECCIÓN DE DISPOSITIVO DE ENTRADA ------
	ofSoundDevice device = ShowSelectAudioDeviceMenu();
	settings.setInDevice(device);
	//settings.setOutDevice(device);
	// ----- SELECCIÓN DE DISPOSITIVO DE SALIDA ------
	auto outDevices = soundStream.getMatchingDevices("Default");
	if (!outDevices.empty()) {
		settings.setOutDevice(outDevices[0]);
	} else {
		ofLogError() << "No se encontro dispositivo de entrada";
	}

	settings.numOutputChannels = 2;
	settings.numInputChannels = 1;

	// ----------- PARÁMETROS GENERALES --------------
	settings.sampleRate = SAMPLERATE;
	settings.bufferSize = BUFFER_SIZE;
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
	if (key == 'w') {
		SaveToWav("ambisonics_WXYZ.wav");
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {
	hoverPlay = btnPlay.inside(x, y);
	hoverStop = btnStop.inside(x, y);
	hoverRec = btnRec.inside(x, y);
	hoverStopRec = btnStopRec.inside(x, y);
	hoverPlayback = btnPlayback.inside(x, y);
	hoverStopPlayback = btnStopPlayback.inside(x, y);

	chkMonitorHover = chkMonitor.inside(x, y);
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

	pressedPlay = btnPlay.inside(x, y);
	pressedStop = btnStop.inside(x, y);
	pressedRec = btnRec.inside(x, y);
	pressedStopRec = btnStopRec.inside(x, y);
	pressedPlayback = btnPlayback.inside(x, y);
	pressedStopPlayback = btnStopPlayback.inside(x, y);

	chkMonitorPressed = chkMonitor.inside(x, y);

	// Toggle checkbox state when pressed
	if (chkMonitorPressed) {
		chkMonitorState = !chkMonitorState;
		std::cout << "Monitor state: " << (chkMonitorState ? "ON" : "OFF") << std::endl;
	}

	if (pressedPlay) soundStream.start();
	if (pressedStop) soundStream.stop();

	if (pressedRec) {
		recordBuffer.clear();
		isRecording = true;
		std::cout << "Recording started\n";
	}

	if (pressedStopRec) {
		isRecording = false;
		std::cout << "Recording stopped\n";
	}

	if (pressedPlayback) {
		if (!recordBuffer.empty()) {
			isPlaying = true;
			playPosition = 0;
			std::cout << "Playback started\n";
		}
	}

	if (pressedStopPlayback) {
		isPlaying = false;
		playPosition = 0;
		std::cout << "Playback stopped\n";
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
	pressedPlay = false;
	pressedStop = false;
	pressedRec = false;
	pressedStopRec = false;
	pressedPlayback = false;
	pressedStopPlayback = false;

	chkMonitorPressed = false;
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

void ofApp::DecodeToSpeakerArray(const std::vector<float> & W, const std::vector<float> & X, const std::vector<float> & Y, const std::vector<float> & Z, std::vector<SourceModelPtr> & speakers) {

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

bool ofApp::SaveToWav(const std::string & filePath) {
	if (recordBuffer.empty()) {
		std::cout << "Error: recordBuffer está vacío." << std::endl;
		return false;
	}

	const uint32_t sampleRate = SAMPLERATE;
	const uint16_t numChannels = 4; // W X Y Z
	const uint16_t bitsPerSample = 32;
	const uint16_t audioFormat = 3; // 3 = IEEE float

	size_t numFrames = recordBuffer.size();

	// --- Interleave WXYZ ---
	std::vector<float> interleaved;
	interleaved.reserve(numFrames * numChannels);

	for (size_t i = 0; i < numFrames; i++) {
		interleaved.push_back(recordBuffer[i].W);
		interleaved.push_back(recordBuffer[i].X);
		interleaved.push_back(recordBuffer[i].Y);
		interleaved.push_back(recordBuffer[i].Z);
	}

	uint32_t dataSize = interleaved.size() * sizeof(float);
	uint32_t chunkSize = 36 + dataSize;
	uint32_t subchunk1Size = 16;
	uint32_t byteRate = sampleRate * numChannels * sizeof(float);
	uint16_t blockAlign = numChannels * sizeof(float);

	std::ofstream out(filePath, std::ios::binary);
	if (!out) {
		std::cout << "No se pudo abrir el archivo WAV." << std::endl;
		return false;
	}

	// RIFF HEADER
	out.write("RIFF", 4);
	out.write(reinterpret_cast<const char *>(&chunkSize), 4);
	out.write("WAVE", 4);

	// fmt chunk
	out.write("fmt ", 4);
	out.write(reinterpret_cast<const char *>(&subchunk1Size), 4);
	out.write(reinterpret_cast<const char *>(&audioFormat), 2);
	out.write(reinterpret_cast<const char *>(&numChannels), 2);
	out.write(reinterpret_cast<const char *>(&sampleRate), 4);
	out.write(reinterpret_cast<const char *>(&byteRate), 4);
	out.write(reinterpret_cast<const char *>(&blockAlign), 2);
	out.write(reinterpret_cast<const char *>(&bitsPerSample), 2);

	// data chunk
	out.write("data", 4);
	out.write(reinterpret_cast<const char *>(&dataSize), 4);
	out.write(reinterpret_cast<const char *>(interleaved.data()), dataSize);

	out.close();
	std::cout << "Archivo WAV guardado correctamente: " << filePath << std::endl;

	return true;
}
