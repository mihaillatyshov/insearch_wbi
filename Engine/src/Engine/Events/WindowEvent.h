#pragma once

#include "Event.h"

namespace LM
{

	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(uint32_t _Width, uint32_t _Height) : m_Width(_Width), m_Height(_Height) { }

		inline uint32_t GetWidth() const { return m_Width; }
		inline uint32_t GetHeight() const { return m_Height; }

		std::string ToString() const override
		{
			return "WindowResizeEvent: " + std::to_string(m_Width) + " : " + std::to_string(m_Height);
		}

		EVENT_CLASS_TYPE(WindowResize)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	protected:
		uint32_t m_Width;
		uint32_t m_Height;
	};

	class WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() = default;

		EVENT_CLASS_TYPE(WindowClose)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

}