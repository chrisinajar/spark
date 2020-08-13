#include "spark_precompiled.h"

//#include <../RenderDll/Common/Shaders/CShader.h>

#include "NavigationAreaComponent.h"

#include <AzFramework/Entity/EntityContextBus.h>
#include <AzFramework/Entity/EntityContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>

#include "Busses/GameManagerBus.h"
#include "Busses/NavigationManagerBus.h"
#include "Busses/CameraBus.h"
#include "Busses/GamePlayerBus.h"
#include "Busses/NetSyncBusses.h"

#include <AzCore/Component/TransformBus.h>
#include <LmbrCentral/Shape/ShapeComponentBus.h>
#include <LmbrCentral/Scripting/TriggerAreaComponentBus.h>
#include <LmbrCentral/Rendering/MeshComponentBus.h>
#include <MathConversion.h>



#include <ISystem.h>
#include <CryAction.h>
#include <Cry_Camera.h>
#include <IRenderer.h>
#include <I3DEngine.h>

#include <IShader.h>
#include <ISharedParamsManager.h>

#include <IMaterial.h>

#include <LyShine/IDraw2d.h>
#include <sparkCVars.h>


#include <AzCore/Jobs/Job.h>
#include <AzCore/Math/MathUtils.h>
#include "Busses/VariableBus.h"

#include "Utils/Log.h"

#include <numeric>

#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>

using namespace spark;
using namespace pathfinder;

using AZStd::vector;


class NavigationAreaNotificationBusHandler
	: public NavigationAreaNotificationBus::Handler
	, public AZ::BehaviorEBusHandler
{
public:
	AZ_EBUS_BEHAVIOR_BINDER(NavigationAreaNotificationBusHandler, "{CC119EBB-1417-4ED1-AFB3-69C2F48C6AC9}", AZ::SystemAllocator, OnMainNavigationAreaChanged);

	void OnMainNavigationAreaChanged(AZ::EntityId id) 
	{
		Call(FN_OnMainNavigationAreaChanged, id);
	}
};

void NavigationAreaComponent::Reflect(AZ::ReflectContext* context)
{
	if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
	{
		serialize->Class<NavigationAreaComponent, AZ::Component>()
			->Version(0)
			->Field("resolution", &NavigationAreaComponent::m_resolution)
			->Field("debug", &NavigationAreaComponent::m_debug)
			->Field("maxSlope", &NavigationAreaComponent::m_maxSlope)
			;

		if (AZ::EditContext* ec = serialize->GetEditContext())
		{
			ec->Class<NavigationAreaComponent>("NavigationAreaComponent", "")
				->ClassElement(AZ::Edit::ClassElements::EditorData, "")
				->Attribute(AZ::Edit::Attributes::Category, "spark")
				->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game"))
				->DataElement(AZ::Edit::UIHandlers::Default, &NavigationAreaComponent::m_resolution, "resolution", "")
				->DataElement(AZ::Edit::UIHandlers::Default, &NavigationAreaComponent::m_debug, "debug renderer", "")
				->DataElement(AZ::Edit::UIHandlers::Default, &NavigationAreaComponent::m_maxSlope, "max slope", "")
				;
		}
	}

	if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
	{
		behaviorContext->EBus<NavigationAreaRequestBus>("NavigationAreaRequestBus")
			->Event("UpdateHeightfield", &NavigationAreaRequestBus::Events::UpdateHeightfield)
			->Event("SetTilesResolution", &NavigationAreaRequestBus::Events::SetTilesResolution)
			->Event("GetTilesResolution", &NavigationAreaRequestBus::Events::GetTilesResolution)
			->Event("IsPointInside", &NavigationAreaRequestBus::Events::IsPointInside)
			->Event("IsUnitInside", &NavigationAreaRequestBus::Events::IsUnitInside)
			->Event("GetAabbMin", &NavigationAreaRequestBus::Events::GetAabbMin)
			->Event("GetAabbMax", &NavigationAreaRequestBus::Events::GetAabbMax)
			;

		behaviorContext->EBus<NavigationAreaNotificationBus>("NavigationAreaNotificationBus")
			->Handler<NavigationAreaNotificationBusHandler>();
	}
}

void NavigationAreaComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
{
	provided.push_back(AZ_CRC("SparkNavigationAreaService"));
}
void NavigationAreaComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
{
	incompatible.push_back(AZ_CRC("SparkNavigationAreaService"));
}
void NavigationAreaComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
{
	required.push_back(AZ_CRC("ShapeService", 0xe86aa5fe));	// need for sure a shape component
	required.push_back(AZ_CRC("ProximityTriggerService", 0x561f262c));//need a trigger area?
}
void NavigationAreaComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
{
	dependent.push_back(AZ_CRC("NavigationManagerService"));
}

void NavigationAreaComponent::Init()
{
	m_resolution.SetZ(1);

	coordinates size = { static_cast<coordinates::CoordinateType>(m_resolution.GetX()), static_cast<coordinates::CoordinateType>(m_resolution.GetY()) };
	m_heightField.SetSize(size);
	m_unitField.SetSize(size);
	m_visibilityField.SetSize(size);
	m_movementBlockingField.SetSize(size);
	m_visionBlockingField.SetSize(size);
}

void NavigationAreaComponent::Activate()
{
	NavigationAreaRequestBus::Handler::BusConnect(GetEntityId());
	//AZ::TickBus::Handler::BusConnect();

	//MapRequestBus::Handler::BusConnect();
	NavigationEntitiesNotificationBus::Handler::BusConnect();

	UpdateHeightfield();

	/*auto tex=GetISystem()->GetIRenderer()->CreateDynTexture2(64,64, eTF_R32F,"",ETexPool());
	tex->GetImageData32();*/




	
	BindVisibilityTexture();
	//AZ_Assert(renderer->EF_GetTextureByName("FogOfWarTex") == m_visibilityMap, "fog of war texture not found");
	

	EBUS_EVENT(NavigationAreaNotificationBus, OnNavigationAreaActivated, GetEntity());
}

void NavigationAreaComponent::Deactivate()
{
	UnbindVisibilityTexture();

	NavigationEntitiesNotificationBus::Handler::BusDisconnect();
	//MapRequestBus::Handler::BusDisconnect();

	//AZ::TickBus::Handler::BusDisconnect();
	NavigationAreaRequestBus::Handler::BusDisconnect();
}

pathfinder::coordinates NavigationAreaComponent::ConvertToGridCoordinates(AZ::Vector3 worldCoordinates)
{
	worldCoordinates = m_resolution * (worldCoordinates - m_aabb.GetMin()) / (m_aabb.GetMax() - m_aabb.GetMin());
	return { worldCoordinates.GetX(), worldCoordinates.GetY() };
}
AZ::Vector3 NavigationAreaComponent::ConvertToWorldCoordinates(pathfinder::coordinates gridCoordinates, AZ::Vector3 cellOffset)
{
	AZ::Vector3 c = AZ::Vector3(gridCoordinates.x, gridCoordinates.y, 0) + cellOffset;
	c = m_aabb.GetMin() + c * (m_aabb.GetMax() - m_aabb.GetMin()) / m_resolution;

	auto *height = m_heightField.GetCellSecure(gridCoordinates);
	if (height)c.SetZ(*height);

	return c;
}

AZ::Vector2 NavigationAreaComponent::GetTileSize()
{
	EBUS_EVENT_ID_RESULT(m_aabb, GetEntityId(), LmbrCentral::ShapeComponentRequestsBus, GetEncompassingAabb);

	auto cell_size = (m_aabb.GetMax() - m_aabb.GetMin()) / m_resolution;

	return AZ::Vector2(cell_size.GetX(), cell_size.GetY());
}

void NavigationAreaComponent::UpdateHeightfield()
{
	EBUS_EVENT_ID_RESULT(m_aabb, GetEntityId(), LmbrCentral::ShapeComponentRequestsBus, GetEncompassingAabb);

	coordinates current_coordinates{ 0,0 };
	auto &field = m_heightField;

	auto cell_size = (m_aabb.GetMax() - m_aabb.GetMin()) / m_resolution;

	m_maxHeight = -1024;
	m_minHeight = 1024;

	for (auto &row : field.data)
	{
		for (auto &cell : row)
		{
			auto world = ConvertToWorldCoordinates(current_coordinates);

			AZStd::vector<float> values;

			values.push_back(gEnv->p3DEngine->GetTerrainElevation(world.GetX(), world.GetY()));
			values.push_back(gEnv->p3DEngine->GetTerrainElevation(world.GetX() + cell_size.GetX(), world.GetY()));
			values.push_back(gEnv->p3DEngine->GetTerrainElevation(world.GetX(), world.GetY() + cell_size.GetY()));
			values.push_back(gEnv->p3DEngine->GetTerrainElevation(world.GetX() + cell_size.GetX(), world.GetY() + cell_size.GetY()));
			values.push_back(gEnv->p3DEngine->GetTerrainElevation(world.GetX() + cell_size.GetX() / 2.0f, world.GetY() + cell_size.GetY() / 2.0f));

			float sum = std::accumulate(std::begin(values), std::end(values), 0.0);
			float avg = sum / values.size();

			float accum = 0.0;
			std::for_each(std::begin(values), std::end(values), [&](const float d) {
				accum += (d - avg) * (d - avg);
			});

			double stdev = sqrt(accum / (values.size() - 1));

			m_heightField.GetCell(current_coordinates) = stdev > m_maxSlope ? InvalidHeight : avg; //if standard deviation is too big the tile is flagged as unpathable


			if (m_minHeight > avg)m_minHeight = avg;
			if (m_maxHeight < avg)m_maxHeight = avg;

			current_coordinates.x++;
		}
		current_coordinates.x = 0;
		current_coordinates.y++;
	}

	sLOG("NavigationAreaComponent Heightfield updated");
}

void NavigationAreaComponent::SetTilesResolution(AZ::Vector2 resolution)
{
	m_resolution = AZ::Vector3((int)resolution.GetX(), (int)resolution.GetY(), 1);
}
AZ::Vector2 NavigationAreaComponent::GetTilesResolution()
{
	return AZ::Vector2(m_resolution.GetX(), m_resolution.GetY());
}

AZ::Vector3 NavigationAreaComponent::GetAabbMin()
{
	return m_aabb.GetMin();
}
AZ::Vector3 NavigationAreaComponent::GetAabbMax()
{
	return m_aabb.GetMax();
}

bool spark::NavigationAreaComponent::IsPointInside(AZ::Vector3 pos)
{
	pos.SetZ(m_aabb.GetMin().GetZ() + 0.01f);
	return m_aabb.Contains(pos);
}

bool spark::NavigationAreaComponent::IsUnitInside(UnitId unit)
{
	AZ::Vector3 pos;
	EBUS_EVENT_ID_RESULT(pos, unit, AZ::TransformBus, GetWorldTranslation);
	return IsPointInside(pos);
}

float NavigationAreaComponent::GetTerrainHeightAt(AZ::Vector3 pos) //z coordinate is ignored
{
	return gEnv->p3DEngine->GetTerrainElevation(pos.GetX(), pos.GetY());
}

AZ::EntityId NavigationAreaComponent::GetUnitAt(AZ::Vector3 world)
{
	auto grid = ConvertToGridCoordinates(world);
	auto *fieldUnit = m_unitField.GetCellSecure(grid);

	if (fieldUnit && fieldUnit->size()) return *(fieldUnit->begin());

	return AZ::EntityId();
}
AZStd::vector<AZ::EntityId> spark::NavigationAreaComponent::GetUnitsAt(AZ::Vector3 world)
{
	auto grid = ConvertToGridCoordinates(world);
	auto *fieldUnit = m_unitField.GetCellSecure(grid);

	if (fieldUnit && fieldUnit->size()) return AZStd::vector<UnitId>(fieldUnit->begin(), fieldUnit->end());

	return AZStd::vector<AZ::EntityId>();
}
VisibilityEnum NavigationAreaComponent::IsPointVisibleByTeam(AZ::Vector3 world, TeamId teamId)
{
	auto grid = ConvertToGridCoordinates(world);
	auto *visibility = m_visibilityField.GetCellSecure(grid);

	if (visibility) {
		return (VisibilityEnum)((*visibility >> (teamId * 2)) & 3);
	}

	return NOT_VISIBLE;
}

VisibilityEnum NavigationAreaComponent::IsPointVisibleByUnit(AZ::Vector3 world, UnitId u)
{
	TeamId teamId;
	EBUS_EVENT_ID_RESULT(teamId, u, UnitRequestBus, GetTeamId);
	return IsPointVisibleByTeam(world, teamId);
}

bool NavigationAreaComponent::IsUnitVisibleByUnit(UnitId target, UnitId u)
{
	TeamId teamId;
	EBUS_EVENT_ID_RESULT(teamId, u, UnitRequestBus, GetTeamId);
	return IsUnitVisibleByTeam(target, teamId);
}

bool NavigationAreaComponent::IsUnitVisibleByTeam(UnitId target, TeamId teamId)
{
	AZ::Vector3 world;
	EBUS_EVENT_ID_RESULT(world, target, AZ::TransformBus, GetWorldTranslation);

	Status status;
	EBUS_EVENT_ID_RESULT(status, target, UnitRequestBus, GetStatus);

	int required = status & STEALTH ? VisibilityEnum::TRUE_VISION : VisibilityEnum::VISIBLE;
	return (int)IsPointVisibleByTeam(world, teamId) >= required;//check for invisible units
}

AZ::Vector3 NavigationAreaComponent::GetNearestFreePosition(AZ::Vector3 world)
{
	auto isFree = [&](pathfinder::coordinates grid) {
		auto *cell = m_unitField.GetCellSecure(grid);
		if (cell)
		{
			int blocking = 0;
			for (auto e : *cell)
			{
				EBUS_EVENT_ID_RESULT(blocking, e, NavigationEntityRequestBus, IsBlocking);
				if (blocking & BlockingFlag::SPAWN_BLOCKING)return false;
			}
		}
		return true;
	};

	auto grid = ConvertToGridCoordinates(world);

	if (isFree(grid))return world;


	int ring_radius = 1;
	while (ring_radius < 8)
	{
		int res = ring_radius * 8;
		float d_alpha = M_PI * 2.0f / (float)res;
		float offset = (rand() % res) * d_alpha; //randomize the first direction to check

		for (float alpha = offset; alpha <= (offset + M_PI * 2.0f); alpha += d_alpha)
		{
			ivec2 proposed = grid + vec2(cosf(alpha), sinf(alpha))*ring_radius;
			if(isFree(proposed))return ConvertToWorldCoordinates(proposed, AZ::Vector3(0.5, 0.5, 0));
		}
		ring_radius++;
	}

	return AZ::Vector3::CreateZero();
}


void NavigationAreaComponent::OnNavigationEntityPositionChanged(AZ::EntityId id, AZ::Vector3 previous, AZ::Vector3 current)
{
	//{
	//	auto grid = ConvertToGridCoordinates(previous);
	//	auto *cell = m_unitField.GetCellSecure(grid);
	//	if (cell && cell->IsValid() && *cell == id)cell->SetInvalid();
	//}

	//{
	//	auto grid = ConvertToGridCoordinates(current);
	//	auto *cell = m_unitField.GetCellSecure(grid);
	//	if (cell)*cell = id;
	//}
}


void AddTile(IPersistentDebug* pPersistentDebug, AZ::Vector3 origin, AZ::Vector3 size, const ColorF color, float timeout)
{
	pPersistentDebug->AddLine(AZVec3ToLYVec3(origin), AZVec3ToLYVec3(origin) + Vec3(size.GetX(), 0, 0), color, timeout);
	pPersistentDebug->AddLine(AZVec3ToLYVec3(origin), AZVec3ToLYVec3(origin) + Vec3(0, size.GetY(), 0), color, timeout);

	pPersistentDebug->AddLine(AZVec3ToLYVec3(origin) + Vec3(size.GetX(), 0, 0), AZVec3ToLYVec3(origin) + Vec3(size.GetX(), size.GetY(), 0), color, timeout);
	pPersistentDebug->AddLine(AZVec3ToLYVec3(origin) + Vec3(0, size.GetY(), 0), AZVec3ToLYVec3(origin) + Vec3(size.GetX(), size.GetY(), 0), color, timeout);
}

//void NavigationAreaComponent::OnTick(float deltaTime, AZ::ScriptTimePoint time)
void NavigationAreaComponent::DrawTiles()
{

	////update visibility texture
	//auto console = gEnv->pConsole;
	//ICVar* var = console->GetCVar("g_FogOfWar");

	//if (var && var->GetIVal() > 0)
	//{
	//	AZStd::set<TeamId> teams;


	//	AZ::EntityId localPlayer;
	//	EBUS_EVENT_RESULT(localPlayer, GameNetSyncRequestBus, GetLocalPlayer);
	//	if (localPlayer.IsValid())
	//	{
	//		TeamId teamId = 0;
	//		EBUS_EVENT_ID_RESULT(teamId, localPlayer, GamePlayerInterfaceBus, GetTeamId);
	//		teams.insert(teamId);
	//	}

	//	//clear the visibility field
	//	ClearVisibilityField();

	//	//update the visibility field
	//	UpdateVisibilityFieldForTeams(teams);

	//	//update the texture
	//	UpdateVisibilityTexture();
	//}


	//if (m_debug)
	{
		//render the grid
		auto *pPersistentDebug = gEnv->pGame->GetIGameFramework()->GetIPersistentDebug();

		if (pPersistentDebug)
		{
			auto tile_size = (m_aabb.GetMax() - m_aabb.GetMin()) / m_resolution;
			coordinates current_coordinates{ 0,0 };
			auto &field = m_heightField;

			const ColorF green(0.000f, 1.000f, 0.000f);
			const ColorF blue(0.000f, 0.000f, 1.000f, 0.5f);
			const ColorF red(1.000f, 0.000f, 0.000f, 0.5f);
			const ColorF orange(1.0f, 0.5f, 0.0f, 0.5f);

			const ColorF palette[] = { Col_Orange,Col_Yellow,Col_Cyan,Col_Violet,Col_Gray,Col_Azure,Col_DarkOliveGreen,Col_Khaki };

			pPersistentDebug->Begin(GetEntityId().ToString().c_str(), true);
			//pPersistentDebug->Begin("NavArea", true);

			for (auto &row : field.data)
			{
				for (auto &cell : row)
				{
					auto world = ConvertToWorldCoordinates(current_coordinates);

					bool visible = false;
					EBUS_EVENT_RESULT(visible, CameraRequestBus, IsPointVisible, world);

					if (visible)
					{
						ColorF color = blue;

						auto visibility = m_visibilityField.GetCell(current_coordinates);
						if (visibility) color = palette[visibility % 8];

						if (!m_unitField.GetCell(current_coordinates).empty())//.IsValid())
							color = red;

						AddTile(pPersistentDebug, world, tile_size / 1.1f, color, 0.1);
						//pPersistentDebug->AddPlanarDisc(AZVec3ToLYVec3(world), 0, tile_size.GetX()/2.1f, blue, 0.1);
					}
					current_coordinates.x++;
				}
				current_coordinates.x = 0;
				current_coordinates.y++;
			}
		}
	}

}

void NavigationAreaComponent::BindMiniMapTexture()
{
	IRenderer* renderer = gEnv->pRenderer;
	if (renderer)
	{
		const int width = m_resolution.GetX();
		const int height = m_resolution.GetY();

		uint8 *data = new uint8[width * height * 4];

		const AZ::Color palette_start = AZ::Color((AZ::u8)112, 66, 20, 255); //sepia color
		const AZ::Color palette_end = AZ::Color((AZ::u8)245, 222, 179, 255); //wheat color

		coordinates current_coordinates{ 0,0 };
		for (auto &row : m_heightField.data)
		{
			for (auto &cell : row)
			{
				float normalized_value = (cell - m_minHeight) / (m_maxHeight - m_minHeight);
				AZ::Color color = palette_start.Lerp(palette_end, normalized_value);

				size_t index = ((height - current_coordinates.y - 1)*width + current_coordinates.x) * 4;
				data[index] = color.GetR8(); //cell.size() * step;
				data[index + 1] = color.GetG8();
				data[index + 2] = color.GetB8();
				data[index + 3] = color.GetA8();

				current_coordinates.x++;
			}
			current_coordinates.x = 0;
			current_coordinates.y++;
		}
		//m_visibilityMap->UpdateTextureRegion((uint8*)data, 0, 0, 0, width, height, 1, eTF_R8);

		renderer->DownLoadToVideoMemory((uint8*)data, width, height, eTF_R8G8B8A8, eTF_R8G8B8A8, 1, true, 2, 0, "MiniMapTex");
		delete data;
	}
}

void NavigationAreaComponent::BindVisibilityTexture()
{
	const int width = m_resolution.GetX();
	const int height = m_resolution.GetY();
	IRenderer* renderer = gEnv->pRenderer;

	int textureId = renderer->DownLoadToVideoMemory(nullptr, width, height, eTF_R8, eTF_R8, 1, true, 2, 0, "FogOfWarTex");
	m_visibilityMap = renderer->EF_GetTextureByID(textureId);

	auto console = gEnv->pConsole;
	auto SetValue = [](ICVar* var, float value) { if (var)var->Set(value); };

	SetValue(console->GetCVar("g_FogOfWar_min_x"), m_aabb.GetMin().GetX());
	SetValue(console->GetCVar("g_FogOfWar_min_y"), m_aabb.GetMin().GetY());
	SetValue(console->GetCVar("g_FogOfWar_max_x"), m_aabb.GetMax().GetX());
	SetValue(console->GetCVar("g_FogOfWar_max_y"), m_aabb.GetMax().GetY());

	//gEnv->pRenderer->ForceUpdateGlobalShaderParameters();
}

void NavigationAreaComponent::UnbindVisibilityTexture()
{

}

void NavigationAreaComponent::UpdateMinimapTexture()
{

}

void NavigationAreaComponent::ClearVisibilityField()
{
	//clear the visibility field (very inefficient -but constant complexity)
	for (auto &row : m_visibilityField.data)
	{
		std::fill(row.begin(), row.end(), 0);
		/*for (auto &cell : row)
		{
			cell = 0;
		}*/
	}
}
void NavigationAreaComponent::UpdateVisibilityFieldForTeams(AZStd::set<TeamId> teams)
{
	auto tile_size = GetTileSize();

	AZStd::vector<AZ::EntityId> units;
	EBUS_EVENT_ID_RESULT(units, GetEntityId(), LmbrCentral::TriggerAreaRequestsBus, GetEntitiesInside);

	//update the visibility field
	for (auto u : units)
	{
		TeamId teamId;
		EBUS_EVENT_ID_RESULT(teamId, u, UnitRequestBus, GetTeamId);

		//check if need to be considered
		if (!teams.empty() && teams.find(teamId) == teams.end())continue;

		EBUS_EVENT_ID(u, UnitRequestBus, SetVisible, true);

		AZ::Vector3 world;
		EBUS_EVENT_ID_RESULT(world, u, AZ::TransformBus, GetWorldTranslation);

		auto grid = ConvertToGridCoordinates(world);
		//auto *fieldUnit = m_unitField.GetCellSecure(grid);
		//if(fieldUnit) *fieldUnit = u;

		//calculate the visibility
		float ray_height = world.GetZ() + 1.0f;

		float vision_range = 0;
		float vision_type = 0;
		EBUS_EVENT_RESULT(vision_range, VariableManagerRequestBus, GetValue, VariableId(u, "vision_range"));
		EBUS_EVENT_RESULT(vision_type, VariableManagerRequestBus, GetValue, VariableId(u, "vision_type"));

		if ((int)vision_type == VisibilityEnum::NOT_VISIBLE || vision_range <= 0.0f)
		{
			continue;
		}

		int visibility_type = min(3, max(0, (int)vision_type)) << (teamId * 2);

		float range = vision_range / (float)tile_size.GetX();

		float res = range * 8;
		float d_alpha = M_PI * 2.0f / res;

		for (float alpha = 0; alpha <= M_PI * 2.0f; alpha += d_alpha)
		{
			grid_raycast(grid, grid + vec2(cosf(alpha), sinf(alpha))*range,
				[&, ray_height](ivec2 c)->bool
			{
				auto *height = m_heightField.GetCellSecure(c);

				if (!height || *height > ray_height)return false;

				auto* entities = m_unitField.GetCellSecure(c);
				
				auto* blocking = m_visionBlockingField.GetCellSecure(c);

				/*int blocking = 0;
				if (entities)
				{
					for (auto e : *entities)
					{		
						EBUS_EVENT_ID_RESULT(blocking, e, NavigationEntityRequestBus, IsBlocking);
						if (blocking & BlockingFlag::VISION_BLOCKING)
						{
							break;
						}
					}
				}*/

				auto *cell = m_visibilityField.GetCellSecure(c);
				if (cell) {
					*cell |= visibility_type;
				}

				return blocking && !*blocking;
			});
		}
	}

	//update the visibility of units
	if (teams.empty())
	{
		for (auto u : units)
		{
			EBUS_EVENT_ID(u, UnitRequestBus, SetVisible, true);
		}
	}
	else
	{
		for (auto u : units)
		{
			TeamId teamId;
			EBUS_EVENT_ID_RESULT(teamId, u, UnitRequestBus, GetTeamId);

			//check if need to be considered
			if (teams.find(teamId) != teams.end())continue;

			AZ::Vector3 world;
			EBUS_EVENT_ID_RESULT(world, u, AZ::TransformBus, GetWorldTranslation);

			auto grid = ConvertToGridCoordinates(world);
			auto *cell = m_visibilityField.GetCellSecure(grid);

			int visibility = cell ? *cell : 0;
			int max_visibility = 0;

			for (auto team : teams)
			{
				max_visibility = AZStd::max(max_visibility,((visibility >> (team * 2)) & 3));
			}

			Status status;
			EBUS_EVENT_ID_RESULT(status, u, UnitRequestBus, GetStatus);
			int required = status & STEALTH ? VisibilityEnum::TRUE_VISION : VisibilityEnum::VISIBLE;

			bool visible = max_visibility >= required;
			EBUS_EVENT_ID(u, UnitRequestBus, SetVisible, (bool)visible);
		}
	}
}

void NavigationAreaComponent::UpdateVisibilityTexture()
{
	if (!m_visibilityMap)return;

	const int width = m_resolution.GetX();
	const int height = m_resolution.GetY();

	uint8 *data = new uint8[width * height];

	const uint8 black = 0x00;
	const uint8 white = 0xff;
	coordinates current_coordinates{ 0,0 };
	for (auto &row : m_visibilityField.data)
	{
		for (auto &cell : row)
		{
			data[(size_t)(current_coordinates.y*width + current_coordinates.x)] = cell ? white : black; //cell.size() * step;


			current_coordinates.x++;
		}
		current_coordinates.x = 0;
		current_coordinates.y++;
	}
	//for (auto &row : m_visibilityField.data)
	//{
	//	for (auto &cell : row)
	//	{
	//		data[(size_t)(current_coordinates.y*width + current_coordinates.x)] = cell ? white : black;


	//		current_coordinates.x++;
	//	}
	//	current_coordinates.x = 0;
	//	current_coordinates.y++;
	//}

	m_visibilityMap->UpdateTextureRegion((uint8*)data, 0, 0, 0, width, height, 1, eTF_R8);
	delete data;

}
