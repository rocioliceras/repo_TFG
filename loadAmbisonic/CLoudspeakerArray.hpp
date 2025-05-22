#ifndef _C_LOUDSPEAKER_ARRAY_HPP_
#define _C_LOUDSPEAKER_ARRAY_HPP_

#include "ofxAudioFile.h"
#include <vector>

class CLoudSpeakerArray {
public:
	struct Speaker {
		float azimuth; // en radianes
		float elevation; // en radianes
	};

	//Dependiendo de lo que se elija (octaedro,cubo,icosaedro o dodecaedro) se colocan las fuentes
	void loadLayoutJSON(std::string name) {
	/*
		if (name == "icosaedro") {
			// Cargar el .json del icosaedro
			speakers.clear();
			speakers.push_back({azimut1, elevacion1 });
			speakers.push_back({ azimut2 ,  elevacion2 });
			...
		}
	*/
	}
	const std::vector<Speaker> & getSpeakers() const;

	int getNumSpeakers() const { return speakers.size(); }

private:
	std::vector<Speaker> speakers;
};


#endif // _C_LOUDSPEAKER_ARRAY_HPP_
