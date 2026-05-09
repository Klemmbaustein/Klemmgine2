#pragma once
#include <functional>
#include <type_traits>

namespace engine
{
	template <typename T, typename fn>
	struct Closeable
	{
	public:
		T Value;
		std::function<void(T)> Close;
		bool Valid = true;

		Closeable(T val, fn Close)
		{
			this->Value = val;
			this->Close = Close;
		}

		Closeable(const Closeable&) = delete;

		~Closeable()
		{
			if (Value && Valid)
			{
				Close(Value);
			}
		}

		void Invalidate()
		{
			Valid = false;
		}

		T* operator->()
		{
			return &Value;
		}
		const T* operator->() const
		{
			return &Value;
		}
		T& operator*()
		{
			return Value;
		}
		const T& operator*() const
		{
			return Value;
		}

		operator T()
		{
			return Value;
		}
	};

	template<typename T>
	struct ClosablePtr
	{
	public:
		T Value;
		bool Valid = true;

		ClosablePtr(T val)
		{
			this->Value = val;
		}

		ClosablePtr(const ClosablePtr&) = delete;

		~ClosablePtr()
		{
			if (Value && Valid)
			{
				delete Value;
			}
		}

		void Invalidate()
		{
			Valid = false;
		}

		T* operator->()
		{
			return &Value;
		}
		const T* operator->() const
		{
			return &Value;
		}
		T& operator*()
		{
			return Value;
		}
		const T& operator*() const
		{
			return Value;
		}

		operator T()
		{
			return Value;
		}

	};
}