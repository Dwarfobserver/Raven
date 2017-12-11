
#include "al_NeuralNetwork.h"
#include "genann/genann.h"
#include "al_Record.h"


using namespace al;

NeuralNetwork::NeuralNetwork(Config const& config) :
	config_{config},
	coefs_(config_.inputsCount, { 1.0, 1.0 }),
	pGenann_{ genann_init(
		config.inputsCount,
		config.layersCount,
		config.layerSize,
		1), genann_free}
{}

void NeuralNetwork::train(double const* inputs, double output, double learningStep) {
	double inputsCopy[7];
	auto i = 0u;
	for (; i < coefs_.size(); ++i) {
		inputsCopy[i] = (inputs[i] - coefs_[i].first) / (coefs_[i].second - coefs_[i].first);
	}
	inputsCopy[i] = inputs[i];
	genann_train(pGenann_.get(), inputsCopy, &output, learningStep);
}

double NeuralNetwork::evaluate(double const* inputs) const {
	double inputsCopy[7];
	for (auto i = 0u; i < coefs_.size(); ++i) {
		inputsCopy[i] = (inputs[i] - coefs_[i].first) / (coefs_[i].second - coefs_[i].first);
	}
	const auto pResults = genann_run(pGenann_.get(), inputsCopy);
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
