
#include "al_NeuralNetwork.h"
#include "genann/genann.h"
#include "al_Record.h"


using namespace al;

NeuralNetwork::NeuralNetwork(Config const& config) :
	config_{config},
	pGenann_{ genann_init(
		config.inputsCount,
		config.layersCount,
		config.layerSize,
		1), genann_free}
{}

void NeuralNetwork::train(double const* inputs, double output, double learningStep) {
	genann_train(pGenann_.get(), inputs, &output, learningStep);
}

double NeuralNetwork::evaluate(double const* inputs) const {
	const auto pResults = genann_run(pGenann_.get(), inputs);
	return *pResults;
}

double NeuralNetwork::evaluate(Record const& record) const {
	double inputs[7];
	int indice = 0;
	forEachIndice(record.attributes(), [&indice, &inputs, &record] (int i) {
		inputs[indice++] = record[i];
	});
	return evaluate(inputs);
}
