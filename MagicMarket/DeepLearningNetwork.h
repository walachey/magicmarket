#pragma once

#include <string>
#include <vector>

#include <lwneuralnetplus\all.h>

namespace AI
{
	class DeepLearningNetwork : private ANN::iomanage
	{
	public:
		DeepLearningNetwork();
		virtual ~DeepLearningNetwork();
		/*
			iomanage: must always present the current layer.
		*/
		// implement methods to server as a trainer for the neural networks
		virtual void info_from_file(const std::string & filename, int *npatterns, int *ninput, int *noutput);
		virtual void load_patterns(const std::string & filename, float **inputs, float **targets, int ninput, int noutput, int npatterns);

		void setLayerSetup(std::vector<int> setup) { layerSetup = setup; }
		void setTrainingInput(std::vector<std::vector<float>> &input) { trainingInput = input; };
		void train() { executeTraining();  }
	private:
		void executeLayerTraining();
		void executeTraining();

		// The number of neurons in each of the hidden layers. Excluding the output layers.
		std::vector<int> layerSetup;
		// For training, save all the networks.
		std::vector<ANN::network*> layers;
		// Also in an encoder version.
		std::vector<ANN::network*> encoders;
		// The current layer that is to be trained.
		int currentLayer;

		struct {
			float minimumError;
			int maxNumberOfEpochs;

		} networkConfiguration;
		// For composing the final network.
		std::vector<std::vector<float>> encoderWeights;
		std::vector<std::vector<float>> decoderWeights;
		// For training the first layer. Other input will be derived.
		std::vector<std::vector<float>> trainingInput;
	};

};