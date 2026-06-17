////////////////////////////////////////////////////////////////////////////
//	Module 		: script_space_forward.h
//	Created 	: 21.07.2004
//  Modified 	: 21.07.2004
//	Author		: Dmitriy Iassenev
//	Description : Script space forward declarations
////////////////////////////////////////////////////////////////////////////

#pragma once

#include <luabind/object.hpp>
#include <luabind/detail/ref.hpp>

namespace luabind {
	template<class T> class functor {
	private:
		object m_function;

	public:
		functor()
		{
		}

		functor(lua_State *L, const detail::lua_reference &reference)
		{
			if (!reference.is_valid())
				return;

			reference.get(L);
			m_function = object(from_stack(L, -1));
			lua_pop(L, 1);
		}

		template <typename... Args>
		T operator()(const Args &... args)
		{
			return object_cast<T>(m_function(args...));
		}
	};

	template<> class functor<void> {
	private:
		object m_function;

	public:
		functor()
		{
		}

		functor(lua_State *L, const detail::lua_reference &reference)
		{
			if (!reference.is_valid())
				return;

			reference.get(L);
			m_function = object(from_stack(L, -1));
			lua_pop(L, 1);
		}

		template <typename... Args>
		void operator()(const Args &... args)
		{
			m_function(args...);
		}
	};
};
