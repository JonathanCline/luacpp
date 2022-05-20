#include <luacpp.hpp>

#include <array>
#include <fstream>

namespace lua
{
	status_code loadfile(state* _lua, const char* _path, load_mode _mode)
	{
		struct reader_data
		{
		private:
			// TODO : Speedup, iostreams are a little aids
			std::ifstream file_;
			std::array<char, 128> buffer_{};

		public:
			const char* read(size_t& _outCount)
			{
				auto& _file = this->file_;
				auto& _buffer = this->buffer_;

				if (_file)
				{
					_file.read(_buffer.data(), _buffer.size());
					_outCount = _file.gcount();
					return _buffer.data();
				}
				else
				{
					_outCount = 0;
					return nullptr;
				};
			};

			explicit reader_data(const char* _filepath, std::ios::openmode _openmode) :
				file_(_filepath, _openmode)
			{};
			explicit reader_data(const char* _filepath) :
				file_(_filepath)
			{};
		};
	
		// The file reader function.
		constexpr reader_fn _readerFn = [](state_ptr _lua, void* _userdata, size_t* _size) -> const char*
		{
			auto& _data = *static_cast<reader_data*>(_userdata);
			return _data.read(*_size);
		};

		// Make the reader data.
		auto _data = reader_data(_path, std::ios::binary);

		// Load in the file.
		const auto _result = load(_lua, _readerFn, &_data, _path, _mode);
		return _result;
	};
}