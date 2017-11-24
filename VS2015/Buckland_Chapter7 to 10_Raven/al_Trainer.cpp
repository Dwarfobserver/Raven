
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
	trainingData.begin_ = pData_.get() + dataSize_ * trainingCount_;
	trainingData.end_ = pData_.get() + dataSize_ * dataCount_;

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

void Trainer::train(NeuralNetwork& nn) const {
	if (nn.config().inputsCount != dataSize_ - 1)
		throw std::runtime_error{ "The neural network has a wrong number of inputs for this trainer." };

	// Look for error margins
	const auto testing = [&, this] {
		double error = 0.0;
		for (int i = trainingCount_; i < dataCount_; ++i) {
			error += nn.evaluate(trainingData.begin() + i * dataSize_);
		}
		return error;
	};
	double newError = testing();
	double oldError = newError + 1;

	while (newError < oldError) {
		for (int passes = 0; passes < 10; ++passes) {
			// Training
			auto indices = randomCombination(trainingCount_);
			for (int i : indices) {
				const int shift = i * dataSize_;
				nn.train(trainingData.begin() + shift, pData_[shift + dataSize_ - 1]);
			}
			oldError = newError;
			newError = testing();
		}
	}
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
