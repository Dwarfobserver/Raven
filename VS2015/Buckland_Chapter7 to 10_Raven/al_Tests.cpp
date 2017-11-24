
#include "al_Tests.h"
#include "AttackLearning.h"
#include "Debug/DebugConsole.h"


using namespace al;

static char const* gError = "No error";
#define REQUIRE(b, msg) if (!(b)) { gError = msg; return false; }

bool Tests::run() {
	using namespace std::string_literals;
	static std::vector<Case> cases {
		{"Record", [] {
			Record r;
			REQUIRE(r[Attributes::OwnerLife] == Record::invalidValue, "Initialization failed");

			float firstVal = 23.f;
			float lastVal = 2.f;
			r[Attributes::FirstFlag] = firstVal;
			r[Attributes::LastFlag] = lastVal;
			REQUIRE(r[Attributes::LastFlag] == lastVal, "Setter failed");

			auto str = r.serialize();
			auto r2 = Record(r.attributes(), str.c_str());
			REQUIRE(r2 == r, "Serialisation failed");

			auto r3 = Record(r.attributes(), { firstVal, lastVal });
			REQUIRE(r3 == r, "Vector copy failed");
			
			return true;
		} },
		{ "DataFile", [] {
			auto fileName = "__testDataFile";
			auto attr = Attributes::FirstFlag | Attributes::LastFlag;
			auto r1 = Record{ attr,{ 1.f, 2.f } };
			auto r2 = Record{ attr,{ 3.f, 4.f } };
			{
				auto dataFile = resources().create(fileName, attr);
				REQUIRE(dataFile.attributes() == attr, "Attributes failed");

				dataFile.addRecord(r1);
				dataFile.addRecord(r2);
				REQUIRE(dataFile.records().size() == 2, "Data count failed");

				dataFile.saveAtDestruction(true);
				// Auto-save
			}
			{
				auto dataFile2 = resources().open(fileName);
				REQUIRE(dataFile2.attributes() == attr, "Attributes serialization failed");
				REQUIRE(dataFile2.records().size() == 2, "Data count serialization failed");

				REQUIRE(dataFile2.records()[0] == r1, "Data serialization failed");
				REQUIRE(dataFile2.records()[1] == r2, "Data serialization failed");

				dataFile2.saveAtDestruction(false);
			}
			resources().remove(fileName);

			return true;
		} },
		{ "NeuralNetwork", [] {
			auto fileName = "__testXOR";
			// Attributes : X1, X2 and the output
			auto attr = Attributes::LastFlag | Attributes::FirstFlag | (Attributes::FirstFlag << 1);
			auto dataFile = resources().open(fileName);
			auto trainer = Trainer{ attr, {&dataFile.records()} };

			auto config = NeuralNetwork::Config {};
			config.inputsCount = 2;
			config.learningStep = 0.1;
			config.layerSize = 2;
			config.layersCount = 1;
			auto nn = NeuralNetwork{ config };

			trainer.train(nn);

			auto it = trainer.trainingData.begin();
			while (it != trainer.trainingData.end()) {
				double output = nn.evaluate(it);
				double res = it[trainer.dataSize() - 1];
				REQUIRE(std::abs(output - res) < 0.5, "Training failed");
				it += trainer.dataSize();
			}

			return true;
		} }
	};

	// Run tests & get errors

	std::vector<std::string> errors;
	for (auto& c : cases) {
		std::string error;
		bool success = false;
		try {
			success = c.test();
		}
		catch (std::exception& e) {
			error = e.what();
		}
		if (!success) {
			if (error.empty()) error = gError;
			errors.emplace_back();
			errors.back() += c.name + " - " + error;
		}
	}

	// Print success or errors

	char const * delimiter = "\n----------------------------------\n";
	debug_con << delimiter;

	if (errors.empty()) {
		debug_con << "\nAttackLearning tests succeed !";
		debug_con << delimiter;
		return true;
	}

	debug_con << '\n' << errors.size() << " tests failed :";
	for (auto const& error : errors) {
		debug_con << "\n   " << error;
	}
	debug_con << delimiter;
	return false;
}

Tests::Case::Case(std::string&& name, std::function<bool()>&& f) :
	name{move(name)}, test{move(f)}
{}
