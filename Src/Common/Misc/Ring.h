#pragma once

#include <cassert>
// This is the typical ring buffer, it is used by resources that will be reused.
// For example, commandlists and 'dynamic' constant buffers, etc..
class Ring
{
public:
	void Create(uint32_t totalSize)
	{
		m_head = 0;
		m_allocatedSize = 0;
		m_totalSize = totalSize;
	}

	uint32_t	GetSize() { return m_allocatedSize; }
	uint32_t	GetHead() { return m_head; }
	uint32_t	GetTail() { return (m_head + m_allocatedSize) % m_totalSize; }

	// Helper to avoid allocating chunk that wouldn't fit contiguously in the ring.
	uint32_t PaddingToAvoidCrossOver(uint32_t size)
	{
		int tail = GetTail();
		if (( tail + size ) > m_totalSize )
			return ( m_totalSize - tail );
		else
			return 0;
	}

	bool Alloc( uint32_t size, uint32_t *pOut )
	{
		if (m_allocatedSize + size <= m_totalSize )
		{
			if( pOut )
				*pOut = GetTail();

			m_allocatedSize += size;
			return true;
		}

		assert( false );
		return false;
	}

	bool Free( uint32_t size )
	{
		if( m_allocatedSize >= size )
		{
			m_head = ( m_head + size ) % m_totalSize;
			m_allocatedSize -= size;
			return true;
		}

		return false;
	}

private:
	uint32_t	m_head;
	uint32_t	m_allocatedSize;
	uint32_t	m_totalSize;
};

// 
// This class can be thought as ring buffer inside a ring buffer. The outer ring is for , 
// the frames and the internal one is for the resources that were allocated for that frame.
// The size of the outer ring is typically the number of back buffers.
//
// When the outer ring is full, for the next allocation it automatically frees the entries 
// of the oldest frame and makes those entries available for the next frame. This happens 
// when you call 'OnBeginFrame()' 
//
class RingWithTabs
{
public:
	void OnCreate( uint32_t numberOfbackBuffers, uint32_t memTotalSize )
	{
		m_backBufferIndex		= 0;
		m_numberOfBackBuffers	= numberOfbackBuffers;

		// Init mem per frame tracker.
		m_memAllocatedInFrame = 0;
		for( int i=0; i < 4; ++i )
			m_allocatedMemPerBackBuffer[ i ] = 0;

		m_mem.Create( memTotalSize );
	}

	void OnDestroy( uint32_t numberOfbackBuffers, uint32_t memTotalSize )
	{
		m_mem.Free( m_mem.GetSize() );
	}

	bool Alloc( uint32_t size, uint32_t *pOut )
	{
		uint32_t padding = m_mem.PaddingToAvoidCrossOver( size );
		if( padding > 0 )
		{
			m_memAllocatedInFrame += padding;

			if( m_mem.Alloc( padding, NULL ) == false ) // alloc chunk to avoid crossover, ignore offset.
			{
				return false;							// no mem, cannot allocate padding.
			}
		}

		if ( m_mem.Alloc( size, pOut ) == true)
		{
			m_memAllocatedInFrame += size;
			return true;
		}

		return false;
	}

	void OnBeginFrame()
	{
		m_allocatedMemPerBackBuffer[ m_backBufferIndex ] = m_memAllocatedInFrame;
		m_memAllocatedInFrame = 0;

		m_backBufferIndex = ( m_backBufferIndex + 1 ) % m_numberOfBackBuffers;

		// free all the entries for the oldest buffer in one go.
		uint32_t memToFace = m_allocatedMemPerBackBuffer[ m_backBufferIndex ];
		m_mem.Free( memToFace );
	}

private:
	// Internal ring Buffer.
	Ring m_mem;

	// This is the external ring buffer.
	uint32_t	m_backBufferIndex;
	uint32_t	m_numberOfBackBuffers;

	uint32_t	m_memAllocatedInFrame;
	uint32_t	m_allocatedMemPerBackBuffer[4];
};