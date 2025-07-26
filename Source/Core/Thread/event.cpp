#include "Core/Thread/event.h"
#include "Core/Container/list.h"
#include "Core/log.h"
#include <Windows.h>

using namespace glex;

static LockFreeList<Event*> s_manualResetList, s_autoResetList;

Event* Event::Get(bool manualReset)
{
	Event* event;
	if ((manualReset ? s_manualResetList : s_autoResetList).Pop(event))
	{
		event->Reset();
		return event;
	}
	return Mem::New<Event>(manualReset);
}

void Event::Release(Event* event)
{
	(event->IsManualReset() ? s_manualResetList : s_autoResetList).Push(event);
}

void Event::Clear()
{
	Event* out;
	while (s_manualResetList.Pop(out))
		Mem::Delete(out);
	while (s_autoResetList.Pop(out))
		Mem::Delete(out);
}

Event::Event(bool manualReset) : m_manualReset(manualReset)
{
	m_handle = reinterpret_cast<uint64_t>(CreateEventW(nullptr, manualReset, FALSE, nullptr));
	if (m_handle == NULL)
		Logger::Fatal("Cannot create event.");
}

Event::~Event()
{
	CloseHandle(reinterpret_cast<HANDLE>(m_handle));
}

void Event::Wait()
{
	WaitForSingleObject(reinterpret_cast<HANDLE>(m_handle), INFINITE);
}

void Event::Set()
{
	SetEvent(reinterpret_cast<HANDLE>(m_handle));
}

void Event::Reset()
{
	ResetEvent(reinterpret_cast<HANDLE>(m_handle));
}