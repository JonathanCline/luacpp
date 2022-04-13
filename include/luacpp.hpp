#pragma once

#include <lua.hpp>


#include <memory>
#include <string>
#include <string_view>


#if __has_include(<concepts>)
#include <concepts>
#define LUA_CPP_CONCEPTS
#endif

#include <iostream>

namespace lua
{
#if false
	namespace impl
	{
		template <typename TagT>
		struct basic_index
		{
		public:
			using rep = int;
			constexpr rep value() const { return this->v_; };

			constexpr explicit operator rep() const noexcept { return this->value(); };

			constexpr bool operator==(const basic_index& rhs) const noexcept
			{
				auto& lhs = *this;
				return lhs.value() == rhs.value();
			};
			constexpr bool operator!=(const basic_index& rhs) const noexcept
			{
				auto& lhs = *this;
				return lhs.value() != rhs.value();
			};

			constexpr basic_index() noexcept = default;
			constexpr explicit basic_index(rep v) noexcept :
				v_(v)
			{};
		private:
			rep v_;
		};
		using index = impl::basic_index<struct index_tag>;
		namespace literals
		{
			consteval index operator""_idx(unsigned long long n) { return index(index::rep(n)); };
		};
		constexpr inline auto ridx_globals = index(LUA_RIDX_GLOBALS);
	};
#endif

	using state = lua_State;
	struct state_deleter
	{
		void operator()(state* _state) const { lua_close(_state); };
	};
	using unique_state = std::unique_ptr<state, state_deleter>;

	using alloc_fn = lua_Alloc;

	inline state* newstate(alloc_fn f, void* ud)
	{
		return lua_newstate(f, ud);
	};
	inline state* newstate()
	{
		return luaL_newstate();
	};
	
	inline void close(state* l)
	{
		lua_close(l);
	};
	inline void close(unique_state l)
	{
		l.reset();
	};

	inline void copy(state* l, int fromIdx, int toIdx)
	{
		lua_copy(l, fromIdx, toIdx);
	};
	inline void copy(state* l, int fromIdx)
	{
		lua_pushvalue(l, fromIdx);
	};

	struct nil_t { constexpr explicit nil_t() noexcept = default; };
	constexpr inline auto nil = nil_t();

	//inline void push(state* l) { lua_pushnil(l); };

	inline void push(state* l, nil_t v) { lua_pushnil(l); };
	inline void push(state* l, lua_Integer v) { lua_pushinteger(l, v); };
	inline void push(state* l, lua_Number v) { lua_pushnumber(l, v); };
	inline void push(state* l, bool v) { lua_pushboolean(l, v); };
	inline void push(state* l, const char* v) { lua_pushstring(l, v); };
	inline void push(state* l, const char* v, size_t len) { lua_pushlstring(l, v, len); };
	inline void push(state* l, const std::string& v) { push(l, v.c_str(), v.size()); };
	inline void push(state* l, const std::string_view& v) { push(l, v.data(), v.size()); };
	inline void push(state* l, int (*_fn)(lua_State*))
	{
		lua_pushcfunction(l, _fn);
	};
	inline void push(state* l, int (*_fn)(lua_State*), int _upValues)
	{
		lua_pushcclosure(l, _fn, _upValues);
	};

	inline void pop(state* l, int n) { lua_pop(l, n); };
	inline void pop(state* l) { pop(l, 1); };
	
	inline auto abs(state* _lua, int idx) { return ::lua_absindex(_lua, idx); };

	inline int top(state* _lua) { return lua_gettop(_lua); };
	inline void settop(state* _lua, int idx) { lua_settop(_lua, idx); };

	inline void remove(state* _lua, int _index) { lua_remove(_lua, _index); }
	

	enum class type  
	{
		nil = LUA_TNIL,
		none = LUA_TNONE,
		function = LUA_TFUNCTION,
		thread = LUA_TTHREAD,
		string = LUA_TSTRING,
		number = LUA_TNUMBER,
		boolean = LUA_TBOOLEAN,
		userdata=LUA_TUSERDATA,
		table= LUA_TTABLE,
		light_userdata= LUA_TLIGHTUSERDATA,
	};
	
	inline type type_of(state* l, int idx) { return type(lua_type(l, idx)); };
	inline const char* type_name(state* l, type t) { return lua_typename(l, (int)t); };
	inline const char* type_name_of(state* l, int idx) { return type_name(l, type_of(l, idx)); };

	inline state* newthread(state* l) { return lua_newthread(l); };
	inline int resetthread(state* l) { return lua_resetthread(l); };
	inline void pushthread(state* l) { lua_pushthread(l); };


	
	template <typename UserdataT = void>
	struct basic_alloc
	{
		using pointer = UserdataT*;
		using reference = UserdataT&;

		pointer userdata() const { return this->ud_; };
		
		pointer operator->() const { return this->userdata(); };
		reference operator*() const { return *this->userdata(); };

		alloc_fn get_fn() const { return this->fn_; };

		constexpr basic_alloc() noexcept = default;
		constexpr explicit basic_alloc(alloc_fn _fn, pointer _ud) noexcept : fn_(_fn), ud_(_ud) {};

		alloc_fn fn_;
		pointer ud_;
	};

	template <>
	struct basic_alloc<void>
	{
		using pointer = void*;

		pointer userdata() const { return this->ud_; };
		alloc_fn get_fn() const { return this->fn_; };

		constexpr basic_alloc() noexcept = default;
		constexpr explicit basic_alloc(alloc_fn _fn, pointer _ud) noexcept : fn_(_fn), ud_(_ud) {};

		alloc_fn fn_;
		pointer ud_;
	};

	using alloc = basic_alloc<void>;


	template <typename UserdataT = void>
	inline basic_alloc<UserdataT> getalloc(state* l)
	{
		void* ud;
		auto fn = lua_getallocf(l, &ud);
		return basic_alloc<UserdataT>(fn, static_cast<UserdataT*>(ud));
	};
	
	inline void setalloc(state* l, alloc_fn fn, void* ud)
	{
		lua_setallocf(l, fn, ud);
	};
	template <typename UserdataT>
	inline void setalloc(state* l, const basic_alloc<UserdataT>& _alloc)
	{
		lua_setallocf(l, _alloc.fn_, _alloc.userdata());
	};

	enum class status_code : int
	{
		ok = LUA_OK,
		yield = LUA_YIELD,
		err_err = LUA_ERRERR,
		err_file = LUA_ERRFILE,
		err_mem = LUA_ERRMEM,
		err_syntax = LUA_ERRSYNTAX,
		err_run = LUA_ERRRUN,
	};

	inline status_code status(state* _lua) { return status_code(::lua_status(_lua)); };
	
	struct resume_result
	{
	public:

		explicit operator bool() const = delete;

		constexpr status_code status() const noexcept { return this->status_; }
		constexpr operator status_code() const noexcept { return this->status(); };
		
		constexpr bool is_error() const noexcept
		{
			const auto _status = this->status();

			// ok and yield are not errors
			return (_status != status_code::ok && _status != status_code::yield);
		};

		constexpr int nrets() const { return this->nrets_; };

		constexpr resume_result() = default;
		constexpr resume_result(status_code _status, int _nrets) :
			status_(_status), nrets_(_nrets)
		{};

	private:
		status_code status_;
		int nrets_;
	};
	
	inline resume_result resume(state* _thread, int _nargs = 0, state* _from = nullptr)
	{
		int _nrets = 0;
		auto _status = ::lua_resume(_thread, _from, _nargs, &_nrets);
		return resume_result{ status_code(_status), _nrets };
	};
	inline int yield(state* _lua, int _nargs = 0)
	{
		return lua_yield(_lua, _nargs);
	};

	inline void call(state* _lua, int _nargs = 0, int _nrets = LUA_MULTRET)
	{
		lua_call(_lua, _nargs, _nrets);
	};
	inline status_code pcall(state* _lua, int _nargs = 0, int _nrets = LUA_MULTRET, int _messageFn = 0)
	{
		const auto _status = lua_pcall(_lua, _nargs, _nrets, _messageFn);
		return status_code(_status);
	};
	

	inline void newtable(state* _lua, int _sequencedArgCount, int _namedArgCount)
	{
		lua_createtable(_lua, _sequencedArgCount, _namedArgCount);
	};
	inline void newtable(state* _lua)
	{
		newtable(_lua, 0, 0);
	};

	inline void setfield(state* _lua, int _tableIdx, const char* _key) { lua_setfield(_lua, _tableIdx, _key); };
	inline type getfield(state* _lua, int _tableIdx, const char* _key) { return type(lua_getfield(_lua, _tableIdx, _key)); };

	inline void rawset(state* _lua, int _index) { lua_rawset(_lua, _index); };
	inline void rawset(state* _lua, int _index, lua_Integer _key) { lua_rawseti(_lua, _index, _key); };
	inline void rawset(state* _lua, int _index, void* _key) { lua_rawsetp(_lua, _index, _key); };
	inline void rawset(state* _lua, int _index, const char* _key)
	{
		_index = abs(_lua, _index);
		auto _valueIndex = abs(_lua, -1);

		push(_lua, _key);
		copy(_lua, _valueIndex);
		rawset(_lua, _index);
		pop(_lua);
	};

	inline type rawget(state* _lua, int _index) { return type(lua_rawget(_lua, _index)); };
	inline type rawget(state* _lua, int _index, lua_Integer _key) { return type(lua_rawgeti(_lua, _index, _key)); };
	inline type rawget(state* _lua, int _index, void* _key) { return type(lua_rawgetp(_lua, _index, _key)); };
	inline type rawget(state* _lua, int _index, const char* _key)
	{
		_index = abs(_lua, _index);
		push(_lua, _key);
		return rawget(_lua, _index);
	};

	inline lua_Unsigned rawlen(state* _lua, int _index) { return lua_rawlen(_lua, _index); };

	inline void get_or_create_table(state* _lua, int _tableIndex, void* _key)
	{
		if (rawget(_lua, _tableIndex, _key) != lua::type::table)
		{
			pop(_lua);
			newtable(_lua);
			copy(_lua, -1);
			rawset(_lua, _tableIndex, _key);
		};
	};
	inline void get_or_create_table(state* _lua, int _tableIndex, const char* _key)
	{
		_tableIndex = abs(_lua, _tableIndex);
		
		// Push the key onto the stack and get the value
		if (rawget(_lua, _tableIndex, _key) != lua::type::table)
		{
			// Remove faulty value.
			pop(_lua);

			// Create the new table.
			newtable(_lua);

			// Copy value to top
			copy(_lua, -1);

			// Set field.
			rawset(_lua, _tableIndex, _key);
		};
	};
	inline void get_or_create_table(state* _lua, int _tableIndex, int _key)
	{
		if (rawget(_lua, _tableIndex, _key) != lua::type::table)
		{
			pop(_lua);
			newtable(_lua);
			copy(_lua, -1);
			rawset(_lua, _tableIndex, _key);
		};
	};



	inline void setglobal(state* _lua, const char* _name) { lua_setglobal(_lua, _name); };
	inline type getglobal(state* _lua, const char* _name) { return type(lua_getglobal(_lua, _name)); };

	inline void setglobal(state* _lua, const char* _name, size_t _nameLen)
	{
		const auto _top = top(_lua);
		lua_rawgeti(_lua, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
		const auto _tableIdx = top(_lua);
		push(_lua, _name, _nameLen);
		copy(_lua, _top);
		lua_rawset(_lua, _tableIdx);
		settop(_lua, _top);
		pop(_lua);
	};
	inline type getglobal(state* _lua, const char* _name, size_t _nameLen)
	{
		lua_rawgeti(_lua, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
		push(_lua, _name, _nameLen);
		const auto _type = type(lua_rawget(_lua, -2));
		lua_remove(_lua, -2);
		return _type;
	};


	inline void foreach_on_stack(state* l, auto&& fn)
	{
		const auto _max = top(l);
		for (int n = 1; n <= _max; ++n)
		{
			std::invoke(fn, l, n);
		};
	};


	inline int next(state* _lua, int _index)
	{
		return lua_next(_lua, _index);
	};


	/**
	 * @tparam OpT Should match the signature : void foo(state* _lua, int _keyIndex, int _valueIndex)
	*/
	template <typename OpT>
#if defined(LUA_CPP_CONCEPTS)
	requires std::invocable<OpT, state*, int, int>
#endif
	inline void foreach_pair_in_table(state* _lua, int _tableIndex, OpT&& _fn)
	{
		_tableIndex = abs(_lua, _tableIndex);

		// Push nil for first key
		push(_lua, nil);
		while (next(_lua, _tableIndex) != 0)
		{
			// -1 = value
			// -2 = key

			auto _top = top(_lua);
			auto _keyIdx = _top - 1;
			auto _valueIdx = _top - 0;


			// Invoke the function.
			std::invoke(_fn, _lua, _keyIdx, _valueIdx);
			// Remove the value from the stack.
			remove(_lua, _valueIdx);

			// Move the old key to the top of the stack if isn't on the top already.
			if (top(_lua) != _keyIdx)
			{
				copy(_lua, _keyIdx);
				remove(_lua, _keyIdx);
			};
		};
	};


};