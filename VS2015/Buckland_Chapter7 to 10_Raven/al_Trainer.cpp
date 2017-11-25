
#include "al_Trainer.h"
#include <random>


using namespace al;

Trainer::Trainer(Attributes attributes, std::vector<std::vector<Record> const*> pRecords) :
	attributes_{ attributes }, dataCount_{ 0 }, dataSize_{ 0 }
{
	forEachIndice(attributes, [this](int) { ++dataSize_; });
	for (auto pVec : pRecords) {
		dataCount_ += pVec->size();
	}
	pData_ = std::make_unique<double[]>(dataSize_ * dataCount_);

	trainingCount_ = dataCount_ * 2 / 3;

	int i = 0;
	for (auto pVec : pRecords) {
		for (const auto& r : *pVec) {
			forEachIndice(attributes, [&] (int indice) {
				pData_[i++] = r[indice];
			});
		}
	}

	normalizeData();
}

double Trainer::train(NeuralNetwork& nn, double errorAccepted, int passesBetweenChecks) const {
	if (nn.config().inputsCount != dataSize_ - 1)
		throw std::runtime_error{ "The neural network has a wrong number of inputs for this trainer." };

	// Look for error margins
	const auto testing = [&, this] {
		double error = 0.0;
		for (int i = trainingCount_; i < dataCount_; ++i) {
			double expected = pData_[(i + 1) * dataSize_ - 1];
			double output = nn.evaluate(pData_.get() + i * dataSize_);
			error += std::abs(expected - output);
		}
		return error;
	};
	double newError = testing();
	double oldError = newError + 1;

	while (newError < oldError && newError > errorAccepted * dataCount_) {
		for (int pass = 0; pass < passesBetweenChecks; ++pass) {
			// Training
			auto indices = randomCombination(trainingCount_);
			for (int i : indices) {
				const int shift = i * dataSize_;
				double d1 = pData_[shift];
				double d2 = pData_[shift + 1];
				double d3 = pData_[shift + 2];
				nn.train(pData_.get() + shift, pData_[shift + dataSize_ - 1]);
			}
		}
		oldError = newError;
		newError = testing();
	}
	return newError / dataCount_;
}

std::vector<int> Trainer::randomCombination(int max) {
	std::vector<int> ints(max);
	for (int i = 0; i < max; ++i) {
		ints[i] = i;
	}

	std::random_device rd;
	std::mt19937 gen(rd());
	const std::uniform_int_distribution<> dis;

	std::vector<int> result(max);
	int count = max;
	while (count --> 0) {
		const int indice = dis(gen) % (count + 1);
		result[count] = ints[indice];
		ints[indice] = ints[count];
	}
	return result;
}

void Trainer::normalizeData() {
	std::vector<double> mins(dataSize_, 10000000000.0);
	std::vector<double> maxs(dataSize_, -10000000000.0);

	for (int shift = 0; shift < dataSize_; ++shift) {
		for (int i = 0; i < dataCount_; ++i) {
			const double val = pData_[i * dataSize_ + shift];
			if (val < mins[shift]) mins[shift] = val;
			if (val > maxs[shift]) maxs[shift] = val;
		}
	}

	for (int shift = 0; shift < dataSize_; ++shift) {
		const double scale = maxs[shift] - mins[shift];
		const double offset = -mins[shift];
		for (int i = 0; i < dataCount_; ++i) {
			double& val = pData_[i * dataSize_ + shift];
			val = (val + offset) / scale;
		}
	}
}
