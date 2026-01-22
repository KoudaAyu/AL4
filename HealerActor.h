#pragma once

#include"KamataEngine.h"
#include"MathUtl.h"

class HealerActor
{
public:
	HealerActor();
	~HealerActor();
	void Initialize(KamataEngine::Camera* camera,const KamataEngine::Vector3& pos);
	void Update();
	void Draw();

	KamataEngine::Vector3 GetPosition() const;
	void MoveTowards(const KamataEngine::Vector3& target, float speed);
	void RefreshTransform();


	void SetAssigned(bool assigned);
	bool IsAssigned() const;

	private:

		KamataEngine::Model* model_ = nullptr;
	    KamataEngine::WorldTransform worldTransform_;
	    KamataEngine::Camera* camera_ = nullptr;
		bool assigned_ = false;

};
