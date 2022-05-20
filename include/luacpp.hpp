#pragma once

/** @file */

#include <lua.hpp>

#include <bit>
#include <array>
#include <string>
#include <memory>
#include <vector>
#include <iterator>
#include <string_view>

#include <cstdint>
#include <cstddef>
#include <cassert>
#include <concepts>

/*
	Type aliases and enums
*/

namespace lua
{
	/**
	 * @brief Alias of the lua_State type.
	*/
	using state = lua_State;

	/**
	 * @brief Shorthand for a lua state pointer.
	*/
	using state_ptr = state*;

	/**
	 * @brief Lua types.
	*/
	enum class type
	{
		nil = LUA_TNIL,
		none = LUA_TNONE,
		function = LUA_TFUNCTION,
		thread = LUA_TTHREAD,
		string = LUA_TSTRING,
		number = LUA_TNUMBER,
		boolean = LUA_TBOOLEAN,
		userdata = LUA_TUSERDATA,
		table = LUA_TTABLE,
		lightuserdata = LUA_TLIGHTUSERDATA,
	};



	/**
	 * @brief Lua state/thread status codes.
	*/
	enum class status_code : int
	{
		/**
		 * @brief No errors
		*/
		ok = LUA_OK,

		/**
		 * @brief The thread (coroutine) yields
		*/
		yield = LUA_YIELD,

		/**
		 * @brief Error while running the message handler
		*/
		err_err = LUA_ERRERR,

		/**
		 * @brief A file-related error (cannot open or read)
		*/
		err_file = LUA_ERRFILE,

		/**
		 * @brief Memory allocation error
		*/
		err_mem = LUA_ERRMEM,

		/**
		 * @brief Syntax error during precompilation
		*/
		err_syntax = LUA_ERRSYNTAX,

		/**
		 * @brief Runtime error
		*/
		err_run = LUA_ERRRUN,
	};

	/**
	 * @brief Lua CFunction pointer type.
	*/
	using cfunction = lua_CFunction;
	
	/**
	 * @brief Lua state deleter function object type.
	*/
	struct state_deleter
	{
		/**
		 * @brief Calls lua_close, deleting a lua state.
		 * @param _state Lua state pointer.
		*/
		void operator()(state* _state) const 
		{ 
			lua_close(_state);
		};
	};

	/**
	 * @brief Owning RAII handle to a lua state.
	*/
	using unique_state = std::unique_ptr<state, state_deleter>;

	/**
	 * @brief Lua allocator function type.
	*/
	using alloc_fn = lua_Alloc;

	/**
	 * @brief Lua chunk reader function type.
	*/
	using reader_fn = lua_Reader;

	/**
	 * @brief Lua chunk writer function type.
	*/
	using writer_fn = lua_Writer;


	/**
	 * @brief "nil" tag type, used to represent a nil value
	*/
	struct nil_t
	{
		constexpr explicit nil_t() noexcept = default;
	};
	
	/**
	 * @brief "nil" tag type value, used to represent a nil value
	*/
	constexpr inline auto nil = nil_t();



	/**
	 * @brief "fail" tag type, used to represent the fail value
	*/
	struct fail_t
	{
		constexpr explicit fail_t() noexcept = default;
	};

	/**
	 * @brief "fail" tag type value, used to represent a fail value
	*/
	constexpr inline auto fail = fail_t();
	



	/**
	 * @brief Customization point for defining how a type can interact with the lua state.
	 * 
	 * @tparam T Type whose stack traits are defined, specialize on this.
	 * @tparam Enable SFINAE specialization point.
	*/
	template <typename T, typename Enable = void>
	struct stack_traits;




	/**
	 * @brief Named lua chunk loading modes.
	*/
	enum class load_mode
	{
		/**
		 * @brief Loads binary or text.
		*/
		bt = 0,

		/**
		 * @brief Loads previously compiled lua binary code.
		*/
		binary,
		b = binary,

		/**
		 * @brief Loads lua source code text.
		*/
		text,
		t = text,
	};

};



/*
	Type invariants
*/

namespace lua
{
	
};



/*
	Basic lua functionality
*/

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

	// Copies n elements from the stack onto the top of the stack.
	inline void copy_n(state_ptr _lua, int _firstFromIdx, int n)
	{
		const auto _firstIdx = lua_absindex(_lua, _firstFromIdx);
		const auto _lastIdx = _firstIdx + n;
		for (auto i = _firstIdx; i != _lastIdx; ++i)
		{
			lua::copy(_lua, i);
		};
	};

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




	namespace impl
	{
		template <typename T, typename ArgTList, typename Enable = void>
		struct is_pushable_args : std::false_type {};

		template <typename T, template <typename... Ts> class ArgTList, typename... Ts>
		struct is_pushable_args <T, ArgTList<Ts...>,
			std::void_t<decltype(stack_traits<std::remove_cvref_t<T>>::push(std::declval<state_ptr>(), std::declval<T>(), std::declval<Ts>()...))>
		> : std::true_type {};


		template <typename T, typename ArgTList, typename Enable = void>
		struct push_result;

		template <typename T, template <typename... Ts> class ArgTList, typename... Ts>
		struct push_result<T, ArgTList<Ts...>, std::enable_if_t<is_pushable_args<T, ArgTList<Ts...>>::value>>
		{
			using type = decltype(
				stack_traits<std::remove_cvref_t<T>>::push(std::declval<state_ptr>(), std::declval<T>(), std::declval<Ts>()...)
			);
		};
	};



	template <typename T, typename... ExtraArgs>
	struct is_pushable : std::bool_constant<impl::is_pushable_args<T, std::tuple<ExtraArgs...>>::value> {};

	template <typename T, typename... ExtraArgs>
	constexpr inline auto is_pushable_v = is_pushable<T, ExtraArgs...>::value;

	template <typename T, typename... ExtraArgs>
	using push_result_t = typename impl::push_result<T, std::tuple<ExtraArgs...>>::type;






	namespace impl
	{
		template <typename T, typename ArgTList, typename Enable = void>
		struct is_pullable_args : std::false_type {};

		template <typename T, template <typename... Ts> class ArgTList, typename... Ts>
		struct is_pullable_args <T, ArgTList<Ts...>,
			std::void_t<decltype(
				stack_traits<std::remove_cvref_t<T>>::to(std::declval<state_ptr>(), std::declval<int>(),
					std::declval<std::remove_reference_t<T>&>(), std::declval<Ts>()...)
			)>
		> : std::true_type {};

		template <typename T, template <typename... Ts> class ArgTList>
		struct is_pullable_args <T, ArgTList<>,
			std::void_t<decltype(
				stack_traits<std::remove_cvref_t<T>>::to(std::declval<state_ptr>(), std::declval<int>(),
					std::declval<std::remove_reference_t<T>&>())
				)>
		> : std::true_type {};

		template <typename T, typename ArgTList, typename Enable = void>
		struct pull_result;

		template <typename T, template <typename... Ts> class ArgTList, typename... Ts>
		struct pull_result<T, ArgTList<Ts...>, std::enable_if_t<is_pullable_args<T, ArgTList<Ts...>>::value>>
		{
			using type = decltype(
				stack_traits<std::remove_cvref_t<T>>::to(std::declval<state_ptr>(), std::declval<int>(),
					std::declval<T&>(), std::declval<Ts>()...)
				);
		};
	};

	template <typename T, typename... ExtraArgs>
	struct is_pullable : std::bool_constant<impl::is_pullable_args<T, std::tuple<ExtraArgs...>>::value> {};

	template <typename T, typename... ExtraArgs>
	constexpr inline auto is_pullable_v = is_pullable<T, ExtraArgs...>::value;

	template <typename T, typename... ExtraArgs>
	using pull_result_t = typename impl::pull_result<T, std::tuple<ExtraArgs...>>::type;


	template <typename T, typename... ExTs>
	concept cx_pushable = is_pushable_v<T, ExTs...>;
	template <typename T, typename... ExTs>
	concept cx_pullable = is_pullable_v<T, ExTs...>;



	template <typename T, typename... ExtraArgs>
	inline auto push(state_ptr _lua, T&& _value, ExtraArgs&&... _extra) ->
		push_result_t<T, ExtraArgs...>
	{
		return stack_traits<std::remove_cvref_t<T>>::push(_lua, std::forward<T>(_value), std::forward<ExtraArgs>(_extra)...);
	};

	template <typename T, typename... ExtraArgs>
	inline auto to(state_ptr _lua, int _index, T& _value, ExtraArgs&&... _extra) ->
		pull_result_t<T, ExtraArgs...>
	{
		return stack_traits<std::remove_cvref_t<T>>::to(_lua, _index, _value, std::forward<ExtraArgs>(_extra)...);
	};

	template <typename T>
	inline auto to(state_ptr _lua, int _index, T& _value) -> pull_result_t<T>
	{
		return stack_traits<std::remove_cvref_t<T>>::to(_lua, _index, _value);
	};

	template <typename T, typename... ExtraArgs>
	requires cx_pullable<T, ExtraArgs...>
	inline T pull(state_ptr _lua, int _index, ExtraArgs&&... _extra)
	{
		auto v = T{};
		to(_lua, _index, v, std::forward<ExtraArgs>(_extra)...);
		return v;
	};






	inline void pop(state* l, int n) { lua_pop(l, n); };
	inline void pop(state* l) { pop(l, 1); };
	
	inline auto abs(state* _lua, int idx) { return ::lua_absindex(_lua, idx); };

	inline int top(state* _lua) { return lua_gettop(_lua); };
	inline void settop(state* _lua, int idx) { lua_settop(_lua, idx); };

	inline void remove(state* _lua, int _index) { lua_remove(_lua, _index); }
	

	inline type type_of(state* l, int idx) { return type(lua_type(l, idx)); };
	inline const char* type_name(state* l, type t) { return lua_typename(l, (int)t); };
	inline const char* type_name_of(state* l, int idx) { return type_name(l, type_of(l, idx)); };

	inline state* newthread(state* l) { return lua_newthread(l); };
	inline status_code resetthread(state* l) { return status_code(lua_resetthread(l)); };
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


	

	inline type getglobal(state* _lua, const char* _name) { return type(lua_getglobal(_lua, _name)); };
	inline void setglobal(state* _lua, const char* _name) { lua_setglobal(_lua, _name); };

	inline type getglobal(state* _lua, const char* _name, size_t _nameLen)
	{
		lua_rawgeti(_lua, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
		push(_lua, _name, _nameLen);
		const auto _type = type(lua_rawget(_lua, -2));
		lua_remove(_lua, -2);
		return _type;
	};
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
	
	inline void setregistry(state_ptr _lua, int _regIndex)
	{
		lua_rawseti(_lua, LUA_REGISTRYINDEX, _regIndex);
	};
	inline type getregistry(state_ptr _lua, int _regIndex)
	{
		return type(lua_rawgeti(_lua, LUA_REGISTRYINDEX, _regIndex));
	};


	template <typename T, typename... ExTs>
	requires cx_pushable<T, ExTs...>
	inline auto pushglobal(state* _lua, std::string_view _name, T&& _value, ExTs&&... _exts)
	{
		push(_lua, std::forward<T>(_value), std::forward<ExTs>(_exts)...);
		setglobal(_lua, _name.data(), _name.size());
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


	namespace impl
	{
		/**
		 * @brief Converts a load mode into a usable string form.
		 * @param _mode Load mode.
		 * @return Non-owning constant string.
		*/
		constexpr const char* load_mode_str(load_mode _mode)
		{
			switch (_mode)
			{
			case load_mode::bt:
				return "bt";
			case load_mode::b:
				return "b";
			case load_mode::t:
				return "t";
			default:
				return nullptr;
			};
		};
	};



	inline status_code load(state_ptr _lua, reader_fn _reader, void* _userdata, const char* _name, load_mode _mode)
	{
		const auto _modeStr = impl::load_mode_str(_mode);
		const auto _status = ::lua_load(_lua, _reader, _userdata, _name, _modeStr);
		return status_code(_status);
	};

	inline status_code load(state* _lua, const char* _str, size_t _strLen, const char* _name, load_mode _mode)
	{
		const auto _modeStr = impl::load_mode_str(_mode);
		const auto _status = luaL_loadbufferx(_lua, _str, _strLen, _name, _modeStr);
		return status_code(_status);
	};
	inline status_code load(state* _lua, const char* _str, size_t _strLen, load_mode _mode)
	{
		return load(_lua, _str, _strLen, nullptr, _mode);
	};
	inline status_code load(state* _lua, const char* _str, size_t _strLen, const char* _name)
	{
		return load(_lua, _str, _strLen, _name, load_mode::bt);
	};
	inline status_code load(state* _lua, const char* _str, size_t _strLen)
	{
		return load(_lua, _str, _strLen, nullptr);
	};

	inline status_code load(state* _lua, std::string_view _str, const char* _name, load_mode _mode)
	{
		return load(_lua, _str.data(), _str.size(), _name, _mode);
	};
	inline status_code load(state* _lua, std::string_view _str, load_mode _mode)
	{
		return load(_lua, _str.data(), _str.size(), _mode);
	};
	inline status_code load(state* _lua, std::string_view _str, const char* _name)
	{
		return load(_lua, _str.data(), _str.size(), _name);
	};
	inline status_code load(state* _lua, std::string_view _str)
	{
		return load(_lua, _str.data(), _str.size());
	};



	status_code loadfile(state* _lua, const char* _path, load_mode _mode);
	inline status_code loadfile(state* _lua, const char* _path)
	{
		return loadfile(_lua, _path, load_mode::bt);
	};




	namespace impl
	{
		template <typename T>
		using nonvoid_write_fn = int(*)(state* _lua, const void* _data, size_t _dataLen, T* _userdata);
	};


	inline int dump(state* _lua, writer_fn _writer, void* _userdata, bool _strip)
	{
		return lua_dump(_lua, _writer, _userdata, _strip);
	};
	template <typename T, impl::nonvoid_write_fn<T> WriteFn>
	inline int dump(state* _lua, T* _userdata, bool _strip)
	{
		constexpr auto _voidWriter = [](state* _lua, const void* _data, size_t _dataLen, void* _userdata) -> int
		{
			return WriteFn(_lua, _data, _dataLen, static_cast<T*>(_userdata));
		};
		return dump(_lua, _voidWriter, _userdata, _strip);
	};

	inline std::vector<std::byte> dump(state* _lua, bool _strip)
	{
		using buffer_type = std::vector<std::byte>;
		auto _buffer = buffer_type();

		constexpr auto writeFn = [](state* _lua, const void* _data, size_t _dataLen, buffer_type* _buffer)
		{
			const auto it = reinterpret_cast<const std::byte*>(_data);
			_buffer->insert(_buffer->end(), it, it + _dataLen);
			return 0;
		};

		dump<buffer_type, writeFn>(_lua, &_buffer, _strip);
		return _buffer;
	};



	inline void xmove(state* _from, state* _to, int _count)
	{
		lua_xmove(_from, _to, _count);
	};
	inline void xmove(state* _from, state* _to)
	{
		return xmove(_from, _to, 1);
	};

	




	inline void* newuserdata(state* _lua, size_t _sizeBytes, int _nUserValues) { return lua_newuserdatauv(_lua, _sizeBytes, _nUserValues); };
	inline void* newuserdata(state* _lua, size_t _sizeBytes) { return lua_newuserdata(_lua, _sizeBytes); };


	template <typename T>
	consteval std::string_view userdata_type_name()
	{
		constexpr auto function_name_v = std::string_view("userdata_type_name");

		auto a = std::string_view(__FUNCSIG__);
		auto p = a.rfind('>');
		a = a.substr(0, p);

		p = a.find(function_name_v);
		p += function_name_v.size();
		a = a.substr(p, a.size() - p);

		p = a.find('<');
		++p;
		a = a.substr(p, a.size() - p);

		return a;
	};


	template <typename T>
	inline T* newuserdata(state* _lua)
	{
		void* _ud = newuserdata(_lua, sizeof(T));

		if constexpr (!std::is_trivially_destructible_v<T>)
		{
			const auto _tname = userdata_type_name<T>();

			auto _tnameStr = std::string(_tname);
			if (luaL_newmetatable(_lua, _tnameStr.c_str()) != 0)
			{
				lua_pushcfunction(_lua, [](state_ptr _lua) -> int
					{
						auto _userdata = static_cast<T*>(lua_touserdata(_lua, 1));
						std::destroy_at(_userdata);
						return 0;
					});
				lua_setfield(_lua, -2, "__gc");
			};
			lua_setmetatable(_lua, -2);
		};
		
		if constexpr (!std::is_trivially_constructible_v<T>)
		{
			return new(_ud) T{};
		}
		else
		{
			return static_cast<T*>(_ud);
		};
	};

};



/*
	Builtin stack traits type specializations
*/

#pragma region STACK_TRAIT_SPECIALIZATIONS
namespace lua
{



	template <>
	struct stack_traits<lua_Integer>
	{
		using type = lua_Integer;
		static void to(state_ptr _lua, int _index, type& _value)
		{
			_value = lua_tointeger(_lua, _index);
		};
		static void push(state_ptr _lua, const type& _value)
		{
			lua_pushinteger(_lua, _value);
		};
	};
	template <typename T>
	struct stack_traits<T, std::enable_if_t<std::is_integral<T>::value>>
	{
		using type = T;
		static void push(state_ptr _lua, const type& _value)
		{
			lua_pushinteger(_lua, static_cast<lua_Integer>(_value));
		};
		static void to(state_ptr _lua, int _index, type& _value)
		{
			lua_Integer _rawValue = lua_tointeger(_lua, _index);
			_value = static_cast<type>(_rawValue);
		};
	};

	template <>
	struct stack_traits<lua_Number>
	{
		using type = lua_Number;
		static void push(state_ptr _lua, const type& _value)
		{
			lua_pushnumber(_lua, _value);
		};
		static void to(state_ptr _lua, int _index, type& _value)
		{
			_value = lua_tonumber(_lua, _index);
		};
	};
	template <typename T>
	struct stack_traits<T, std::enable_if_t<std::is_floating_point<T>::value>>
	{
		using type = T;
		static void push(state_ptr _lua, const type& _value)
		{
			lua_pushnumber(_lua, static_cast<lua_Number>(_value));
		};
		static void to(state_ptr _lua, int _index, type& _value)
		{
			lua_Number _rawValue = lua_tonumber(_lua, _index);
			_value = static_cast<type>(_rawValue);
		};
	};

	template <>
	struct stack_traits<nil_t>
	{
		using type = nil_t;
		static void push(state_ptr _lua, const type& _value)
		{
			lua_pushnil(_lua);
		};
	};
	template <>
	struct stack_traits<bool>
	{
		using type = bool;
		static void to(state_ptr _lua, int _index, type& _value)
		{
			_value = lua_toboolean(_lua, _index);
		};
		static void push(state_ptr _lua, const type& _value)
		{
			lua_pushboolean(_lua, _value);
		};
	};
	template <>
	struct stack_traits<const char*>
	{
		using type = const char*;
		static void to(state_ptr _lua, int _index, type& _value)
		{
			_value = lua_tostring(_lua, _index);
		};
		static auto push(state_ptr _lua, const type& _value)
		{
			return lua_pushstring(_lua, _value);
		};
	};

	/**
	 * @brief Stack traits type for C++ STL string view types.
	 * @tparam Alloc String allocator type.
	*/
	template <>
	struct stack_traits<std::string_view>
	{
		using type = std::string_view;
		static void to(state_ptr _lua, int _index, type& _value)
		{
			size_t _len;
			const char* _str = lua_tolstring(_lua, _index, &_len);
			_value = type(_str, _len);
		};
		static auto push(state_ptr _lua, const type& _value)
		{
			return lua_pushlstring(_lua, _value.data(), _value.size());
		};
	};

	/**
	 * @brief Stack traits type for C++ STL string types.
	 * @tparam Alloc String allocator type.
	*/
	template <typename Alloc>
	struct stack_traits<std::basic_string<char, std::char_traits<char>, Alloc>>
	{
	private:
		using char_type = char;
	public:
		using type = std::basic_string<char_type, std::char_traits<char_type>, Alloc>;
		static void to(state_ptr _lua, int _index, type& _value)
		{
			const auto _str = lua_tostring(_lua, _index);
			_value = _str;
		};
		static const char* push(state_ptr _lua, const type& _value)
		{
			return lua_pushstring(_lua, _value.c_str());
		};
	};

	/**
	 * @brief Stack traits type for lua C functions.
	*/
	template <>
	struct stack_traits<lua_CFunction>
	{
		using type = lua_CFunction;
		static void to(state_ptr _lua, int _index, type& _value)
		{
			_value = lua_tocfunction(_lua, _index);
		};
		static auto push(state_ptr _lua, const type& _value)
		{
			lua_pushcfunction(_lua, _value);
		};
		static auto push(state_ptr _lua, const type& _value, int _upvalues)
		{
			lua_pushcclosure(_lua, _value, _upvalues);
		};
	};

	/**
	 * @brief Stack traits type for the fail value.
	*/
	template <>
	struct stack_traits<fail_t>
	{
		using type = fail_t;
		static void push(state_ptr _lua, const type& _value)
		{
			luaL_pushfail(_lua);
		};
	};

};
#pragma endregion



/*
	Debugging related functionality
*/

namespace lua
{
	/**
	 * @brief Holds debugging information about a lua state.
	 *
	 * Alias of lua_Debug.
	*/
	using debug_info = lua_Debug;

	inline bool getstack(state_ptr _lua, int _level, debug_info& _outInfo)
	{
		// 0 == _level is invalid
		// 1 == success
		const auto _result = lua_getstack(_lua, _level, &_outInfo);
		return _result == 1;
	};

	inline debug_info getstack(state_ptr _lua, int _level)
	{
		auto _info = debug_info{};
		const auto _result = getstack(_lua, _level, _info);
		assert(_result);
		return _info;
	};
	inline debug_info getstack(state_ptr _lua)
	{
		return getstack(_lua, 1);
	};



	enum class info_field : uint16_t
	{
		function_on_stack = 0x00'01, // '>'

		f = 0x00'02, // 'f' - function
		l = 0x00'04, // 'l' - currentline
		n = 0x00'08, // 'n' - name, namewhat
		r = 0x00'10, // 'r' - ftransfer, ntransfer
		S = 0x00'20, // 'S' - source, short_src, linedefined, lastlinedefined
		t = 0x00'40, // 't' - istailcall
		u = 0x00'80, // 'u' - nups, nparams, isvararg
		L = 0x01'00, // 'L' - function_lines

		// 'f'
		function = f,

		// 'l'
		currentline = l,

		// 'n'
		name = n,
		namewhat = n,

		// 'r'
		ftransfer = r,
		ntransfer = r,

		// 'S'
		source = S,
		short_src = S,
		linedefined = S,
		lastlinedefined = S,

		// 't'
		istailcall = t,

		// 'u'
		nups = u,
		nparams = u,
		isvararg = u,

		// 'L'
		function_lines = L,
	};

	constexpr info_field operator&(info_field lhs, info_field rhs)
	{
		using rep = std::underlying_type_t<info_field>;
		return info_field(static_cast<rep>(lhs) & static_cast<rep>(rhs));
	};
	constexpr info_field operator|(info_field lhs, info_field rhs)
	{
		using rep = std::underlying_type_t<info_field>;
		return info_field(static_cast<rep>(lhs) | static_cast<rep>(rhs));
	};
	constexpr info_field operator^(info_field lhs, info_field rhs)
	{
		using rep = std::underlying_type_t<info_field>;
		return info_field(static_cast<rep>(lhs) ^ static_cast<rep>(rhs));
	};

	constexpr info_field& operator&=(info_field& lhs, info_field rhs)
	{
		using rep = std::underlying_type_t<info_field>;
		lhs = lhs & rhs;
		return lhs;
	};
	constexpr info_field& operator|=(info_field& lhs, info_field rhs)
	{
		using rep = std::underlying_type_t<info_field>;
		lhs = lhs | rhs;
		return lhs;
	};
	constexpr info_field& operator^=(info_field& lhs, info_field rhs)
	{
		using rep = std::underlying_type_t<info_field>;
		lhs = lhs ^ rhs;
		return lhs;
	};

	constexpr info_field operator~(info_field rhs)
	{
		using rep = std::underlying_type_t<info_field>;
		return info_field(~static_cast<rep>(rhs));
	};


	namespace impl
	{
		constexpr char info_field_char(info_field _field)
		{
			switch (_field)
			{
			case info_field::function_on_stack:
				return '>';
			case info_field::f:
				return 'f';
			case info_field::L:
				return 'L';
			case info_field::l:
				return 'l';
			case info_field::n:
				return 'n';
			case info_field::r:
				return 'r';
			case info_field::S:
				return 'S';
			case info_field::t:
				return 't';
			case info_field::u:
				return 'u';
			default:
				assert(false);
				return '\0';
			};
		};

		constexpr bool check_field(info_field _field, info_field _checkFor)
		{
			return (_field & _checkFor) == _checkFor;
		};
		
		// Helper for creating the info field string
		struct info_field_writer
		{
		public:
			using value_type = char;
		
		private:
			using buffer_type = std::array<value_type, 16>;
			using iterator = typename buffer_type::iterator;

		public:
			constexpr const value_type* data() const
			{
				return this->buf_.data();
			};
			constexpr auto as_array() const
			{
				return this->buf_;
			};

			constexpr info_field_writer& append(value_type c)
			{
				auto& _it = this->it_;
				const auto& _end = this->buf_.end();

				assert(_it != _end);
				*_it = c;
				std::advance(_it, 1);
				return *this;
			};
			constexpr info_field_writer& append_if(info_field _fields, info_field _hasField)
			{
				if (check_field(_fields, _hasField))
				{
					const auto c = info_field_char(_hasField);
					if (c != '\0')
					{
						this->append(c);
					}
					else
					{
						assert(false);
					};
				};
				return *this;
			};

			constexpr info_field_writer() :
				buf_{ 0 },
				it_(this->buf_.begin())
			{};
			
		private:
			buffer_type buf_;
			iterator it_;
		};

		constexpr auto info_field_string(info_field _fields)
		{
			auto _buffer = info_field_writer();

			_buffer.append_if(_fields, info_field::function_on_stack);
			_buffer.append_if(_fields, info_field::f);
			_buffer.append_if(_fields, info_field::l);
			_buffer.append_if(_fields, info_field::n);
			_buffer.append_if(_fields, info_field::r);
			_buffer.append_if(_fields, info_field::S);
			_buffer.append_if(_fields, info_field::t);
			_buffer.append_if(_fields, info_field::u);
			_buffer.append_if(_fields, info_field::L);

			return _buffer.as_array();
		};
	};


	inline bool getinfo(state_ptr _lua, const char* _what, debug_info& _inOutInfo)
	{
		const auto _result = lua_getinfo(_lua, _what, &_inOutInfo);
		return _result == 1;
	};
	inline debug_info getinfo(state_ptr _lua, const char* _what)
	{
		auto _info = debug_info{};
		const auto _result = getinfo(_lua, _what, _info);
		assert(_result);
		return _info;
	};

	inline bool getinfo(state_ptr _lua, info_field _fields, debug_info& _inOutInfo)
	{
		const auto _fieldStrBuffer = impl::info_field_string(_fields);
		const auto _fieldStr = _fieldStrBuffer.data();
		return getinfo(_lua, _fieldStr, _inOutInfo);
	};
	inline debug_info getinfo(state_ptr _lua, info_field _fields)
	{
		const auto _fieldStrBuffer = impl::info_field_string(_fields);
		const auto _fieldStr = _fieldStrBuffer.data();
		return getinfo(_lua, _fieldStr);
	};


	// Pushes the function at a given level onto the stack
	inline bool getfunction(state_ptr _lua, int _level, debug_info& _inOutInfo)
	{
		if (!getstack(_lua, _level, _inOutInfo))
		{
			return false;
		};
		if (!getinfo(_lua, info_field::function, _inOutInfo))
		{
			return false;
		};
		assert(type_of(_lua, -1) == type::function);
		return true;
	};

	// Pushes the function at a given level onto the stack
	inline bool getfunction(state_ptr _lua, int _level)
	{
		auto _info = debug_info{};
		const auto _result = getfunction(_lua, _level, _info);
		return _result;
	};



};


