
#pragma once

#include <vector>
#include <memory>


struct genann;

namespace al {

	class NeuralNetwork {
	public:
		struct Config {
			int inputsCount;
			int layersCount;
			int layerSize;
			double learningStep;
		};
		explicit NeuralNetwork(Config const& config);
		
		void train(double const* inputs, double output);
		double evaluate(double const* inputs) const;

		Config const& config() const { return config_; }
	private:
		Config config_;
		std::unique_ptr<genann, void (*) (genann*)> pGenann_;
	};

}
