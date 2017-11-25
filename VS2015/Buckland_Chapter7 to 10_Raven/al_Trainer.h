
#pragma once

#include "al_Record.h"
#include <memory>
#include "al_NeuralNetwork.h"


namespace al {

	class Trainer {
	public:
		Trainer(Attributes attributes, std::vector<std::vector<Record> const*> pRecords);

		double train(NeuralNetwork& nn, double errorAccepted = 0.01, int passesBetweenChecks = 1000) const;
		static std::vector<int> randomCombination(int max);

		int dataCount() const { return dataCount_; }
		int dataSize() const { return dataSize_; }

		double const* begin() const { return pData_.get(); }
		double const* end() const { return pData_.get() + dataCount_ * dataSize_; }
	private:
		Attributes attributes_;
		int dataCount_;
		int dataSize_;
		std::unique_ptr<double[]> pData_;
		int trainingCount_;

		void normalizeData();
	};

}
