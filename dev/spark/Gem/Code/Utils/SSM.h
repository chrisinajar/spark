#pragma once


#include <AzCore/std/containers/stack.h>
#include <AzCore/base.h>
#include <AzCore/std/containers/array.h>
#include <AzCore/std/delegate/delegate.h>


#define SSM_STATE_NAME(_STATE) _STATE,#_STATE

namespace spark {

	class SSM
	{
	public:
		typedef unsigned char StateId;
		enum ReservedEventIds
		{
			EnterEventId = -1,
			ExitEventId = -2,
			SuspendEventId   = -3,
			ResumeEventId = -4,

			OnEnter = -1,
			OnExit  = -2,
			OnSuspend = -3,
			OnResume  = -4
		};
		static const unsigned char MaxNumberOfStates = 254;
		static const unsigned char InvalidStateId = 255;
		static const int InvalidEventId = -8;

		struct Event
		{
			Event()
				: id(InvalidEventId)
				, userData(NULL) {}
			Event(int ID)
				: id(ID)
				, userData(NULL) {}
			template<typename T>
			Event(int ID,const T &data): id(ID), userData((void*)&data) {}

			int     id;
			void*   userData;

			template<typename T>
			T GetData() const { return *(T*)userData; }
		};
		typedef AZStd::delegate<bool(SSM& sm, const Event& e)> StateHandler;
		typedef AZStd::delegate<void()>	EmptyStackHandler;

		SSM(const char* name = "SSM no name");

		const char*     GetName() const { return m_name; }

		/// Starts the state machine.
		void            Start(StateId initialState);
		/// Dispatches an event
		void            Dispatch(const Event& e);
		void            PushState(StateId s);
		void			ChangeState(StateId s);
		void			PopState();
		void			PopStates(int numberStates=9999);
		void			ResizeStack(int size = 1);

		StateId         GetCurrentState() const { return m_stack.empty()?InvalidStateId:m_stack.top(); }

		/**
		 * Set a handler for a state ID. This function will overwrite the current state handler.
		 * \param id state id from 0 to MaxNumberOfStates
		 * \param name state debug name.
		 * \param handler delegate to the state function.
		 */
		void            SetStateHandler(StateId id, const char* name, const StateHandler& handler);
		/// Resets the state to invalid mode.
		void            ClearStateHandler(StateId id);
		/// @{ Event handing
		const char*     GetStateName(StateId id) const { return m_states[id].name; }
		bool            IsValidState(StateId id) const { return id>=0 && id<MaxNumberOfStates && !m_states[id].handler.empty(); }
		bool            IsInState(StateId id) const;
		/// @}
		void			SetOnEmptyStackHandler(EmptyStackHandler handler);

	protected:
		const char*     m_name;

		AZStd::stack<StateId> m_stack;
		struct State
		{
			State()
				: name(NULL) {}
			StateHandler handler;
			const char*  name;
		};
		AZStd::array<State, MaxNumberOfStates>   m_states;
		EmptyStackHandler m_emptyStateHandler;
	};

}
