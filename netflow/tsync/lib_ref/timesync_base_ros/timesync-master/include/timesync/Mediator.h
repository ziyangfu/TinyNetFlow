// Sliding median filter
// Created 2012 by Colin Raffel
// Portions Copyright (c) 2011 ashelly.myopenid.com under <http://www.opensource.org/licenses/mit-license>


#ifndef MEDIATOR_H
#define MEDIATOR_H

template <typename Item> class Mediator
{
public:
  
  Mediator(int nItems ):N(nItems)
  {
    data = new Item[nItems];
    pos = new int[nItems];
    allocatedHeap = new int[nItems]; 
    heap = allocatedHeap + (nItems/2);
    minCt = maxCt = idx = 0;
    // Set up initial heap fill pattern: median, max, min, max, ...
    while (nItems--)
    {  
      pos[nItems] = ((nItems + 1)/2)*((nItems & 1) ? -1 : 1);
      heap[pos[nItems]] = nItems;
    }
  }
  
  ~Mediator()
  {
    delete[] data;
    delete[] pos;
    delete[] allocatedHeap;
  };
  
  // Inserts item, maintains median in O(lg nItems)
  void insert(const Item& v )
  {
    const int p = pos[idx];
    const Item old = data[idx];
    data[idx] = v;
    idx = (idx+1) % N;
    // New item is in minheap
    if (p>0)
    {  
      if (minCt < (N-1)/2)
      {
        minCt++;
      }
      else if (v > old)
      { 
        minSortDown( p );
        return;
      }
      if (minSortUp( p ) && mmCmpExch( 0, -1 ))
      {
        maxSortDown( -1 );
      }
    }
    // New item is in maxheap
    else if (p<0)
    {  
      if (maxCt < N/2)
      {
        maxCt++;
      }
      else if (v < old)
      {
        maxSortDown( p );
        return;
      }
      if (maxSortUp( p ) && minCt && mmCmpExch( 1, 0 ))
      {
        minSortDown( 1 );
      }
    }
    // New item is at median
    else
    {  
      if (maxCt && maxSortUp( -1 ))
      {
        maxSortDown( -1 );
      }
      if (minCt && minSortUp( 1 ))
      { 
        minSortDown( 1 );
      }
    }
  };
  
  // Returns median item (or average of 2 when item count is even)
  Item getMedian()
  {
    Item v = data[heap[0]];
    if (minCt<maxCt)
    { 
      v = (v + data[heap[-1]])/2;
    }
    return v;
  };
  
private:
  
  // Swaps items i&j in heap, maintains indexes
  int mmexchange(const int i,const int j )
  {
    int t = heap[i];
    heap[i] = heap[j];
    heap[j] = t;
    pos[heap[i]] = i;
    pos[heap[j]] = j;
    return 1;
  };
  
  // Maintains minheap property for all items below i.
  void minSortDown( int i )
  {
    for (i*=2; i <= minCt; i*=2)
    {
      if (i < minCt && mmless( i+1, i ))
      {
        ++i;
      }
      if (!mmCmpExch( i, i/2 ))
      {
        break;
      }
    }
  };

  // Maintains maxheap property for all items below i. (negative indexes)
  void maxSortDown( int i )
  {
    for (i*=2; i >= -maxCt; i*=2)
    {  
      if (i > -maxCt && mmless( i, i-1 )) 
      { 
        --i;
      }
      if (!mmCmpExch( i/2, i ))
      {
        break;
      }
    }
  };
  
  // Returns 1 if heap[i] < heap[j]
  inline int mmless(const int i,const int j )
  {
    return (data[heap[i]] < data[heap[j]]);
  };
  
  // Swaps items i&j if i<j; returns true if swapped
  inline int mmCmpExch(const int i,const int j )
  {
    return (mmless( i, j ) && mmexchange( i, j ));
  };
  
  // Maintains minheap property for all items above i, including median
  // Returns true if median changed
  inline int minSortUp( int i )
  {
    while (i > 0 && mmCmpExch( i, i/2 ))
    {
      i /= 2;
    }
    return (i == 0);
  };
  
  // Maintains maxheap property for all items above i, including median
  // Returns true if median changed
  inline int maxSortUp( int i )
  {
    while (i < 0 && mmCmpExch( i/2, i ))
    {
      i /= 2;
    }
    return ( i==0 );
  };
  // Allocated size
  const int N;
  // Circular queue of values
  Item* data;
  // Index into `heap` for each value
  int* pos;
  // Max/median/min heap holding indexes into `data`.
  int* heap;
  // heap holds a pointer to the middle of its data; this is where the data is allocated.
  int* allocatedHeap;
  // Position in circular queue
  int idx; 
  // Count of items in min heap
  int minCt;
  // Count of items in max heap
  int maxCt;
};

#endif
