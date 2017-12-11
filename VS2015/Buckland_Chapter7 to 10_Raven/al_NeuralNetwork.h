
#pragma once

#include <memory>
#include <vector>


struct genann;

namespace al {

	class Record;
	class NeuralNetwork {
	public:
		struct Config {
			int inputsCount;
			int layersCount;
			int layerSize;
		};
		explicit NeuralNetwork(Config const& config);
		
		void train(double const* inputs, double output, double learningStep);
		double evaluate(double const* inputs) const;
		double evaluate(Record const& record) const;

		std::pair<double, double>& coef(int i) { return coefs_[i]; }
		Config const& config() const { return config_; }
	private:
		Config config_;
		std::vector<std::pair<double, double>> coefs_;
		std::unique_ptr<genann, void (*) (genann*)> pGenann_;
	};

}
