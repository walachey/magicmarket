#include "DeepLearningNetwork.h"
#include <sstream>
#include <cassert>

#include <fstream>

#include <lwneuralnetplus\all.h>

#include "Helpers.h"

namespace AI
{
	DeepLearningNetwork::DeepLearningNetwork()
	{
		// Don't train the input layer..
		currentLayer = 1;

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
		// One layer always has exactly three layers in the training network.
		const int &currentLayerNeuronCount = layerSetup[currentLayer];
		const int &inputLayerNeuronCount = layerSetup[currentLayer - 1];
		ANN::network *layerNet = new ANN::network(ANN::network::LOGISTIC, 3, inputLayerNeuronCount, currentLayerNeuronCount, inputLayerNeuronCount);
		layerNet->randomize(0.5);
		layers.push_back(layerNet);

		std::ostringstream errorFilename;
		errorFilename << "saves/deeplearning/error." << currentLayer << ".log";
		std::ostringstream accuracyFilename;
		accuracyFilename << "saves/deeplearning/accuracy." << currentLayer << ".log";
		ANN::trainer layerTrainer(layerNet, errorFilename.str(), errorFilename.str());
		layerTrainer.set_max_epochs(networkConfiguration.maxNumberOfEpochs);
		layerTrainer.set_min_error(networkConfiguration.minimumError);

		const char dataID[2] = { currentLayer, '\0' };
		layerTrainer.set_iomanager(this);
		layerTrainer.load_training(dataID);
		layerTrainer.train_online(true);
		std::ostringstream outname; outname << "saves/deeplearning/weights." << currentLayer << ".txt";
		std::fstream out(outname.str().c_str(), std::ios_base::out);
		// Now fetch the neuron setup of the trained layer and remember it..
		auto saveWeights = [&](int layer, std::vector<std::vector<float>> &target, ANN::network **encoder)
		{
			std::vector<float> weights;
			const int neuronCount[] = { layerNet->get_no_of_neurons(layer), layerNet->get_no_of_neurons(layer + 1) };
			assert((layer == 0 && neuronCount[0] > neuronCount[1]) || (layer == 1 && neuronCount[0] < neuronCount[1]));
			weights.reserve(neuronCount[0] * neuronCount[1]);
			
			if (encoder != nullptr)
			{
				(*encoder) = new ANN::network(ANN::network::LOGISTIC, 2, neuronCount[0], neuronCount[1]);
			}

			for (int i = 0; i < neuronCount[0]; ++i)
			{
				for (int j = 0; j < neuronCount[1]; ++j)
				{
					weights.push_back(layerNet->get_weight(layer + 1, i, j));
					if (encoder != nullptr)
					{
						(*encoder)->set_weight(layer + 1, i, j, weights.back());
						out << weights.back() << ",\t";
					}
				}
				out << std::endl;
			}
			target.push_back(weights);
		};
		ANN::network *encoder;
		saveWeights(0, encoderWeights, &encoder);
		encoders.push_back(encoder);
		saveWeights(1, decoderWeights, nullptr);

		std::cout << "TRAINING EPOCH\t" << layerTrainer.get_current_epoch() << std::endl;
		layerTrainer.test();
		std::cout << "TRAINING ERROR\t" << layerTrainer.get_error_on_training() << std::endl;
		std::cout << "TRAINING ACCURACY\t" << layerTrainer.get_accuracy_on_training() << std::endl;

		std::ostringstream graphname; graphname << "saves/deeplearning/net." << currentLayer << ".dot";
		layerNet->export_to_graphviz(graphname.str().c_str());
	}

	void DeepLearningNetwork::info_from_file(const std::string & filename, int *npatterns, int *ninput, int *noutput)
	{
		const char &layerChar = filename.data()[0];
		const int layer = static_cast<int>(layerChar);
		assert(layer > 0 && layer < static_cast<int>(layerSetup.size()));

		*npatterns = trainingInput.size();
		*ninput = layerSetup[layer - 1];
		*noutput = *ninput;
	}

	void DeepLearningNetwork::load_patterns(const std::string & filename, float **inputs, float **targets, int ninput, int noutput, int npatterns)
	{
		const char &layerChar = filename.data()[0];
		const int layer = static_cast<int>(layerChar);
		assert(layer > 0 && layer < static_cast<int>(layerSetup.size()));
		assert(ninput == noutput);
		std::cout << "LOADING PATTERNRS ---------\n\tlayer " << layer << std::endl;

		for (int i = 0; i < npatterns; ++i)
		{
			if (layer == 1) // first layer, train by input
			{
				for (int in = 0; in < ninput; ++in)
					inputs[i][in] = targets[i][in] = trainingInput[i][in];
			}
			else
			{
				// other layers, train by output of previous layers
				std::vector<float> output(trainingInput[i].begin(), trainingInput[i].end());
				for (int previousLayer = 1; previousLayer < layer; ++previousLayer)
				{
					assert(static_cast<int>(encoders.size()) >= previousLayer);
					assert(layerSetup[previousLayer] >= ninput);
					ANN::network *network = encoders[previousLayer - 1];
					assert(network->get_no_of_inputs() == static_cast<int>(output.size()));
					
					std::vector<float> newOutput;
					newOutput.resize(layerSetup[previousLayer]);
					assert(network->get_no_of_outputs() == static_cast<int>(newOutput.size()));

					network->compute(output.data(), newOutput.data());
					output = newOutput;
				}
				assert(output.size() == ninput);
				std::cout << "VAR\t" << ::MM::Math::stddev(output) << std::endl;
				std::cout << "\t"; for (auto &f : output) std::cout << f << ",\t"; std::cout << std::endl;
				for (int in = 0; in < ninput; ++in)
					inputs[i][in] = targets[i][in] = output[in];
			}
		}
	}
};