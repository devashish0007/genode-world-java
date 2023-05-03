
#pragma once


// Genode includes
#include <base/lock.h>
#include <base/rpc_server.h>
#include <base/attached_ram_dataspace.h>
#include <os/ring_buffer.h>
#include <terminal_session/terminal_session.h>

#include <gems/magic_ring_buffer.h>

namespace Controller {

	using namespace Genode;

	enum { BUFFER_SIZE = 4096 };

	class Session_component;
}


class Controller::Session_component : public Rpc_object<Terminal::Session, Session_component>
{
	private:

		Env&                        _env;
		Session_capability          _session_cap;
		Attached_ram_dataspace      _io_buffer;
		Signal_context_capability   _parent_read_ready_cap;
		Signal_context_capability   _read_avail_sigh        { };

		Magic_ring_buffer<char>     _read_buffer            { _env, BUFFER_SIZE };
		Magic_ring_buffer<char>     _write_buffer           { _env, BUFFER_SIZE };

	public:

		Session_component(Env& env, Signal_context_capability data_ready_cap) :
			_env{env},
			_session_cap{_env.ep().rpc_ep().manage(this)},
			_io_buffer{env.ram(), env.rm(), BUFFER_SIZE},
			_parent_read_ready_cap{data_ready_cap}
		{ }

		Session_capability cap() { return _session_cap; }

		/********************************
		 ** Terminal session interface **
		 ********************************/

		Size size() override { return Session::Size(0, 0); }
		void size_changed_sigh(Genode::Signal_context_capability) override { }

		Genode::Dataspace_capability _dataspace() { return _io_buffer.cap(); }

		bool avail() override { return _write_buffer.read_avail() > 0; }

		void connected_sigh(Genode::Signal_context_capability sigh) override
		{
			Signal_transmitter(sigh).submit();
		}

		void read_avail_sigh(Genode::Signal_context_capability sigh) override
		{
			_read_avail_sigh = sigh;
			Signal_transmitter(sigh).submit();
		}

		size_t _read(size_t num_bytes)
		{
			const char*   src         { _write_buffer.read_addr() };
			char*         dst         { _io_buffer.local_addr<char>() };
			const size_t  num_to_read { min(_write_buffer.read_avail(), num_bytes) };
			const size_t  num_to_copy { min(num_to_read, _io_buffer.size()) };

			memcpy(dst, src, num_to_copy);

			_write_buffer.drain(num_to_copy);
			return num_to_copy;
		}

		size_t _write(size_t num_bytes)
		{
			const char* src { _io_buffer.local_addr<const char>() };

			// JAVA sends additional '\n'
			if ((num_bytes == 1) && (*src == '\n'))
				return num_bytes;

			size_t num_to_copy { min(_read_buffer.write_avail(), num_bytes) };
			{
				char* dst { _read_buffer.write_addr() };

				memcpy(dst, src, num_to_copy);

				_read_buffer.fill(num_to_copy);
			}

			Signal_transmitter(_parent_read_ready_cap).submit();
			return num_to_copy;
		}

		size_t read(void *buffer, size_t num_bytes) override
		{
			const char* src          { _read_buffer.read_addr() };
			const size_t num_to_copy { min(_read_buffer.read_avail(), num_bytes) };

			memcpy(buffer, src, num_to_copy);

			_read_buffer.drain(num_to_copy);
			return num_to_copy;
		}

		Genode::size_t write(const void* buffer, size_t count) override
		{
			char*   dst         { _write_buffer.write_addr() };
			size_t  num_to_copy { min(_write_buffer.write_avail(), count) };

			memcpy(dst, buffer, num_to_copy);

			_write_buffer.fill(num_to_copy);
			return num_to_copy;
		}
};
