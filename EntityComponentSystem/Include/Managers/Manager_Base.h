#ifndef _ECS_MANAGER_BASE_H_
#define _ECS_MANAGER_BASE_H_
#include <Entity.h>
#include <Memory_Base.h>
#include <DLLExport.h>
#include <EntityManager_Interface.h>
#include <ResourceHandler\Resource.h>
#include <ResourceHandler\FileSystem_Interface.h>
#include <functional>
#include <fstream>
#include <GUID.h>

namespace ECS
{
	class Manager_Base : public Memory_Base
	{
	public:
		virtual bool IsRegistered(Entity entity)const noexcept = 0;
		virtual void CreateFromResource(Entity entity, ResourceHandler::Resource resource) = 0;
		virtual void CreateFromStream(Entity entity, std::istream* stream) = 0;
		virtual uint64_t GetDataWriter(Entity entity, std::function<bool(std::ostream* file)>& writer)const noexcept = 0;
		virtual void Destroy(Entity entity)noexcept = 0;
		virtual void DestroyEntities(const Entity entities[], uint32_t numEntities)noexcept = 0;
		virtual void DestroyAll()noexcept = 0;

		virtual void ToggleActive(Entity entity, bool active)noexcept = 0;
		virtual void ToggleActive(const Entity entities[], uint32_t numEntites, bool active)noexcept = 0;

		virtual uint32_t GetNumberOfRegisteredEntities()const noexcept = 0;
		virtual void GetRegisteredEntities(Entity entities[], uint32_t numEntities)const noexcept = 0;
	
		virtual void Frame()noexcept = 0;

		virtual Utilities::GUID GetManagerType()const noexcept = 0;
	protected:
		Manager_Base(){}

		virtual void GarbageCollection()noexcept = 0;
	};
}
DECLDIR_ECS bool Manager_Base_IsRegistered_C(ECS::Manager_Base* mb, uint32_t entity);
DECLDIR_ECS void Manager_Base_Destroy_C(ECS::Manager_Base* mb, uint32_t entity);
DECLDIR_ECS void Manager_Base_DestroyEntities_C(ECS::Manager_Base* mb, const uint32_t entities[], uint32_t numEntities);
DECLDIR_ECS void Manager_Base_DestroyAll_C(ECS::Manager_Base* mb);
DECLDIR_ECS void Manager_Base_ToggleActive_C(ECS::Manager_Base* mb, uint32_t entity, bool active);
DECLDIR_ECS void Manager_Base_ToggleActiveMultiple_C(ECS::Manager_Base* mb, const uint32_t entities[], uint32_t numEntities, bool active);
DECLDIR_ECS uint32_t Manager_Base_GetNumberOfRegisteredEntities_C(ECS::Manager_Base* mb);
DECLDIR_ECS void Manager_Base_GetRegisteredEntities_C(ECS::Manager_Base* mb, uint32_t* entities, uint32_t numEntities);
DECLDIR_ECS void Manager_Base_Frame_C(ECS::Manager_Base* mb);
DECLDIR_ECS void Manager_Base_CreateFromResource_C(ECS::Manager_Base* mb, uint32_t entity, const char* guid, const char* type);
DECLDIR_ECS void Manager_Base_CreateFromResourceG_C(ECS::Manager_Base* mb, uint32_t entity, uint32_t guid, uint32_t type);
DECLDIR_ECS Utilities::Error Manager_Base_WriteComponent_C(ECS::Manager_Base* mb, ResourceHandler::FileSystem_Interface* li, uint32_t entity, const char* guid, const char* type);
DECLDIR_ECS uint32_t Manager_Base_GetManagerType_C(ECS::Manager_Base* mb);
#endif