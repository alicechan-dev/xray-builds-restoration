#pragma once

// Build 1935 carries luabind implementation .cpp files, but the matching
// public headers are missing. The available external luabind 0.7 headers use
// newer object/handle names. Keep the compatibility surface local to
// xrSE_Factory instead of editing the external headers.

#include <luabind/object.hpp>
#include <luabind/detail/stack_utils.hpp>

namespace xray_luabind_compat
{
	inline lua_State* lua_state(luabind::object const& object)
	{
		return object.interpreter();
	}

	inline int type(luabind::object const& object)
	{
		lua_State* L = lua_state(object);
		object.push(L);
		luabind::detail::stack_pop pop(L, 1);
		return lua_type(L, -1);
	}

	inline void pushvalue(luabind::object const& object)
	{
		object.push(lua_state(object));
	}

	inline luabind::object object_from_stack(lua_State* L, int index = -1)
	{
		return luabind::object(luabind::from_stack(L, index));
	}

	inline void assign_from_stack(luabind::object& object, lua_State* L, int index = -1)
	{
		luabind::object replacement = object_from_stack(L, index);
		object.swap(replacement);
	}
}
