
#pragma once

#include "al_Record.h"
#include "al_NeuralNetwork.h"
#include <memory>


namespace al {

	std::vector<int> randomCombination(int max);

	class PackedRecords {
	public:
		PackedRecords(Attributes attributes, std::vector<std::vector<Record> const*> pRecords);

		Attributes attributes() const { return attributes_; }

		int dataCount() const { return dataCount_; }
		int dataSize() const { return dataSize_; }

		double const* begin() const { return pData_.get(); }
		double const* end() const { return pData_.get() + dataCount_ * dataSize_; }

		std::pair<double, double> coef(int i) const { return coefs_[i]; }
	private:
		void normalizeData();

		Attributes attributes_;
		int dataCount_;
		int dataSize_;
		std::vector<std::pair<double, double>> coefs_;
		std::unique_ptr<double[]> pData_;
	};

	struct TrainConfig {
		double errorAccepted;
		int passesBetweenChecks;
		double learningStep;
		double trainingSampleRatio;
	};

	double train(NeuralNetwork& nn, PackedRecords const& records, TrainConfig const& config);
}
