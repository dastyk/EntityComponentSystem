#ifndef _ECS_TRANSFORM_MANAGER_H_
#define _ECS_TRANSFORM_MANAGER_H_
#include <Managers\TransformManager_Interface.h>
#include <Sofa.h>
#include <DirectXMath.h>
#include <random>

using namespace DirectX;
namespace ECS
{
	class TransformManager : public TransformManager_Interface
	{
	public:
		TransformManager(ECS::TransformManagerInitializationInfo initInfo);
		~TransformManager();
		virtual void Create(Entity entity, const Vector& position = Vector(), const Vector& rotaiton = Vector(), const Vector& scale = Vector())noexcept override;
		
		virtual void SetPosition(Entity entity, const Vector& position)noexcept override;
		virtual Vector GetPosition(Entity entity)const noexcept override;

		virtual void SetRotation(Entity entity, const Vector& rotation)noexcept override;
		virtual Vector GetRotation(Entity entity)const noexcept  override;

		virtual void SetScale(Entity entity, const Vector& scale)noexcept override;
		virtual Vector GetScale(Entity entity)const noexcept override;


		virtual void Destroy(Entity entity)noexcept override;
		virtual void DestroyEntities(const Entity entities[], uint32_t numEntities)noexcept override;
		virtual uint32_t GetNumberOfRegisteredEntities()const noexcept override;
		virtual void GetRegisteredEntities(Entity entities[], uint32_t numEntities)const noexcept override;
		virtual void Frame()noexcept override;


		virtual uint64_t GetMemoryUsage()const noexcept override;
		virtual void ShrinkToFit()noexcept override;
		virtual void WriteToFile(std::ofstream& file)const override;
		virtual void CreateFromFile(std::ifstream& file) override;
	private:
		virtual void GarbageCollection()noexcept override;
		void UpdateDirtyEntities()noexcept;


		TransformManagerInitializationInfo initInfo;
		uint32_t version = 0;
		std::default_random_engine generator;

		Sofa<
			Entity, Entity::Hasher,
			XMFLOAT3,
			XMFLOAT3,
			XMFLOAT3,
			XMFLOAT4X4,
			bool>
			entries;
		struct EntryNames
		{
			enum EntryNamesEnum
			{
				Entity,
				Position,
				Rotation,
				Scale,
				Transform,
				Dirty
			};
		};
	};
}
#endif
