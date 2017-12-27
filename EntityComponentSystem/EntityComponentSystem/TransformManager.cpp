#include "TransformManager.h"
#include <algorithm>
#include <Profiler.h>
#undef min
#undef max
namespace ECS
{
	XMFLOAT3 ToXMFLOAT3(const Vector& v)
	{
		return { v.x,  v.y, v.z };
	}
	Vector ToVector(const XMFLOAT3& f)
	{
		return { f.x, f.y, f.z };
	}
	
	TransformManager::TransformManager(ECS::TransformManagerInitializationInfo initInfo)
		: initInfo(initInfo)
	{
		_ASSERT_EXPR(initInfo.entityManager, "Transform manager must have an entity manager");
		initInfo.entityManager->RegisterManagerForDestroyNow(this);
	}
	TransformManager::~TransformManager()
	{
		initInfo.entityManager->UnregisterManagerForDestroyNow(this);
	}



	void TransformManager::Create(Entity entity, const Vector & position, const Vector & rotaiton, const Vector & scale)noexcept
	{
		StartProfile;
		if (auto find = entries.find(entity); find.has_value())
			return;

		if (!initInfo.entityManager->IsAlive(entity))
			return;

		auto index = entries.add(entity);
		entries.get<EntryNames::Position>(index) = ToXMFLOAT3(position);
		entries.get<EntryNames::Rotation>(index) = ToXMFLOAT3(rotaiton);
		entries.get<EntryNames::Scale>(index) = ToXMFLOAT3(scale);
		XMStoreFloat4x4(&entries.get<EntryNames::Transform>(index), XMMatrixIdentity());
		entries.get<EntryNames::Dirty>(index) = true;
	}
	void TransformManager::BindChild(Entity parent, Entity child, TransformFlags flags)noexcept
	{
	}
	void TransformManager::SetPosition(Entity entity, const Vector & position)noexcept
	{
		StartProfile;
		if (auto find = entries.find(entity); !find.has_value())
			return;
		else
		{
			entries.get<EntryNames::Position>(find->second) = ToXMFLOAT3(position);
			entries.get<EntryNames::Dirty>(find->second) = true;
		}
	}
	Vector TransformManager::GetPosition(Entity entity)const noexcept
	{
		StartProfile;
		if (auto find = entries.find(entity); !find.has_value())
			return Vector();
		else
			return ToVector(entries.getConst<EntryNames::Position>(find->second));
	}	
	void TransformManager::SetRotation(Entity entity, const Vector & rotation)noexcept
	{
		StartProfile;
		if (auto find = entries.find(entity); !find.has_value())
			return;
		else
		{
			entries.get<EntryNames::Rotation>(find->second) = ToXMFLOAT3(rotation);
			entries.get<EntryNames::Dirty>(find->second) = true;
		}
	}
	Vector TransformManager::GetRotation(Entity entity) const noexcept
	{ 
		StartProfile;
		if (auto find = entries.find(entity); !find.has_value())
			return Vector();
		else
			return ToVector(entries.getConst<EntryNames::Rotation>(find->second));
	}
	void TransformManager::SetScale(Entity entity, const Vector & scale)noexcept
	{
		StartProfile;
		if (auto find = entries.find(entity); !find.has_value())
			return;
		else
		{
			entries.get<EntryNames::Scale>(find->second) = ToXMFLOAT3(scale);
			entries.get<EntryNames::Dirty>(find->second) = true;
		}
	}
	Vector TransformManager::GetScale(Entity entity) const noexcept
	{
		StartProfile;
		if (auto find = entries.find(entity); !find.has_value())
			return Vector();
		else
			return ToVector(entries.getConst<EntryNames::Scale>(find->second));
	}
	void TransformManager::SetTransform(Entity entity, const Matrix & transform)noexcept
	{
		StartProfile;
		if (auto find = entries.find(entity); !find.has_value())
			return;
		else
		{
			XMMATRIX trans = XMLoadFloat4x4((XMFLOAT4X4*)&transform);
			XMVECTOR scale;
			XMVECTOR pos;
			XMVECTOR rot;
			XMMatrixDecompose(&scale, &rot, &pos, trans);
			XMStoreFloat3(&entries.get<EntryNames::Scale>(find->second), scale);
			XMStoreFloat3(&entries.get<EntryNames::Position>(find->second), pos);
			XMStoreFloat3(&entries.get<EntryNames::Rotation>(find->second), rot);
			entries.get<EntryNames::Dirty>(find->second) = true;
		}
		
	}
	Matrix TransformManager::GetTransform(Entity entity)noexcept
	{
		StartProfile;
		if (auto find = entries.find(entity); !find.has_value())
			return Matrix();
		else
		{
			Matrix mat;
			XMStoreFloat4x4((XMFLOAT4X4*)&mat, XMLoadFloat4x4(&entries.getConst<EntryNames::Transform>(find->second)));
			return mat;
		}
		
	}

	void TransformManager::RegisterTransformUser(Manager_TransformUser * tUser)noexcept
	{
		StartProfile;
		for (auto tu : transformUsers)
			if (tu == tUser)
				return;

		transformUsers.push_back(tUser);
	}

	void TransformManager::UnregisterTransformUser(Manager_TransformUser * tUser)noexcept
	{
		StartProfile;
		for(auto& tu : transformUsers)
			if (tu == tUser)
			{
				tu = transformUsers[transformUsers.size() - 1];
				transformUsers.pop_back();
				break;
			}
	}



	void TransformManager::Destroy(Entity entity)noexcept
	{
		StartProfile;
		if (auto find = entries.find(entity); !find.has_value())
			return;
		else
			entries.destroy(find->second);
	}
	void TransformManager::DestroyEntities(const Entity entities[], uint32_t numEntities)noexcept
	{
		StartProfile;
		for (uint32_t i = 0; i < numEntities; i++)
		{
			if (auto find = entries.find(entities[i]); !find.has_value())
				return;
			else
				entries.destroy(find->second);
		}
	}
	uint64_t TransformManager::GetMemoryUsage() const noexcept
	{
		return entries.GetMemoryUsage() + sizeof(*this);
	}
	void TransformManager::ShrinkToFit()noexcept
	{
		StartProfile;
		entries.shrink_to_fit();
	}

	uint32_t TransformManager::GetNumberOfRegisteredEntities() const noexcept
	{
		return static_cast<uint32_t>(entries.size());
	}
	void TransformManager::GetRegisteredEntities(Entity entities[], uint32_t numEntities) const noexcept
	{
		if (numEntities != entries.size())
			return;
		memcpy(entities, entries.getConst<EntryNames::Entity>(), sizeof(Entity)*numEntities);
	}

	void TransformManager::Frame()noexcept
	{
		StartProfile;
		GarbageCollection();
		UpdateDirtyEntities();
	}

	void TransformManager::WriteToFile(std::ofstream & file) const
	{
		file.write((char*)&version, sizeof(version));
		uint32_t used = static_cast<uint32_t>(entries.size());
		file.write((char*)&used, sizeof(used));
		file.write((char*)entries.getConst<EntryNames::Entity>(), sizeof(Entity)*used);
		file.write((char*)entries.getConst<EntryNames::Position>(), sizeof(XMFLOAT3)*used);
		file.write((char*)entries.getConst<EntryNames::Rotation>(), sizeof(XMFLOAT3)*used);
		file.write((char*)entries.getConst<EntryNames::Scale>(), sizeof(XMFLOAT3)*used);
	}
	void TransformManager::CreateFromFile(std::ifstream & file)
	{
		uint32_t ver = 0;
		file.read((char*)&ver, sizeof(ver));
		uint32_t used = 0;
		file.read((char*)&used, sizeof(used));
		entries.Allocate(used);
		file.read((char*)entries.get<EntryNames::Entity>(), sizeof(Entity)*used);
		file.read((char*)entries.getConst<EntryNames::Position>(), sizeof(XMFLOAT3)*used);
		file.read((char*)entries.getConst<EntryNames::Rotation>(), sizeof(XMFLOAT3)*used);
		file.read((char*)entries.getConst<EntryNames::Scale>(), sizeof(XMFLOAT3)*used);
		entries.Reinit(used);
		for (uint32_t i = 0; i < used; i++)
		{
			entries.get<EntryNames::Dirty>()[i] = true;
			XMStoreFloat4x4(&entries.get<EntryNames::Transform>()[i], XMMatrixIdentity());
		}
	}
	void TransformManager::UpdateDirtyEntities()noexcept
	{
		auto transforms = (Matrix*)entries.getConst<EntryNames::Transform>();
		for (size_t i = 0; i < entries.size(); i++)
		{
			if (entries.get<EntryNames::Dirty>(i))
			{
				const auto& translation = XMMatrixTranslationFromVector(XMLoadFloat3(&entries.get<EntryNames::Position>(i)));
				const auto& rotation = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&entries.get<EntryNames::Rotation>(i)));
				const auto& scale = XMMatrixScalingFromVector(XMLoadFloat3(&entries.get<EntryNames::Scale>(i)));
				XMStoreFloat4x4((XMFLOAT4X4*)&transforms[i], scale*rotation*translation);
			}
		}
		auto entities = entries.get<EntryNames::Entity>();
		for(auto tu : transformUsers)
			tu->UpdateEntityTransforms(transforms, entities, static_cast<uint32_t>(entries.size()));
	}
	void TransformManager::GarbageCollection()noexcept
	{
		StartProfile;
		uint32_t alive_in_row = 0;
		const uint32_t quitWhenReached = std::max(uint32_t(entries.size() * 0.1f), 40U);
		while (entries.size() > 0 && alive_in_row < quitWhenReached)
		{
			std::uniform_int_distribution<size_t> distribution(0U, entries.size() - 1U);
			size_t i = distribution(generator);
			if (initInfo.entityManager->IsAlive(entries.get<EntryNames::Entity>(i)))
			{
				alive_in_row++;
				continue;
			}
			alive_in_row = 0;
			entries.destroy(i);
		}
	}
}