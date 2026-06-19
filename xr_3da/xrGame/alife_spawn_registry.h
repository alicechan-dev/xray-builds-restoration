////////////////////////////////////////////////////////////////////////////
//	Module 		: alife_spawn_registry.h
//	Created 	: 15.01.2003
//  Modified 	: 12.05.2004
//	Author		: Dmitriy Iassenev
//	Description : ALife spawn registry
////////////////////////////////////////////////////////////////////////////

#pragma once

#include "alife_spawn_registry_header.h"
#include "xrServer_Objects_ALife_Monsters.h"
#include "game_graph.h"

class CALifeSpawnRegistry : CRandom {
public:
	typedef CGameGraph::LEVEL_POINT_VECTOR	ARTEFACT_SPAWNS;
	struct SSpawnGroupMember {
		ALife::_SPAWN_ID					id;
		CSE_ALifeDynamicObject				*object;
		float								probability;
	};
	typedef xr_vector<SSpawnGroupMember>	SPAWN_GROUP_MEMBER_VECTOR;
	typedef xr_map<ALife::_SPAWN_ID,CSE_SpawnGroup*> SPAWN_GROUP_MAP;
	typedef xr_map<ALife::_SPAWN_ID,SPAWN_GROUP_MEMBER_VECTOR> SPAWN_GROUP_MEMBERS;

protected:
	CALifeSpawnHeader						m_header;
	ALife::D_OBJECT_P_VECTOR				m_spawns;
	ALife::D_OBJECT_P_VECTOR				m_spawns_by_id;
	SPAWN_GROUP_MAP							m_spawn_groups;
	SPAWN_GROUP_MEMBERS						m_group_members;
	xr_vector<ALife::_SPAWN_ID>				m_member_group;
	ARTEFACT_SPAWNS							m_artefact_spawn_positions;
	ALife::ITEM_SET_MAP						m_artefact_anomaly_map;
	LPSTR									m_spawn_name;

protected:
			void							load_flat					(IReader &file_stream);
			void							load_graph					(IReader &file_stream);
			void							register_spawn_object		(CSE_Abstract *object, ALife::_SPAWN_ID spawn_id);
			void							register_spawn_edge			(ALife::_SPAWN_ID source_id, ALife::_SPAWN_ID target_id, float probability);

public:
											CALifeSpawnRegistry			(LPCSTR section);
	virtual									~CALifeSpawnRegistry		();
	virtual void							load						(IReader &file_stream);
	virtual void							save						(IWriter &memory_stream);
			void							load						(IReader &file_stream, LPCSTR game_name);
			void							load						(LPCSTR spawn_name);
	IC		const CALifeSpawnHeader			&header						() const;
	IC		const ALife::D_OBJECT_P_VECTOR	&spawns						() const;
	IC		void							assign_artefact_position	(CSE_ALifeAnomalousZone	*anomaly, CSE_ALifeDynamicObject *object);
	IC		const ALife::ITEM_SET_MAP		&artefact_anomaly_map		() const;
	IC		bool							valid_spawn_id				(ALife::_SPAWN_ID spawn_id) const;
	IC		CSE_ALifeDynamicObject			*spawn						(ALife::_SPAWN_ID spawn_id) const;
	IC		bool							is_spawn_group				(ALife::_SPAWN_ID spawn_id) const;
	IC		CSE_SpawnGroup					*spawn_group				(ALife::_SPAWN_ID spawn_id) const;
	IC		ALife::_SPAWN_ID				spawn_group_id				(ALife::_SPAWN_ID member_id) const;
	IC		const SPAWN_GROUP_MEMBERS		&spawn_groups				() const;
	IC		const SPAWN_GROUP_MEMBER_VECTOR	&spawn_group_members			(ALife::_SPAWN_ID group_id) const;
	IC		const SPAWN_GROUP_MEMBER_VECTOR	&spawn_group_members_by_member	(ALife::_SPAWN_ID member_id) const;
};

#include "alife_spawn_registry_inline.h"
