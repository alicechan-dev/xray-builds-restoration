# xrSE_Factory Luabind Compatibility Overlay

`xrSE_Factory/luabind` contains local luabind implementation `.cpp` files, but
the matching public headers are missing from this snapshot. The CMake port uses
the supplied external luabind 0.7 headers and a local overlay under
`xrSE_Factory/luabind_compat`.

Shimmed safely:

- `object.lua_state()` intent: `object.interpreter()`
- `object.pushvalue()` intent: `object.push(object.interpreter())`
- old stack-to-object assignment intent: `luabind::object(luabind::from_stack(...))`
- old `object.type()` intent: push the object and call `lua_type`

Not shimmed:

- `detail::proxy_object`
- `detail::proxy_array_object`
- `detail::proxy_raw_object`
- `object::iterator`
- `object.set()` as a member API
- `class_rep::m_table_ref`
- `class_rep::m_default_table_ref`

Those names belonged to a different luabind object/reference model. The local
`luabind/object.cpp` only implements that obsolete object/proxy layer and cannot
be made compatible with luabind 0.7 by simple aliases. The CMake build therefore
does not compile that one implementation file while using luabind 0.7 headers,
which already provide their own object, proxy, iterator, and comparison support.

The overlay is intentionally scoped to `xrSE_Factory` and does not edit external
luabind, Lua, Boost, Loki, gameplay, script, config, or data files.
