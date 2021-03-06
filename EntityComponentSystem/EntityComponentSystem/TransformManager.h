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
		TransformManager(ECS::TransformManager_InitializationInfo initInfo);
		~TransformManager();
		virtual void Create(Entity entity,
			const Vector& position = Vector(), 
			const Vector& rotaiton = Vector(), 
			const Vector& scale = { 1.0f, 1.0f, 1.0f })noexcept override;
		
		virtual void BindChild(Entity parent, Entity child, TransformFlags flags)noexcept override;
		virtual void UnbindParent(Entity entity, TransformFlags flags)noexcept override;
		virtual void UnbindAllChildren(Entity entity, TransformFlags flags)noexcept override;
		virtual Entity GetParent(Entity entity)const noexcept override;
		virtual uint32_t GetNumberOfChildren(Entity entity)const noexcept override;
		virtual void GetChildren(Entity parent, Entity children[])const noexcept override;

		virtual void SetPosition(Entity entity, const Vector& position)noexcept override;
		virtual Vector GetPosition(Entity entity)const noexcept override;

		virtual void SetRotation(Entity entity, const Vector& rotation)noexcept override;
		virtual Vector GetRotation(Entity entity)const noexcept  override;

		virtual void SetScale(Entity entity, const Vector& scale)noexcept override;
		virtual Vector GetScale(Entity entity)const noexcept override;

		virtual void SetTransform(Entity entity, const Matrix& transform)noexcept override;
		virtual Matrix GetTransform(Entity entity)noexcept override;

		virtual void RegisterTransformUser(Manager_TransformUser* tUser)noexcept override;
		virtual void UnregisterTransformUser(Manager_TransformUser* tUser)noexcept override;

		virtual void CreateFromResource(Entity entity, ResourceHandler::Resource resource) override;
		virtual void CreateFromStream(Entity entity, std::istream* stream)noexcept override;
		virtual uint64_t GetDataWriter(Entity entity, std::function<bool(std::ostream* file)>& writer)const noexcept override;

		virtual bool IsRegistered(Entity entity)const noexcept override;
		virtual void Destroy(Entity entity)noexcept override;
		virtual void DestroyEntities(const Entity entities[], uint32_t numEntities)noexcept override;
		virtual void DestroyAll()noexcept override;

		virtual void ToggleActive(Entity entity, bool active)noexcept override;
		virtual void ToggleActive(const Entity entities[], uint32_t numEntites, bool active)noexcept override;

		virtual uint32_t GetNumberOfRegisteredEntities()const noexcept override;
		virtual void GetRegisteredEntities(Entity entities[], uint32_t numEntities)const noexcept override;
		virtual void Frame()noexcept override;
		virtual Utilities::GUID GetManagerType()const noexcept override;

		virtual uint64_t GetMemoryUsage()const noexcept override;
		virtual void ShrinkToFit()noexcept override;
		virtual void WriteToFile(std::ofstream& file)const override;
		virtual void CreateFromFile(std::ifstream& file) override;
	private:
		virtual void GarbageCollection()noexcept override;
		void UpdateDirtyEntities()noexcept;


		TransformManager_InitializationInfo initInfo;
		uint32_t version = 0;
		std::default_random_engine generator;
		std::vector<Manager_TransformUser*> transformUsers;
		Utilities::SofA::Array::SofA<
			Entity, Entity::Hasher,
			XMFLOAT3,
			XMFLOAT4,
			XMFLOAT3,
			XMFLOAT4X4,
			bool,
			TransformFlags,
			uint32_t,
			uint32_t,
			uint32_t>
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
				Dirty,
				Flags,
				Child,
				Sibling,
				Parent
			};
		};
	};
}
#endif
