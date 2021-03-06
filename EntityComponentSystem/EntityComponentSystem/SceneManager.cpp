#include "SceneManager.h"
#include <Profiler.h>
#include <StringReadWrite.h>

ECS::SceneManager::SceneManager(SceneManager_InitializationInfo ii) : initInfo(ii)
{
	_ASSERT_EXPR(initInfo.entityManager, L"SceneManager must have entitymanager");
	_ASSERT_EXPR(initInfo.transformManager, L"SceneManager must have transform manager");

}

ECS::SceneManager::~SceneManager()
{
}

struct OneShotReadBuf : public std::streambuf
{
	OneShotReadBuf(char* s, std::size_t n)
	{
		setg(s, s, s + n);
	}
};
uint32_t ECS::SceneManager::GetNumberOfChildResourcesOfSceneResource(ResourceHandler::Resource resource) const 
{
	StartProfile;

	{
		ResourceHandler::ResourceData<char*> data(resource);
		OneShotReadBuf osrb(data.Get(), data.GetSize());
		std::istream istr(&osrb);
		auto stream = &istr;

		decltype(version) ver = 0;
		stream->read((char*)&ver, sizeof(ver));
		std::string name;
		Utilities::readString(stream, name);

		uint32_t numCS = 0;
		stream->read((char*)&numCS, sizeof(numCS));
		return numCS;
	}
	return 0;
}

void ECS::SceneManager::GetChildResourcesOfSceneResource(ResourceHandler::Resource resource, Utilities::GUID resources[], uint32_t num) const 
{
	StartProfile;
	{
		ResourceHandler::ResourceData<char*> data(resource);
		OneShotReadBuf osrb(data.Get(), data.GetSize());
		std::istream istr(&osrb);
		auto stream = &istr;

		decltype(version) ver = 0;
		stream->read((char*)&ver, sizeof(ver));
		std::string name;
		Utilities::readString(stream, name);

		uint32_t numCS = 0;
		stream->read((char*)&numCS, sizeof(numCS));
		numCS = std::min(numCS, num);
		stream->read((char*)resources, numCS * sizeof(Utilities::GUID));
	}

}

void ECS::SceneManager::RemoveEntityFromScene(Entity scene, Entity entity)noexcept
{
	StartProfile;
	if (auto find = entityToEntry.find(scene); find == entityToEntry.end())
		return;
	else
	{
		auto& map = entries.entityToEntityInScene[find->second];
		if (auto findE = map.find(entity); findE == map.end())
			return;
		else
		{
			auto& ents = entries.entitiesInScene[find->second];
			auto& names = entries.entityNamesInScene[find->second];
			uint32_t last = static_cast<uint32_t>(ents.size() - 1);
			auto lastE = ents[last];

			ents[findE->second] = ents[last];
			names[findE->second] = names[last];

			ents.pop_back();
			names.pop_back();

			map[lastE] = findE->second; 
			map.erase(entity);
		}
	}
}

void ECS::SceneManager::SetNameOfEntityInScene(Entity scene, Entity entity, const std::string & name)noexcept
{
	StartProfile;
	if (auto find = entityToEntry.find(scene); find == entityToEntry.end())
		return;
	else
	{
		auto& map = entries.entityToEntityInScene[find->second];
		if (auto findE = map.find(entity); findE == map.end())
			return;
		else
		{
			entries.entityNamesInScene[find->second][findE->second] = name;
			return;
		}
	}
}

const char * ECS::SceneManager::GetNameOfEntityInScene(Entity scene, Entity entity) const noexcept
{
	StartProfile;
	if (auto find = entityToEntry.find(scene); find == entityToEntry.end())
		return nullptr;
	else
	{
		auto& map = entries.entityToEntityInScene[find->second];
		if (auto findE = map.find(entity); findE == map.end())
			return nullptr;
		else
		{
			return entries.entityNamesInScene[find->second][findE->second].c_str();
		}
	}
}

const char * ECS::SceneManager::GetNameOfScene(Entity scene) const noexcept
{
	StartProfile;
	if (auto find = entityToEntry.find(scene); find == entityToEntry.end())
		return nullptr;
	else
	{
		return entries.name[find->second].c_str();
	}
}

void ECS::SceneManager::SetNameOfScene(Entity scene, const std::string & name)noexcept
{
	StartProfile;
	if (auto find = entityToEntry.find(scene); find == entityToEntry.end())
		return;
	else
	{
		 entries.name[find->second] = name;
	}
}

void ECS::SceneManager::RegisterManager(Manager_Base * manager)noexcept
{
	StartProfile;
	for (auto& m : managers)
		if (m == manager)
			return;
	managers.push_back(manager);
}


void ECS::SceneManager::CreateFromResource(Entity entity, ResourceHandler::Resource resource)
{
	StartProfile;
	if (auto find = entityToEntry.find(entity); find != entityToEntry.end())
		return;

	if (!initInfo.entityManager->IsAlive(entity))
		return;

	{
		 ResourceHandler::ResourceData<char*> data(resource);
		OneShotReadBuf osrb(data.Get(), data.GetSize());
		std::istream istr(&osrb);
		CreateFromStream(entity, &istr);

	}
}


void ReadComponents(ECS::SceneManager* sm, const std::string& name, ECS::Entity entity, std::istream* stream, uint32_t componentCount, const std::vector<ECS::Manager_Base*>& managers)
{
	StartProfile;
	for (uint32_t i = 0; i < componentCount; i++)
	{
		Utilities::GUID type;
		stream->read((char*)&type, sizeof(type));
		if (sm && type == sm->GetManagerType())
			sm->CreateFromResource(entity, ResourceHandler::Resource(name, sm->GetManagerType()));
		else
			for (auto& m : managers)
				if (m->GetManagerType() == type)
					m->CreateFromStream(entity, stream);

	}
};
void ECS::SceneManager::CreateFromStream(Entity entity, std::istream * stream)noexcept
{
	StartProfile;
	if (auto find = entityToEntry.find(entity); find != entityToEntry.end())
		return;

	if (!initInfo.entityManager->IsAlive(entity))
		return;


	decltype(version) ver = 0;
	stream->read((char*)&ver, sizeof(ver));
	std::string name;
	Utilities::readString(stream, name);

	uint32_t index = static_cast<uint32_t>(entries.entity.size());
	entries.entity.push_back(entity);
	entries.entitiesInScene.push_back({});
	entries.entityToEntityInScene.push_back({});
	entries.entityNamesInScene.push_back({});
	entries.name.push_back(name);

	entityToEntry[entity] = index;

	uint32_t numCS = 0;
	stream->read((char*)&numCS, sizeof(numCS));
	std::vector<Utilities::GUID> resources(numCS);
	stream->read((char*)resources.data(), numCS * sizeof(Utilities::GUID));
	//stream->seekg(size_t(stream->tellg()) + numCS * sizeof(Utilities::GUID));

	uint32_t componentCount = 0;
	stream->read((char*)&componentCount, sizeof(componentCount));
	ReadComponents(nullptr, name, entity, stream, componentCount, managers);

	uint32_t numEnts = 0;
	stream->read((char*)&numEnts, sizeof(numEnts));
	entries.entitiesInScene[index].resize(numEnts);
	entries.entityNamesInScene[index].resize(numEnts);
	initInfo.entityManager->CreateMultiple(entries.entitiesInScene[index].data(), uint32_t(entries.entitiesInScene[index].size()));
	for (uint32_t i = 0; i < numEnts; i++)
	{
		entries.entityToEntityInScene[index][entries.entitiesInScene[index][i]] = i;
		Utilities::readString(stream, entries.entityNamesInScene[index][i]);
		stream->read((char*)&componentCount, sizeof(componentCount));
		ReadComponents(this, entries.entityNamesInScene[index][i], entries.entitiesInScene[index][i], stream, componentCount, managers);
		uint8_t bindToParent = 0;
		stream->read((char*)&bindToParent, sizeof(bindToParent));
		if (bindToParent)
			initInfo.transformManager->BindChild(entity, entries.entitiesInScene[index][i], TransformFlags::INHERIT_ALL);
	}


}

uint64_t ECS::SceneManager::GetDataWriter(Entity entity, std::function<bool(std::ostream* file)>& writer)const noexcept
{
	StartProfile;
	if (auto findf = entityToEntry.find(entity); findf == entityToEntry.end())
		return 0;
	else
	{



		uint64_t size = sizeof(version) + sizeof(uint32_t);
		auto& ents = entries.entitiesInScene[findf->second];
		auto& names = entries.entityNamesInScene[findf->second];
		struct Component
		{
			Utilities::GUID type;
			std::function<bool(std::ostream* file)> writer;
		};
		struct WriteInfo
		{
			std::string name;
			uint32_t componentCount;
			uint8_t bindToParent;
			std::vector<Component> eInfo;
		};
		WriteInfo myComponents{ entries.name[findf->second] };
		size += sizeof(uint32_t) + static_cast<uint64_t>(myComponents.name.size());
		std::vector<Utilities::GUID> childScenes;
		uint32_t numChildScenes = 0;
		for (auto& e : ents)
			if (auto findC = entityToEntry.find(e); findC != entityToEntry.end())
				childScenes.push_back(entries.name[findC->second]);
		size += sizeof(uint32_t) + sizeof(Utilities::GUID)*childScenes.size();

		size += sizeof(uint32_t);
		for (auto m : managers)
		{
			if (m == this)
				continue;
			if (m->IsRegistered(entity))
			{
				Component comp{ m->GetManagerType() };

				auto writerSize = m->GetDataWriter(entity, comp.writer);
				if (writerSize == 0)
					continue;
				size += sizeof(comp.type);
				size += writerSize;
				myComponents.eInfo.push_back(std::move(comp));
			}

		}
		myComponents.componentCount = static_cast<uint32_t>(myComponents.eInfo.size());


		std::vector<WriteInfo> writeInfo;
		writeInfo.reserve(ents.size());
		for(size_t i = 0; i < ents.size(); i++)
		{
			auto& e = ents[i];
			WriteInfo info{ names[i], 0 };
			size += sizeof(uint32_t) + static_cast<uint64_t>(info.name.size());
			size += sizeof(uint32_t);
			if (IsRegistered(e))
			{
				Component comp{ GetManagerType() };
				comp.writer = [](std::ostream* out)
				{
					return true;
				};
				

				size += sizeof(comp.type);
				info.eInfo.push_back(std::move(comp));
			}
			else
			{
				for (auto m : managers)
				{

					if (m->IsRegistered(e))
					{
						Component comp{ m->GetManagerType() };
						uint64_t writerSize = 0;

						writerSize = m->GetDataWriter(e, comp.writer);
						if (writerSize == 0)
							continue;

						size += sizeof(comp.type);

						size += writerSize;
						info.eInfo.push_back(std::move(comp));
					}

				}
			}
			
			info.componentCount = static_cast<uint32_t>(info.eInfo.size());

			size += sizeof(info.bindToParent);
			info.bindToParent = uint8_t(initInfo.transformManager->GetParent(e) == entity);
			writeInfo.push_back(std::move(info));

			
		}

		writer = [entity, this,writeInfo, myComponents, size, childScenes](std::ostream* file) {

			if (auto find = entityToEntry.find(entity); find == entityToEntry.end())
				return false;
			else
			{
				uint64_t start = file->tellp();
				auto& ents = entries.entitiesInScene[find->second];

				file->write((char*)&version, sizeof(version));
				Utilities::writeString(file, myComponents.name);
				
				uint32_t numCS = uint32_t(childScenes.size());
				file->write((char*)&numCS, sizeof(numCS));
				file->write((char*)childScenes.data(), sizeof(Utilities::GUID)*numCS);

				file->write((char*)&myComponents.componentCount, sizeof(myComponents.componentCount));
				for (auto& comp : myComponents.eInfo)
				{
					file->write((char*)&comp.type, sizeof(comp.type));
					if (!comp.writer(file))
						return false;
				}

				uint32_t numEnt = static_cast<uint32_t>(ents.size());
				file->write((char*)&numEnt, sizeof(numEnt));
				for (auto& info : writeInfo)
				{
					Utilities::writeString(file, info.name);
					file->write((char*)&info.componentCount, sizeof(info.componentCount));
					for (auto& comp : info.eInfo)
					{
						file->write((char*)&comp.type, sizeof(comp.type));
						if (!comp.writer(file))
							return false;
					}
					file->write((char*)&info.bindToParent, sizeof(info.bindToParent));
				}

				uint64_t asize = uint64_t(file->tellp()) - start;
				if (asize != size)
					return false;

				return true;
			}};



		return size;
	}
}



void ECS::SceneManager::Create(Entity entity, const std::string& name)noexcept
{
	StartProfile;
	if (auto find = entityToEntry.find(entity); find != entityToEntry.end())
		return;

	if (!initInfo.entityManager->IsAlive(entity))
		return;

	uint32_t index = static_cast<uint32_t>(entries.entity.size());
	entries.entity.push_back(entity);
	entries.entitiesInScene.push_back({});
	entries.entityToEntityInScene.push_back({});
	entries.entityNamesInScene.push_back({});
	entries.name.push_back(name);

	entityToEntry[entity] = index;

	initInfo.transformManager->Create(entity);
}
void ECS::SceneManager::AddEntityToScene(Entity scene, Entity entity, const std::string & name)noexcept
{
	StartProfile;
	if (!initInfo.entityManager->IsAlive(entity))
		return;

	if (auto find = entityToEntry.find(scene); find == entityToEntry.end())
		return;
	else
	{
		if (auto findChild = entityToEntry.find(entity); findChild != entityToEntry.end())
			entries.name[findChild->second] = name;
		auto& sceneMap = entries.entityToEntityInScene[find->second];
		if (auto findInScene = sceneMap.find(entity); findInScene != sceneMap.end())
		{
			auto& names = entries.entityNamesInScene[find->second];
			names[findInScene->second] = name;
		}
		else
		{
			auto& ents = entries.entitiesInScene[find->second];
			auto& names = entries.entityNamesInScene[find->second];
			uint32_t index = static_cast<uint32_t>(ents.size());
			ents.push_back(entity);
			names.push_back(name);
			sceneMap[entity] = index;
		}
		initInfo.transformManager->Create(entity);
		initInfo.transformManager->BindChild(scene, entity, TransformFlags::INHERIT_ALL);

	}
}
void ECS::SceneManager::AddEntityToScene(Entity scene, Entity entity)noexcept
{
	StartProfile;
	if (!initInfo.entityManager->IsAlive(entity))
		return;

	if (auto find = entityToEntry.find(scene); find == entityToEntry.end())
		return;
	else
	{
		auto& sceneMap = entries.entityToEntityInScene[find->second];
		if (auto findInScene = sceneMap.find(entity); findInScene != sceneMap.end())
			return;

		auto& ents = entries.entitiesInScene[find->second];
		auto& names = entries.entityNamesInScene[find->second];
		uint32_t index = static_cast<uint32_t>(ents.size());
		ents.push_back(entity);
		if (auto findChild = entityToEntry.find(entity); findChild != entityToEntry.end())
			names.push_back(entries.name[findChild->second]);
		else
			names.push_back({});
		sceneMap[entity] = index;

		initInfo.transformManager->Create(entity);
		initInfo.transformManager->BindChild(scene, entity, TransformFlags::INHERIT_ALL);
	}
}

void ECS::SceneManager::AddEntitiesToScene(Entity scene, const Entity entities[], uint32_t numEntities)noexcept
{
	StartProfile;
	
	if (auto find = entityToEntry.find(scene); find == entityToEntry.end())
		return;
	else
	{
		auto& sceneMap = entries.entityToEntityInScene[find->second];
		for (uint32_t i = 0; i < numEntities; i++)
		{
			if (!initInfo.entityManager->IsAlive(entities[i]))
				continue;

		
			if (auto findInScene = sceneMap.find(entities[i]); findInScene != sceneMap.end())
				continue;

			auto& ents = entries.entitiesInScene[find->second];
			auto& names = entries.entityNamesInScene[find->second];
			uint32_t index = static_cast<uint32_t>(ents.size());
			ents.push_back(entities[i]);
			names.push_back({});
			sceneMap[entities[i]] = index;
			initInfo.transformManager->Create(entities[i]);
			initInfo.transformManager->BindChild(scene, entities[i], TransformFlags::INHERIT_ALL);
		}
	}
}

uint32_t ECS::SceneManager::GetNumberOfEntitiesInScene(Entity scene)const noexcept
{
	StartProfile;

	if (auto find = entityToEntry.find(scene); find == entityToEntry.end())
		return 0;
	else
	{
		return static_cast<uint32_t>(entries.entitiesInScene[find->second].size());
	}
}

void ECS::SceneManager::GetEntitiesInScene(Entity scene, Entity entities[])const noexcept
{

	StartProfile;

	if (auto find = entityToEntry.find(scene); find == entityToEntry.end())
		return;
	else
	{
		memcpy(entities, entries.entitiesInScene[find->second].data(), sizeof(Entity)* entries.entitiesInScene[find->second].size());
	}
}

void ECS::SceneManager::Destroy(Entity entity)noexcept
{
	StartProfile;
	if (auto find = entityToEntry.find(entity); find == entityToEntry.end())
		return;
	else
	{
		uint32_t last = static_cast<uint32_t>(entries.entity.size() - 1);
		Entity lastE = entries.entity[last];

		initInfo.entityManager->DestroyMultiple(entries.entitiesInScene[find->second].data(), uint32_t(entries.entitiesInScene[find->second].size()));

		entries.entity[find->second] = entries.entity[last];
		entries.entitiesInScene[find->second] = entries.entitiesInScene[last];
		entries.entityToEntityInScene[find->second] = entries.entityToEntityInScene[last];
		entries.entityNamesInScene[find->second] = entries.entityNamesInScene[last];
		entries.name[find->second] = entries.name[last];

		entries.entity.pop_back();
		entries.name.pop_back();
		entries.entitiesInScene.pop_back();
		entries.entityNamesInScene.pop_back();
		entries.entityToEntityInScene.pop_back();

		entityToEntry[lastE] = find->second;
		entityToEntry.erase(entity);
	}
}

void ECS::SceneManager::DestroyEntities(const Entity entities[], uint32_t numEntities)noexcept
{
	StartProfile;
	for (uint32_t i = 0; i < numEntities; i++)
	{
		if (auto find = entityToEntry.find(entities[i]); find == entityToEntry.end())
			return;
		else
		{
			uint32_t last = static_cast<uint32_t>(entries.entity.size() - 1);

			initInfo.entityManager->DestroyMultiple(entries.entitiesInScene[find->second].data(), uint32_t(entries.entitiesInScene[find->second].size()));


			entries.entity[find->second] = entries.entity[last];
			entries.entitiesInScene[find->second] = entries.entitiesInScene[last];
			entries.entityToEntityInScene[find->second] = entries.entityToEntityInScene[last];
			entries.entityNamesInScene[find->second] = entries.entityNamesInScene[last];
			entries.name[find->second] = entries.name[last];

			entries.entity.pop_back();
			entries.name.pop_back();
			entries.entitiesInScene.pop_back();
			entries.entityNamesInScene.pop_back();
			entries.entityToEntityInScene.pop_back();

			entityToEntry[entries.entity[find->second]] = find->second;
		}
	}
}

uint32_t ECS::SceneManager::GetNumberOfRegisteredEntities() const noexcept
{
	return static_cast<uint32_t>(entries.entity.size());
}

void ECS::SceneManager::GetRegisteredEntities(Entity entities[], uint32_t numEntities) const noexcept
{
	StartProfile;
	if (numEntities != entries.entity.size())
		return;
	memcpy(entities, entries.entity.data(), sizeof(Entity)*numEntities);
}

void ECS::SceneManager::Frame()noexcept
{
	StartProfile;
}

uint64_t ECS::SceneManager::GetMemoryUsage() const noexcept
{
	return uint64_t();
}

void ECS::SceneManager::ShrinkToFit() noexcept
{
	StartProfile;
	for (auto& eis : entries.entitiesInScene)
		eis.shrink_to_fit();
	for (auto& eis : entries.entityNamesInScene)
		eis.shrink_to_fit();

	entries.entitiesInScene.shrink_to_fit();
	entries.entity.shrink_to_fit();
	entries.entityNamesInScene.shrink_to_fit();
	entries.entityToEntityInScene.shrink_to_fit();
	entries.name.shrink_to_fit();
}



bool ECS::SceneManager::IsRegistered(Entity entity) const noexcept
{
	return entityToEntry.find(entity) != entityToEntry.end();
}

Utilities::GUID ECS::SceneManager::GetManagerType() const noexcept
{
	return "Scene";
}

void ECS::SceneManager::DestroyAll()noexcept
{
	StartProfile;
	for (auto& eis : entries.entitiesInScene)
		initInfo.entityManager->DestroyMultiple(eis.data(), uint32_t(eis.size()));

	entries.entitiesInScene.clear();
	entries.entity.clear();
	entries.entityNamesInScene.clear();
	entries.entityToEntityInScene.clear();
	entries.name.clear();

	entityToEntry.clear();


}

void ECS::SceneManager::ToggleActive(Entity entity, bool active)noexcept
{
}

void ECS::SceneManager::ToggleActive(const Entity entities[], uint32_t numEntites, bool active)noexcept
{
}

void ECS::SceneManager::WriteToFile(std::ofstream & file) const 
{
}

void ECS::SceneManager::CreateFromFile(std::ifstream & file)
{
}

void ECS::SceneManager::GarbageCollection()noexcept
{
}
