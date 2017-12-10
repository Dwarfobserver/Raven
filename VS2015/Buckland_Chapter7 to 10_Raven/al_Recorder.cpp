
#include "al_Recorder.h"
#include "Raven_Bot.h"
#include "al_FileParser.h"
#include "al_Resources.h"
#include "Raven_Game.h"
#include "Raven_Map.h"
#include "Raven_WeaponSystem.h"
#include "armory/Raven_Weapon.h"
#include "2d/WallIntersectionTests.h"
#include "goals/Raven_Feature.h"

using namespace al;
using namespace std::literals;
using namespace std::chrono;

char const* Recorder::FILE_NAME {"in-game_records"};

Recorder::Recorder(Raven_Bot& bot) :
		bot_{bot}
{
	try {
		pFile_ = resources().open(FILE_NAME);
	}
	catch (...) {
		resources().create(FILE_NAME,
			Attributes::TargetDistance |
			Attributes::LineOfSight    |
			Attributes::OwnerLife      |
			Attributes::AmmoCount      |
			Attributes::Decision);
		pFile_ = resources().open(FILE_NAME);
	}
	pFile_->saveAtDestruction(true);
	lastUpdate_ = high_resolution_clock::now();
}

void Recorder::update() {
	const auto currentTime = high_resolution_clock::now();
	if (currentTime - lastUpdate_ < 1s) {
		return;
	}
	if (!bot_.GetWeaponSys()->GetCurrentWeapon()->isReadyForNextShot()) {
		return;
	}

	lastUpdate_ = currentTime;
	record(false);
}

void Recorder::record(bool hasFired) {
	if (!bot_.GetTargetBot()) return;

	const auto healthRatio = Raven_Feature::Health(&bot_);
	const auto weaponsRatio = Raven_Feature::TotalWeaponStrength(&bot_);

	auto pWorld = bot_.GetWorld();
	const auto pTarget = bot_.GetTargetBot();

	const double distance = Vec2DDistanceSq(bot_.Pos(), pTarget->Pos());

	const bool sight = doWallsObstructLineSegment(
		bot_.Pos(),
		pTarget->Pos(),
		pWorld->GetMap()->GetWalls());
	
	Record r;
	r[Attributes::TargetDistance] = distance;
	r[Attributes::LineOfSight] = !sight ? 1.0 : 0.0;
	r[Attributes::OwnerLife] = healthRatio;
	r[Attributes::AmmoCount] = weaponsRatio;
	r[Attributes::Decision] = hasFired;

	pFile_->addRecord(r);


}
