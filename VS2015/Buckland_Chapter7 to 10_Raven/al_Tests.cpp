﻿
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
				REQUIRE(dataFile->attributes() == attr, "Attributes failed");

				dataFile->addRecord(r1);
				dataFile->addRecord(r2);
				REQUIRE(dataFile->records().size() == 2, "Data count failed");

				dataFile->saveAtDestruction(true);
				// Auto-save
			}
			{
				auto dataFile2 = resources().open(fileName);
				REQUIRE(dataFile2->attributes() == attr, "Attributes serialization failed");
				REQUIRE(dataFile2->records().size() == 2, "Data count serialization failed");

				REQUIRE(dataFile2->records()[0] == r1, "Data serialization failed");
				REQUIRE(dataFile2->records()[1] == r2, "Data serialization failed");

				dataFile2->saveAtDestruction(false);
			}
			resources().remove(fileName);

			return true;
		} },
		{ "NeuralNetwork", [] {
			auto fileName = "__testXOR";
			// Attributes : X1, X2 and the output
			auto attr = Attributes::LastFlag | Attributes::FirstFlag | (Attributes::FirstFlag << 1);
			auto dataFile = resources().open(fileName);
			auto records = PackedRecords{ attr, {&dataFile->records()} };

			auto nnConfig = NeuralNetwork::Config {};
			nnConfig.inputsCount = 2;
			nnConfig.layerSize = 5;
			nnConfig.layersCount = 2;
			auto nn = NeuralNetwork{ nnConfig };

			auto trainConfig = TrainConfig{};
			trainConfig.learningStep = 2.0;
			trainConfig.errorAccepted = 0.03;
			trainConfig.passesBetweenChecks = 1000;
			trainConfig.trainingSampleRatio = 0.67;

			double avgError = train(nn, records, trainConfig);
			REQUIRE(avgError < trainConfig.errorAccepted, "Training failed");

			int errorCount = 0;
			int testSampleCount = static_cast<int>((1 - trainConfig.trainingSampleRatio) * records.dataCount());
			auto it = records.begin() + records.dataSize() * (records.dataCount() - testSampleCount);
			while (it != records.end()) {
				Record r;
				r[Attributes::FirstFlag] = it[0];
				r[Attributes::FirstFlag << 1] = it[1];

				double output = nn.evaluate(r);
				double res = it[records.dataSize() - 1];
				if (abs(output - res) >= 0.5) {
					++errorCount;
				}
				it += records.dataSize();
			}
			REQUIRE(errorCount < trainConfig.errorAccepted * testSampleCount, "Sample tests failed");

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
