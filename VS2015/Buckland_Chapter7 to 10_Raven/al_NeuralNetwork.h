
#pragma once

#include <memory>


struct genann;

namespace al {

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

		Config const& config() const { return config_; }
	private:
		Config config_;
		std::unique_ptr<genann, void (*) (genann*)> pGenann_;
	};

}
