
#include "al_Training.h"
#include <random>


using namespace al;

std::vector<int> al::randomCombination(int max) {
	std::vector<int> ints(max);
	for (int i = 0; i < max; ++i) {
		ints[i] = i;
	}

	std::random_device rd;
	std::mt19937 gen(rd());
	const std::uniform_int_distribution<> dis;

	std::vector<int> result(max);
	int count = max;
	while (count--> 0) {
		const int indice = dis(gen) % (count + 1);
		result[count] = ints[indice];
		ints[indice] = ints[count];
	}
	return result;
}

PackedRecords::PackedRecords(Attributes attributes, std::vector<std::vector<Record> const*> pRecords) :
	attributes_{attributes}, dataCount_{0}, dataSize_{0}
{
	forEachIndice(attributes, [this](int) { ++dataSize_; });
	for (auto pVec : pRecords) {
		dataCount_ += pVec->size();
	}
	pData_ = std::make_unique<double[]>(dataSize_ * dataCount_);

	int i = 0;
	for (auto pVec : pRecords) {
		for (const auto& r : *pVec) {
			forEachIndice(attributes, [&](int indice) {
				pData_[i++] = r[indice];
			});
		}
	}

	normalizeData();
}

void PackedRecords::normalizeData() {
	std::vector<double> mins(dataSize_, 10000000000.0);
	std::vector<double> maxs(dataSize_, -10000000000.0);

	for (int shift = 0; shift < dataSize_; ++shift) {
		for (int i = 0; i < dataCount_; ++i) {
			const double val = pData_[i * dataSize_ + shift];
			if (val < mins[shift]) mins[shift] = val;
			if (val > maxs[shift]) maxs[shift] = val;
		}
	}
	for (auto i = 0u; i < mins.size(); ++i) {
		coefs_.push_back({ mins[i], maxs[i] });
	}
}

double al::train(NeuralNetwork& nn, PackedRecords const& records, TrainConfig const& config) {
	if (nn.config().inputsCount != records.dataSize() - 1)
		throw std::runtime_error{ "The neural network has a wrong number of inputs for this trainer." };

	for (int i = 0; i < records.dataSize() - 1; ++i) {
		nn.coef(i) = records.coef(i);
	}

	int trainSampleCount = static_cast<int>(config.trainingSampleRatio * records.dataCount());

	// Look for error margins
	const auto testing = [&] {
		int nbErrors = 0;
		for (int i = trainSampleCount; i < records.dataCount(); ++i) {
			double expected = records.begin()[(i + 1) * records.dataSize() - 1];
			double output = nn.evaluate(records.begin() + i * records.dataSize());
			if (abs(expected - output) >= 0.5) ++nbErrors;
		}
		return nbErrors;
	};
	int newError = testing();
	int oldError = newError + 1;

	while (newError < oldError
		&& newError > config.errorAccepted * records.dataCount())
	{
		for (int pass = 0; pass < config.passesBetweenChecks; ++pass) {
			// Training
			auto indices = randomCombination(trainSampleCount);
			for (int i : indices) {
				const int shift = i * records.dataSize();
				nn.train(
					records.begin() + shift,
					records.begin()[shift + records.dataSize() - 1],
					config.learningStep);
			}
		}
		oldError = newError;
		newError = testing();
	}
	return newError / static_cast<double>(records.dataCount());
}
