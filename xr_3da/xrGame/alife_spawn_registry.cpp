////////////////////////////////////////////////////////////////////////////
//	Module 		: alife_spawn_registry.cpp
//	Created 	: 15.01.2003
//  Modified 	: 12.05.2004
//	Author		: Dmitriy Iassenev
//	Description : ALife spawn registry
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "alife_spawn_registry.h"
#include "object_broker.h"
#include "game_base.h"
#include "graph_abstract.h"
#include "server_entity_wrapper.h"

CALifeSpawnRegistry::CALifeSpawnRegistry	(LPCSTR section)
{
	m_spawn_name				= 0;
}

CALifeSpawnRegistry::~CALifeSpawnRegistry	()
{
	ALife::D_OBJECT_P_IT		I = m_spawns.begin();
	ALife::D_OBJECT_P_IT		E = m_spawns.end();
	for ( ; I != E; ++I)
		xr_delete				(*I);
	SPAWN_GROUP_MAP::iterator	G = m_spawn_groups.begin();
	SPAWN_GROUP_MAP::iterator	GE = m_spawn_groups.end();
	for ( ; G != GE; ++G)
		xr_delete				((*G).second);
	xr_free						(m_spawn_name);
}

void CALifeSpawnRegistry::save	(IWriter &memory_stream)
{
	Msg							("* Saving spawn extra info...");
	memory_stream.open_chunk	(SPAWN_CHUNK_DATA);
	memory_stream.w_stringZ		(m_spawn_name);
	memory_stream.close_chunk	();
}

void CALifeSpawnRegistry::load	(IReader &file_stream, LPCSTR game_name)
{
	Msg							("* Loading spawn registry...");
	R_ASSERT2					(file_stream.find_chunk(SPAWN_CHUNK_DATA),"Cannot find chunk SPAWN_CHUNK_DATA!");
	xr_free						(m_spawn_name);
	m_spawn_name				= (LPSTR)xr_malloc(256*sizeof(char));
	file_stream.r_stringZ		(m_spawn_name);

	string256					file_name;
	IReader						*stream;
	R_ASSERT3					(FS.exist(file_name, "$game_spawn$", m_spawn_name, ".spawn"),"Can't find file spawn file:",m_spawn_name);
	int							spawn_age = FS.get_file_age(file_name);
	
	R_ASSERT					(FS.exist(game_name));
	int							game_age = FS.get_file_age(game_name);
	R_ASSERT3					(game_age >= spawn_age,"Delete saved game ",game_name);
	
	string256					graph_file_name;
	FS.update_path				(graph_file_name,"$game_data$",GRAPH_NAME);
	int							graph_age = FS.get_file_age(graph_file_name);
	VERIFY3						(spawn_age >= graph_age,"Rebuild spawn file ",file_name);

	stream						= FS.r_open(file_name);
	load						(*stream);
	FS.r_close					(stream);
}

void CALifeSpawnRegistry::load	(LPCSTR spawn_name)
{
	Msg							("* Loading spawn registry...");
	xr_free						(m_spawn_name);
	m_spawn_name				= xr_strdup(spawn_name);

	string256					file_name;
	IReader						*stream;
	R_ASSERT3					(FS.exist(file_name, "$game_spawn$", m_spawn_name, ".spawn"),"Can't find file spawn file:",m_spawn_name);
	int							spawn_age = FS.get_file_age(file_name);
	
	string256					graph_file_name;
	FS.update_path				(graph_file_name,"$game_data$",GRAPH_NAME);
	int							graph_age = FS.get_file_age(graph_file_name);
	VERIFY3						(spawn_age >= graph_age,"Rebuild spawn file ",file_name);

	stream						= FS.r_open(file_name);
	load						(*stream);
	FS.r_close					(stream);
}

void CALifeSpawnRegistry::load	(IReader &file_stream)
{
	if (file_stream.find_chunk(SPAWN_POINT_CHUNK_VERSION)) {
		load_flat				(file_stream);
		return;
	}

	if (file_stream.find_chunk(0)) {
		load_graph				(file_stream);
		return;
	}

	R_ASSERT2					(false,"Can't find spawn registry header chunk in the 'game.spawn'");
}

void CALifeSpawnRegistry::register_spawn_object	(CSE_Abstract *object, ALife::_SPAWN_ID spawn_id)
{
	VERIFY						(object);
	VERIFY						(spawn_id < m_spawns_by_id.size());

	CSE_SpawnGroup				*spawn_group = smart_cast<CSE_SpawnGroup*>(object);
	if (spawn_group) {
		m_spawn_groups.insert	(mk_pair(spawn_id,spawn_group));
		return;
	}

	CSE_ALifeDynamicObject		*dynamic_object = smart_cast<CSE_ALifeDynamicObject*>(object);
	R_ASSERT2					(dynamic_object,"Non-ALife object in the 'game.spawn'");

	R_ASSERT2					((GAME_SINGLE == object->s_gameid) || (GAME_ANY == object->s_gameid),"Invalid game type!");
	R_ASSERT3					(!dynamic_object->used_ai_locations() || (dynamic_object->m_tNodeID != u32(-1)),"Invalid vertex for object ",dynamic_object->s_name_replace);

	m_spawns_by_id[spawn_id]		= dynamic_object;
	m_spawns.push_back			(dynamic_object);

	CSE_ALifeAnomalousZone		*anomaly = smart_cast<CSE_ALifeAnomalousZone*>(object);
	if (anomaly) {
		ALife::EAnomalousZoneType	type = anomaly->m_tAnomalyType;
		for (u16 i=0, n = anomaly->m_wItemCount; i<n; ++i) {
			ALife::ITEM_SET_PAIR_IT	I = m_artefact_anomaly_map.find(anomaly->m_cppArtefactSections[i]);
			if (m_artefact_anomaly_map.end() != I)
				(*I).second.insert(type);
			else {
				m_artefact_anomaly_map.insert(mk_pair(anomaly->m_cppArtefactSections[i],ALife::U32_SET()));
				I = m_artefact_anomaly_map.find(anomaly->m_cppArtefactSections[i]);
				if ((*I).second.find(type) == (*I).second.end())
					(*I).second.insert(type);
			}
		}
	}

	if (psAI_Flags.test(aiALife))
		Msg						("Spawn point %s is loaded",object->s_name_replace);
}

void CALifeSpawnRegistry::register_spawn_edge	(ALife::_SPAWN_ID source_id, ALife::_SPAWN_ID target_id, float probability)
{
	CSE_SpawnGroup				*group = spawn_group(source_id);
	if (!group)
		return;

	CSE_ALifeDynamicObject		*object = spawn(target_id);
	if (!object)
		return;

	SSpawnGroupMember			member;
	member.id					= target_id;
	member.object				= object;
	member.probability			= probability;
	m_group_members[source_id].push_back(member);
	m_member_group[target_id]	= source_id;
}

void CALifeSpawnRegistry::load_flat	(IReader &file_stream)
{
	m_header.load				(file_stream);
	m_spawns.clear				();
	m_spawns_by_id.clear		();
	m_spawn_groups.clear		();
	m_group_members.clear		();
	m_member_group.clear		();
	m_spawns_by_id.resize		(header().count(),0);
	m_member_group.resize		(header().count(),ALife::_SPAWN_ID(-1));
	m_artefact_anomaly_map.clear();
	NET_Packet					tNetPacket;
	IReader						*S = 0;
	u16							ID;
	int							id;
	for (id=0; id < (int)header().count(); ++id) {
		R_ASSERT2				(0!=(S = file_stream.open_chunk(id)),"Can't find entity chunk in the 'game.spawn'");
		// Spawn
		tNetPacket.B.count		= S->r_u16();
		S->r					(tNetPacket.B.data,tNetPacket.B.count);
		tNetPacket.r_begin		(ID);
		R_ASSERT2				(M_SPAWN == ID,"Invalid packet ID (!= M_SPAWN)!");

		string64				s_name;
		tNetPacket.r_stringZ	(s_name);
		if (psAI_Flags.test(aiALife)) {
			Msg					("Loading spawn point %s",s_name);
		}
		CSE_Abstract			*E = F_entity_Create(s_name);

		R_ASSERT3				(E,"Can't create entity.",s_name);
		E->Spawn_Read			(tNetPacket);
		// Update
		tNetPacket.B.count		= S->r_u16();
		S->r					(tNetPacket.B.data,tNetPacket.B.count);
		tNetPacket.r_begin		(ID);
		R_ASSERT2				(M_UPDATE == ID,"Invalid packet ID (!= M_UPDATE)!");
		E->UPDATE_Read			(tNetPacket);

		VERIFY					(smart_cast<CSE_ALifeObject*>(E));

		register_spawn_object	(E,ALife::_SPAWN_ID(id));
	}
	R_ASSERT2					(0!=(S = file_stream.open_chunk(id++)),"Can't find artefact spawn points chunk in the 'game.spawn'");
	load_data					(m_artefact_spawn_positions,file_stream);
	Msg							("%d spawn points are successfully loaded",id);
}

void CALifeSpawnRegistry::load_graph	(IReader &file_stream)
{
	typedef CGraphAbstract<CServerEntityWrapper*,float,ALife::_SPAWN_ID,u32>	SPAWN_GRAPH;

	m_header.load				(file_stream,0);
	m_spawns.clear				();
	m_spawns_by_id.clear		();
	m_spawn_groups.clear		();
	m_group_members.clear		();
	m_member_group.clear		();
	m_spawns_by_id.resize		(header().count(),0);
	m_member_group.resize		(header().count(),ALife::_SPAWN_ID(-1));
	m_artefact_anomaly_map.clear();

	IReader						*chunk = file_stream.open_chunk(1);
	R_ASSERT2					(chunk,"Can't find spawn graph chunk in the 'game.spawn'");

	SPAWN_GRAPH					spawn_graph;
	load_data					(spawn_graph,*chunk);
	chunk->close				();

	SPAWN_GRAPH::const_vertex_iterator	I = spawn_graph.vertices().begin();
	SPAWN_GRAPH::const_vertex_iterator	E = spawn_graph.vertices().end();
	for ( ; I != E; ++I) {
		CServerEntityWrapper	*wrapper = (*I)->data();
		CSE_Abstract			*object = wrapper->detach_object();
		register_spawn_object	(object,(*I)->vertex_id());
	}

	I							= spawn_graph.vertices().begin();
	for ( ; I != E; ++I) {
		SPAWN_GRAPH::const_iterator	i = (*I)->edges().begin();
		SPAWN_GRAPH::const_iterator	e = (*I)->edges().end();
		for ( ; i != e; ++i)
			register_spawn_edge	((*I)->vertex_id(),(*i).vertex_id(),(*i).weight());
	}

	chunk						= file_stream.open_chunk(2);
	R_ASSERT2					(chunk,"Can't find artefact spawn points chunk in the 'game.spawn'");
	load_data					(m_artefact_spawn_positions,*chunk);
	chunk->close				();

	Msg							("%d spawn points are successfully loaded",m_spawns.size());
}
