#include <Managers\TransformManager_Interface.h>
#include "TransformManager.h"
DECLDIR ECS::TransformManager_Interface * TransformManager_CreateTransformManager_C(ECS::TransformManagerInitializationInfo ii)
{
	return new ECS::TransformManager(ii);
}

DECLDIR void TransformManager_Create_C(ECS::TransformManager_Interface * tm, uint32_t entity, ECS::Vector position, ECS::Vector rotation, ECS::Vector scale)
{
	tm->Create(entity, position, rotation, scale);
}

DECLDIR void TransformManager_BindChild_C(ECS::TransformManager_Interface * tm, uint32_t parent, uint32_t child, uint8_t flags)
{
	tm->BindChild(parent, child, ECS::TransformFlags(flags));
}

DECLDIR void TransformManager_SetPosition_C(ECS::TransformManager_Interface * tm, uint32_t entity, ECS::Vector position)
{
	tm->SetPosition(entity, position);
}

DECLDIR ECS::Vector TransformManager_GetScale_C(ECS::TransformManager_Interface * tm, uint32_t entity)
{
	return tm->GetScale(entity);
}

DECLDIR void TransformManager_SetTransform_C(ECS::TransformManager_Interface * tm, uint32_t entity, ECS::Matrix transform)
{
	tm->SetTransform(entity, transform);
}

DECLDIR ECS::Matrix TransformManager_GetTransform_C(ECS::TransformManager_Interface * tm, uint32_t entity)
{
	return tm->GetTransform(entity);
}

DECLDIR void TransformManager_RegisterTransformUser(ECS::TransformManager_Interface * tm, ECS::Manager_TransformUser * tu)
{
	tm->RegisterTransformUser(tu);
}

DECLDIR void TransformManager_UnregisterTransformUser(ECS::TransformManager_Interface * tm, ECS::Manager_TransformUser * tu)
{
	tm->UnregisterTransformUser(tu);
}

DECLDIR ECS::Vector TransformManager_GetRotation_C(ECS::TransformManager_Interface * tm, uint32_t entity)
{
	return tm->GetRotation(entity);
}

DECLDIR void TransformManager_SetScale_C(ECS::TransformManager_Interface * tm, uint32_t entity, ECS::Vector scale)
{
	tm->SetScale(entity, scale);
}


DECLDIR ECS::Vector TransformManager_GetPosition_C(ECS::TransformManager_Interface * tm, uint32_t entity)
{
	return tm->GetPosition(entity);
}

DECLDIR void TransformManager_SetRotation_C(ECS::TransformManager_Interface * tm, uint32_t entity, ECS::Vector rotation)
{
	tm->SetRotation(entity, rotation);
}
