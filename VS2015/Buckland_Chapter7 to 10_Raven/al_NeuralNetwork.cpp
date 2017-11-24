
#include "al_NeuralNetwork.h"
#include "genann/genann.h"


using namespace al;

NeuralNetwork::NeuralNetwork(Config const& config) :
	config_{config},
	pGenann_{ genann_init(
		config.inputsCount,
		config.layersCount,
		config.layerSize,
		1), genann_free}
{}

void NeuralNetwork::train(double const* inputs, double output) {
	genann_train(pGenann_.get(), inputs, &output, config_.learningStep);
}

double NeuralNetwork::evaluate(double const* inputs) const {
	const auto pResults = genann_run(pGenann_.get(), inputs);
	return *pResults;
}
