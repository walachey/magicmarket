#include "DeepLearningNetwork.h"
#include <sstream>
#include <cassert>

#include <memory>
#include <fstream>

#include <lwneuralnetplus\all.h>
#include <fann.h>

#include "Helpers.h"

namespace CAPIWrapper
{
	::AI::DeepLearningNetwork *learner(nullptr);
	int layer;

	void getOneTrainingDataSet(unsigned int num, unsigned int numInput, unsigned int numOutput, float *input, float *output)
	{
		assert(numInput == numOutput);
		assert(num >= 0 && num < learner->trainingInputForLayers[layer].size());
		// training the first hidden layer uses just the original input
		if (layer == 1)
		{
			std::vector<float> &inputs = learner->trainingInputForLayers[layer][num];
			memcpy(static_cast<void*>(input), static_cast<void*>(&learner->trainingInputForLayers[layer][num]), numInput);
			memcpy(static_cast<void*>(output), static_cast<void*>(&learner->trainingInputForLayers[layer][num]), numOutput);
		}
		else
		{
			assert(learner->trainingDatasetsForLayers.size() > static_cast<size_t>(layer - 1));

			// otherwise we will need to run the last input through the next layer
			assert(learner->trainingInputForLayers[layer - 1].size() > num);
			std::vector<float> &lastInputSet = learner->trainingInputForLayers[layer - 1][num];
			assert(lastInputSet.size() == learner->layerSetup[layer - 2]);

			fann *encoder = learner->encoders[layer - 1];
			const int outputLength = fann_get_num_output(encoder);
			assert(outputLength == learner->layerSetup[layer - 1]);

			float *output = fann_run(encoder, lastInputSet.data());
			std::vector<float> newOutput;
			newOutput.resize(outputLength);
			memcpy(static_cast<void*>(newOutput.data()), static_cast<void*>(output), newOutput.size());
			learner->trainingInputForLayers[layer].push_back(newOutput);
		}
	};
};

namespace AI
{
	DeepLearningNetwork::DeepLearningNetwork()
	{
		// Don't train the input layer..
		currentLayer = 1;
		// This makes indexing the arrays more straight-forward later.
		encoders.push_back(nullptr);
		trainingInputForLayers.push_back({});
		trainingDatasetsForLayers.push_back(nullptr);

		networkConfiguration.maxNumberOfEpochs = 2000;
		networkConfiguration.minimumError = 0.01f;
	}


	DeepLearningNetwork::~DeepLearningNetwork()
	{
	}

	void DeepLearningNetwork::executeTraining()
	{
		for (; currentLayer < static_cast<int>(layerSetup.size()); ++currentLayer)
		{
			executeLayerTraining();
		}
	}

	void DeepLearningNetwork::executeLayerTraining()
	{
		std::cout << "ANN Training Layer " << currentLayer << std::endl;
		// One layer always has exactly three layers in the training network.
		const int &currentLayerNeuronCount = layerSetup[currentLayer];
		const int &inputLayerNeuronCount = layerSetup[currentLayer - 1];
		
		::fann *layerNet = fann_create_standard(3, inputLayerNeuronCount, currentLayerNeuronCount, inputLayerNeuronCount);
		fann_set_activation_function_hidden(layerNet, FANN_SIGMOID_SYMMETRIC);
		fann_set_activation_function_output(layerNet, FANN_SIGMOID_SYMMETRIC);
		layers.push_back(layerNet);

		struct fann_train_data *data = getTrainingDataForLayer(currentLayer);
		fann_set_train_error_function(layerNet, FANN_ERRORFUNC_TANH);
		fann_set_train_stop_function(layerNet, FANN_STOPFUNC_MSE);
		fann_train_on_data(layerNet, data, networkConfiguration.maxNumberOfEpochs, 100, networkConfiguration.minimumError);

		std::ostringstream outname; outname << "saves/deeplearning/network." << currentLayer << ".fann";
		fann_save(layerNet, outname.str().c_str());
		
		// Now fetch the neuron setup of the trained layer and remember it..
		auto saveWeights = [&](int layer, std::vector<std::vector<float>> &target, struct fann **encoder)
		{
			std::unique_ptr<unsigned int> layerLayout(static_cast<unsigned int*>(malloc(sizeof(unsigned int) * fann_get_num_layers(layerNet))));
			std::unique_ptr<unsigned int> biasLayout (static_cast<unsigned int*>(malloc(sizeof(unsigned int) * fann_get_num_layers(layerNet))));
			fann_get_layer_array(layerNet, layerLayout.get());
			fann_get_bias_array(layerNet, biasLayout.get());
			const int neuronCount[] = { layerLayout.get()[layer], layerLayout.get()[layer + 1]};
			const int neuronCountWitBiases[] = { layerLayout.get()[layer] + biasLayout.get()[layer], layerLayout.get()[layer + 1] + biasLayout.get()[layer + 1] };

			std::vector<float> weights;
			assert((layer == 0 && neuronCount[0] > neuronCount[1]) || (layer == 1 && neuronCount[0] < neuronCount[1]));
			weights.resize(neuronCountWitBiases[0] * neuronCountWitBiases[1]);
			fann_get_weights_for_layer(layerNet, &weights[0], layer, weights.size());

			if (encoder != nullptr)
			{
				(*encoder) = fann_create_standard(2, neuronCount[0], neuronCount[1]);
				assert(layerSetup.size() == currentLayer + 1 || fann_get_num_output(*encoder) == layerSetup[currentLayer]);
				assert(fann_get_num_input(*encoder) == layerSetup[currentLayer - 1]);
				fann_set_weights_for_layer(*encoder, &weights[0], 0);
			}
			
			target.push_back(weights);
		};
		struct fann *encoder;
		saveWeights(0, encoderWeights, &encoder);
		encoders.push_back(encoder);
		saveWeights(1, decoderWeights, nullptr);;

	}

	struct fann_train_data * DeepLearningNetwork::getTrainingDataForLayer(int layer)
	{
		if (trainingDatasetsForLayers.size() > static_cast<size_t>(layer)) return trainingDatasetsForLayers[layer];
		
		// wrapper magic...
		::CAPIWrapper::layer = layer;
		::CAPIWrapper::learner = this;
		// generate training input..
		fann_train_data *data = fann_create_train_from_callback(trainingInputForLayers[1].size(), layerSetup[layer - 1], layerSetup[layer - 1], &::CAPIWrapper::getOneTrainingDataSet);
		assert(data->num_output == data->num_input);
		trainingDatasetsForLayers.push_back(data);
		return data;
	}
};
