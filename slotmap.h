// Oleg Kotov

#pragma once

#include <stdint.h>
#include <array>
#include <assert.h>

class Handle
{
public:

	Handle( uint32_t slotIndex, uint32_t serialNumber ) 
		: m_slotIndex( slotIndex )
		, m_serialNumber( serialNumber ) 
	{}

	uint32_t getSlotIndex() const
	{
		return m_slotIndex;
	}

	uint32_t getSerialNumber() const
	{
		return m_serialNumber;
	}

private:

	uint32_t m_slotIndex;
	uint32_t m_serialNumber;
};

// fixed size realization
template <class UserType, size_t _capacity>
class SlotMap
{
	using index_type = uint32_t;
	using value_type = UserType;
	using iterator = typename std::array<value_type, _capacity>::iterator;
	using const_iterator = typename std::array<value_type, _capacity>::const_iterator;

	struct Slot
	{
		union
		{
			index_type nextFreeSlot; // if slot is free
			index_type dataIndex;    // if slot is not free - index in slotmap m_data[] array
		};

		uint32_t serialNumber;
	};

public:

	SlotMap()
	{
		m_nextSerialNumber = 0;
		m_count = 0;

		clear();
	}

	Handle add( UserType&& item )
	{
		index_type slotIndex = initFreeSlot();
		const Slot& slot = m_slots[slotIndex];

		m_data[slot.dataIndex] = item;
		m_erase[slot.dataIndex] = slotIndex;

		return Handle( slotIndex, slot.serialNumber );
	}

	Handle add( const UserType& item )
	{
		return add( UserType{ item } );
	}

	void remove( const Handle& handle )
	{
		assert( m_count > 0 );
		assert( contains( handle ) );
		freeSlot( handle );
	}

	bool contains( const Handle& handle ) const
	{
		index_type slotIndex = handle.getSlotIndex();

		bool isIndexOutOfRange = slotIndex >= _capacity;
		bool isSerialNumberMismatch = m_slots[slotIndex].serialNumber != handle.getSerialNumber();

		if ( isIndexOutOfRange || isSerialNumberMismatch ) return false;

		return true;
	}

	UserType get( const Handle& handle ) const
	{
		assert( contains( handle ) );

		index_type slotIndex = handle.getSlotIndex();
		const Slot& slot = m_slots[slotIndex];

		return m_data[slot.dataIndex];
	}

	UserType operator[]( const Handle& handle )
	{
		return get( handle );
	}
	
	const UserType operator[]( const Handle& handle ) const
	{
		return get( handle );
	}

	size_t capacity() const
	{
		return _capacity;
	}

	size_t count() const
	{
		return m_count;
	}

	bool empty() const
	{
		return m_count == 0;
	}

	void clear()
	{
		initFreeList();
		m_count = 0;
	}

	// Iterators ----------------------------------------------

	iterator begin() { return m_data.begin(); }
	iterator end() { return m_data.begin() + m_count; }

	const_iterator begin() const { return m_data.begin(); }
	const_iterator end() const { return m_data.begin() + m_count; }

	const_iterator cbegin() const { return m_data.cbegin(); }
	const_iterator cend() const { return m_data.cbegin() + m_count; }

private:

	void initFreeList()
	{
		for ( index_type i = 0; i < _capacity; ++i )
		{
			m_slots[i].nextFreeSlot = i + 1;
		}

		m_firstFreeSlot = 0;
	}

	index_type initFreeSlot()
	{
		assert( m_count < _capacity && "no space left in the slotmap\n" );

		index_type slotIndex = m_firstFreeSlot;
		m_firstFreeSlot = m_slots[slotIndex].nextFreeSlot;

		Slot& slot = m_slots[slotIndex];
		slot.dataIndex = m_count;
		slot.serialNumber = m_nextSerialNumber;

		++m_count;
		++m_nextSerialNumber;

		return slotIndex;
	}

	void freeSlot( const Handle& handle )
	{
		index_type slotIndex = handle.getSlotIndex();

		Slot& slot = m_slots[slotIndex];
		index_type dataIndex = slot.dataIndex;

		slot.nextFreeSlot = m_firstFreeSlot;
		m_firstFreeSlot = slotIndex;

		slot.serialNumber = m_nextSerialNumber;

		// lastDataIndex
		index_type lastIndex = m_count - 1;

		if ( dataIndex != lastIndex )
		{
			m_data[dataIndex] = m_data[lastIndex];
			m_erase[dataIndex] = m_erase[lastIndex];

			m_slots[m_erase[dataIndex]].dataIndex = dataIndex;
		}

		--m_count;
		++m_nextSerialNumber;
	}

private:

	index_type m_firstFreeSlot;
	index_type m_nextSerialNumber;
	size_t m_count;

	std::array<Slot, _capacity> m_slots;
	std::array<UserType, _capacity> m_data;
	std::array<index_type, _capacity> m_erase; // data to slot index
};

