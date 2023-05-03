
#pragma once


// Genode includes
#include <base/allocator.h>
#include <base/attached_ram_dataspace.h>
#include <base/attached_rom_dataspace.h>
#include <os/session_policy.h>
#include <root/component.h>

// local includes
#include "terminal_session_component.h"


namespace Controller {

	using namespace Genode;

	class Terminal_root;
}


class Controller::Terminal_root final: public Root_component<Session_component, Single_client>
{
	private:

		Env&                        _env;
		Allocator&                  _alloc;
		Session_component*          _current_session;
		Signal_context_capability   _connected_cap;
		Signal_context_capability   _data_ready_cap;

		// delete some methods to make the compiler happy
		Terminal_root(const Controller::Terminal_root&)            = delete;
		Terminal_root& operator=(const Controller::Terminal_root&) = delete;

	protected:

		Session_component* _create_session(char const*) override
		{
			_current_session = new (_alloc) Session_component { _env, _data_ready_cap };

			Signal_transmitter(_connected_cap).submit();

			return _current_session;
		}

		void _destroy_session(Session_component *session) override
		{
			Genode::destroy(md_alloc(), session);
			_current_session = nullptr;
		}

	public:

		Terminal_root(Env &env, Allocator &alloc,
		              Signal_context_capability connected_cap,
		              Signal_context_capability data_ready_cap) :
			Root_component{&env.ep().rpc_ep(), &alloc},
			_env{env},
			_alloc{alloc},
			_current_session{nullptr},
			_connected_cap{connected_cap},
			_data_ready_cap{data_ready_cap}
		{ }

		Session_component* current_session() const { return _current_session; }
};
