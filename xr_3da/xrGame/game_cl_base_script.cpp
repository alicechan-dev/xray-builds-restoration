#include "stdafx.h"
#include "game_cl_base.h"
#include "script_space.h"

using namespace luabind;

static void zone_map_entities_push_back(xr_vector<SZoneMapEntityData> &self, const SZoneMapEntityData &value)
{
	self.push_back(value);
}


void SZoneMapEntityData::script_register(lua_State *L)
{
	module(L)
		[
			luabind::class_<SZoneMapEntityData>("SZoneMapEntityData")
			.def(	constructor<>()								)
			.def_readwrite("pos",				&SZoneMapEntityData::pos	)
			.def_readwrite("color",				&SZoneMapEntityData::color	),

			luabind::class_< xr_vector<SZoneMapEntityData> >("ZoneMapEntities")
				.def("push_back",				&zone_map_entities_push_back)
		];
}

void RPoint::script_register(lua_State *L)
{
	module(L)
		[
			luabind::class_<RPoint>("RPoint")
			.def(	constructor<>()						)
			.def_readwrite("P",				&RPoint::P	)
			.def_readwrite("A",				&RPoint::A	)
		];
}
