#pragma once

#include "Types.hpp"
#include <memory>
//#include <wrl/client.h>

struct IUnknown;

namespace Luden
{
	template<typename T>
	class Ref
	{
	public:
		Ref()
			: m_Ptr(nullptr)
		{
		}
		Ref(std::nullptr_t)
			: m_Ptr(nullptr)
		{
		}
		Ref(T* Other)
			: m_Ptr(Other)
		{
			InternalAddRef();
		}
		Ref(Ref&& Other)
			: m_Ptr(std::exchange(Other.m_Ptr, nullptr))
		{
			InternalAddRef();
		}
		Ref(const Ref& Other)
			: m_Ptr(Other.m_Ptr)
		{
			InternalAddRef();
		}
		~Ref()
		{
			//Release();
			InternalRelease();
		}

		uint32 Release()
		{
			uint32 prevCount = m_RefCount.fetch_sub(1);

			if (prevCount == 1)
			{
				delete m_Ptr;
				m_RefCount = 0;
			}
		
			return prevCount;
		}

		uint32 Reset()
		{
			return InternalRelease();
		}


		T* Get() const
		{
			return m_Ptr;
		}

		T** GetAddressOf()
		{
			return &m_Ptr;
		}

		T* const* GetAddressOf() const
		{
			return &m_Ptr;
		}

		T** ReleaseAndGetAddressOf() const
		{
			InternalRelease();

			return &m_Ptr;
		}

		template<typename K>
		void As(Ref<K>& pOther)
		{
			static_assert(std::is_base_of_v<IUnknown, K> && std::is_base_of_v<IUnknown, T>, "Invalid base type!");

			//m_Ptr->QueryInterface(IID_PPV_ARGS(&pOther));
			pOther = static_cast<K*>(m_Ptr);
		}

		operator bool() const
		{
			return m_Ptr != nullptr;
		}

		T* operator->() const
		{
			return m_Ptr;
		}

		operator T*() const
		{
			return m_Ptr;
		}

		T* operator*() const
		{
			return m_Ptr;
		}

		T** operator&()
		{
			return std::addressof(m_Ptr);
		}

		T& operator=(std::nullptr_t) noexcept
		{
			InternalRelease();

			m_Ptr = nullptr;

			return *m_Ptr;
		}

		Ref& operator=(const Ref& Other) noexcept
		{
			if (m_Ptr != Other.m_Ptr)
			{
				Ref(Other).Swap(*this);
			}

			return *this;
		}


		void Swap(Ref&& Other) noexcept
		{
			std::swap(m_Ptr, Other.m_Ptr);
		}

		void Swap(Ref& Other) noexcept
		{
			std::swap(m_Ptr, Other.m_Ptr);
		}

		uint32 AddRef()
		{
			return m_RefCount.fetch_add(1);
		}


	protected:
		void InternalAddRef()
		{
			if (m_Ptr)
			{
				m_Ptr->AddRef();
			}
		}

		uint32 InternalRelease()
		{
			uint32 refCount = 0;

			T* temp = m_Ptr;

			if (temp != nullptr)
			{
				m_Ptr = nullptr;
				refCount = temp->Release();
			}

			return refCount;
		}

	private:
		T* m_Ptr = nullptr;
		std::atomic<uint32> m_RefCount = 0;

	};
} // namespace Luden
