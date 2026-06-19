////////////////////////////////////////////////////////////////////////////
//	Module 		: alife_spawn_registry_inline.h
//	Created 	: 15.01.2003
//  Modified 	: 12.05.2004
//	Author		: Dmitriy Iassenev
//	Description : ALife spawn registry inline functions
////////////////////////////////////////////////////////////////////////////

#pragma once

IC	const CALifeSpawnHeader &CALifeSpawnRegistry::header	() const
{
	return				(m_header);
}

IC	void CALifeSpawnRegistry::assign_artefact_position		(CSE_ALifeAnomalousZone	*anomaly, CSE_ALifeDynamicObject *object)
{
	object->m_tGraphID		= anomaly->m_tGraphID;
	u32						index = anomaly->m_dwStartIndex + randI(anomaly->m_wArtefactSpawnCount);
	object->o_Position		= m_artefact_spawn_positions[index].level_point();
	object->m_tNodeID		= m_artefact_spawn_positions[index].level_vertex_id();
	object->m_fDistance		= m_artefact_spawn_positions[index].distance();
#ifdef DEBUG
	if (psAI_Flags.test(aiALife)) {
		Msg					("[LSS] Zone %s[%f][%f][%f] %d: generated artefact position %s[%f][%f][%f]",anomaly->s_name_replace,VPUSH(anomaly->o_Position),anomaly->m_dwStartIndex,object->s_name_replace,VPUSH(object->o_Position));
	}
#endif
}

IC	const ALife::D_OBJECT_P_VECTOR &CALifeSpawnRegistry::spawns	() const
{
	return					(m_spawns);
}

IC	const ALife::ITEM_SET_MAP &CALifeSpawnRegistry::artefact_anomaly_map	() const
{
	return					(m_artefact_anomaly_map);
}

IC	bool CALifeSpawnRegistry::valid_spawn_id				(ALife::_SPAWN_ID spawn_id) const
{
	return					((spawn_id >=0) && (spawn_id < (ALife::_SPAWN_ID)m_spawns_by_id.size()) && m_spawns_by_id[spawn_id]);
}

IC	CSE_ALifeDynamicObject *CALifeSpawnRegistry::spawn	(ALife::_SPAWN_ID spawn_id) const
{
	if (!valid_spawn_id(spawn_id))
		return				(0);
	return					(m_spawns_by_id[spawn_id]);
}

IC	bool CALifeSpawnRegistry::is_spawn_group				(ALife::_SPAWN_ID spawn_id) const
{
	return					(m_spawn_groups.end() != m_spawn_groups.find(spawn_id));
}

IC	CSE_SpawnGroup *CALifeSpawnRegistry::spawn_group		(ALife::_SPAWN_ID spawn_id) const
{
	SPAWN_GROUP_MAP::const_iterator	I = m_spawn_groups.find(spawn_id);
	if (m_spawn_groups.end() == I)
		return				(0);
	return					((*I).second);
}

IC	ALife::_SPAWN_ID CALifeSpawnRegistry::spawn_group_id	(ALife::_SPAWN_ID member_id) const
{
	if ((member_id >= (ALife::_SPAWN_ID)m_member_group.size()) || (ALife::_SPAWN_ID(-1) == m_member_group[member_id]))
		return				(ALife::_SPAWN_ID(-1));
	return					(m_member_group[member_id]);
}

IC	const CALifeSpawnRegistry::SPAWN_GROUP_MEMBERS &CALifeSpawnRegistry::spawn_groups	() const
{
	return					(m_group_members);
}

IC	const CALifeSpawnRegistry::SPAWN_GROUP_MEMBER_VECTOR &CALifeSpawnRegistry::spawn_group_members	(ALife::_SPAWN_ID group_id) const
{
	SPAWN_GROUP_MEMBERS::const_iterator	I = m_group_members.find(group_id);
	if (m_group_members.end() == I) {
		static const SPAWN_GROUP_MEMBER_VECTOR	empty;
		return				(empty);
	}
	return					((*I).second);
}

IC	const CALifeSpawnRegistry::SPAWN_GROUP_MEMBER_VECTOR &CALifeSpawnRegistry::spawn_group_members_by_member	(ALife::_SPAWN_ID member_id) const
{
	ALife::_SPAWN_ID		group_id = spawn_group_id(member_id);
	if (ALife::_SPAWN_ID(-1) == group_id) {
		static const SPAWN_GROUP_MEMBER_VECTOR	empty;
		return				(empty);
	}
	return					(spawn_group_members(group_id));
}
