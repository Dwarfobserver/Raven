
#pragma once

#include "al_Record.h"
#include <memory>
#include "al_NeuralNetwork.h"


namespace al {

	class Trainer {
	public:
		Trainer(Attributes attributes, std::vector<std::vector<Record> const*> pRecords);

		void train(NeuralNetwork& nn) const;
		static std::vector<int> randomCombination(int max);

		int dataCount() const { return dataCount_; }
		int dataSize() const { return dataSize_; }

		struct DataSpan {
			friend class Trainer;
			double const* begin() const { return begin_; }
			double const* end() const { return end_; }
		private:
			double const* begin_;
			double const* end_;
		};
		DataSpan trainingData;
	private:
		Attributes attributes_;
		int dataCount_;
		int dataSize_;
		std::unique_ptr<double[]> pData_;
		int trainingCount_;

		void normalizeData();
	};

}
