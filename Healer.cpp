#include "Healer.h"

#include <cfloat>
#include <cmath>

Healer::Healer() {}

Healer::~Healer() {}

void Healer::NotifyWallDestroyed(const KamataEngine::Vector3& pos, const KamataEngine::Vector3& rot) {
	destroyedQueue_.push_back(DestroyedWallInfo{pos, rot, nullptr});
}


static float DistanceSq(const KamataEngine::Vector3& a, const KamataEngine::Vector3& b) {
	float dx = a.x - b.x;
	float dy = a.y - b.y;
	float dz = a.z - b.z;
	return dx * dx + dy * dy + dz * dz;
}

void Healer::Update(KamataEngine::Camera* camera, std::list<Wall*>& walls, std::list<HealerActor*>& healers) {
	if (destroyedQueue_.empty()) return;

	
	for (DestroyedWallInfo& info : destroyedQueue_) {
		if (info.assignedHealer != nullptr) continue;

		float bestDist = FLT_MAX;
		HealerActor* best = nullptr;

		for (HealerActor* ha : healers) {
			if (!ha) continue;

			bool alreadyAssigned = false;
			for (const DestroyedWallInfo& other : destroyedQueue_) {
				if (other.assignedHealer == ha) { alreadyAssigned = true; break; }
			}
			if (alreadyAssigned) continue;

			float d = DistanceSq(ha->GetPosition(), info.pos);
			if (d < bestDist) {
				bestDist = d;
				best = ha;
			}
		}

		info.assignedHealer = best;
	}

	
	const float speed = 0.05f;
	for (DestroyedWallInfo& info : destroyedQueue_) {
		if (info.assignedHealer) {
			info.assignedHealer->MoveTowards(info.pos, speed);
			info.assignedHealer->RefreshTransform();
		}
	}

	++healFrameCounter_;
	if (healFrameCounter_ < kHealIntervalFrames) return;

	healFrameCounter_ = 0;

	// 復元先として nullptr のスロットを探す。見つかればそこで壁を復元する。
	for (Wall*& w : walls) {
		if (w == nullptr) {
			Wall* newWall = new Wall();
			const DestroyedWallInfo info = destroyedQueue_.front();
			newWall->Initialize(camera, info.pos);
			newWall->SetRotation(info.rot);
			newWall->Update();
			w = newWall;

		
			if (info.assignedHealer) {
				for (HealerActor*& slot : healers) {
					if (slot == info.assignedHealer) {
						delete slot;
						slot = nullptr;
						break;
					}
				}
			}

			destroyedQueue_.pop_front();
			break;
		}
	}
}
