
#include "spark_precompiled.h"

#include "SSM.h"

using namespace spark;

SSM::SSM(const char* name)
	: m_name(name)
{
}

void SSM::PopState() {
	if (m_stack.empty())return;

	Dispatch(Event(ExitEventId));
	m_stack.pop();

	if (m_stack.empty())
	{
		if (!m_emptyStateHandler.empty())
		{
			m_emptyStateHandler();
		}
	}
	else
	{
		Dispatch(Event(ResumeEventId));
	}
}

void SSM::PopStates(int numberStates)
{
	while (!m_stack.empty() && numberStates--)
	{
		Dispatch(Event(ExitEventId));
		m_stack.pop();
	}

	if (m_stack.empty())
	{
		if (!m_emptyStateHandler.empty())
		{
			m_emptyStateHandler();
		}
	}
	else
	{
		Dispatch(Event(ResumeEventId));
	}
}

void SSM::ResizeStack(int size)
{
	while (!m_stack.empty() && m_stack.size() > size)
	{
		Dispatch(Event(ExitEventId));
		m_stack.pop();
		Dispatch(Event(ResumeEventId));
	}
}

void SSM::ChangeState(StateId s)
{
	if (m_stack.empty())return;

	Dispatch(Event(ExitEventId));
	m_stack.pop();
	m_stack.push(s);
	Dispatch(Event(EnterEventId));
}
void SSM::PushState(StateId s) {
	Dispatch(Event(SuspendEventId));
	m_stack.push(s);
	Dispatch(Event(EnterEventId));
}

void SSM::Start(StateId initialState)
{
	PushState(initialState);
}

void SSM::Dispatch(const Event & e)
{
	StateId id = GetCurrentState();
	if (IsValidState(id)) {
		m_states[id].handler(*this,e);
	}
}

void SSM::SetStateHandler(StateId id, const char* name, const StateHandler& handler)
{
	State state;
	state.handler = handler;
	state.name = name;
	m_states[id] = state;
}

void SSM::ClearStateHandler(StateId id)
{
	m_states[id].handler.clear();
	m_states[id].name = NULL;
}


bool SSM::IsInState(StateId id) const {
	return GetCurrentState() == id;
}

void SSM::SetOnEmptyStackHandler(EmptyStackHandler handler)
{
	m_emptyStateHandler = handler;
}


