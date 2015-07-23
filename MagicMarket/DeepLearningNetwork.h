#pragma once

#include <string>
#include <vector>

struct fann_train_data;
struct fann;

namespace CAPIWrapper
{
	void getOneTrainingDataSet(unsigned int num, unsigned int numInput, unsigned int numOutput, float *input, float *output);
};

namespace AI
{
	class DeepLearningNetwork
	{
		friend void ::CAPIWrapper::getOneTrainingDataSet(unsigned int num, unsigned int numInput, unsigned int numOutput, float *input, float *output);

	public:
		DeepLearningNetwork();
		virtual ~DeepLearningNetwork();

		void setLayerSetup(std::vector<int> setup) { layerSetup = setup; }
		void setTrainingInput(std::vector<std::vector<float>> &input) 
		{
			if (trainingInputForLayers.size() < 2) trainingInputForLayers.resize(2);
			trainingInputForLayers[1] = input;
		}
		void train() { executeTraining();  }
		::fann_train_data *getTrainingDataForLayer(int layer);
		
	private:
		void executeLayerTraining();
		void executeTraining();

		// The number of neurons in each of the hidden layers. Excluding the output layers.
		std::vector<int> layerSetup;
		// For training, save all the networks.
		std::vector<::fann*> layers;
		// Also in an encoder version.
		std::vector<::fann*> encoders;
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
		std::vector<std::vector<std::vector<float>>> trainingInputForLayers;
		std::vector<::fann_train_data*> trainingDatasetsForLayers;
	};

};