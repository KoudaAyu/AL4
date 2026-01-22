#include "HealerActor.h"
#include <cmath>

using namespace KamataEngine;

HealerActor::HealerActor() {}

HealerActor::~HealerActor() {}

void HealerActor::Initialize(Camera* camera, const KamataEngine::Vector3& pos) {

	camera_ = camera;

	worldTransform_.Initialize();
	worldTransform_.translation_ = pos;
	worldTransform_.scale_ = {0.3f, 0.3f, 0.3f};

	model_ = Model::CreateFromOBJ("TD_Healer",true);

}

void HealerActor::Update() {

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();

}

void HealerActor::Draw() {
	model_->Draw(worldTransform_, *camera_); 
}

KamataEngine::Vector3 HealerActor::GetPosition() const {
	return worldTransform_.translation_;
}

void HealerActor::MoveTowards(const KamataEngine::Vector3& target, float speed) {
	KamataEngine::Vector3& pos = worldTransform_.translation_;
	KamataEngine::Vector3 dir{target.x - pos.x, target.y - pos.y, target.z - pos.z};
	float len = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
	if (len < 1e-4f) return;
	dir.x /= len; dir.y /= len; dir.z /= len;
	pos.x += dir.x * speed;
	pos.y += dir.y * speed;
	pos.z += dir.z * speed;
}

void HealerActor::RefreshTransform() {
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();
}

void HealerActor::SetAssigned(bool assigned) { assigned_ = assigned; }

bool HealerActor::IsAssigned() const { return assigned_; }
