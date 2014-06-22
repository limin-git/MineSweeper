// Combination algorithm implementation

// Copyright (C) 2004, BenBear
//
// This file is an algorithm of the combination. This library is free
// software; you can redistribute it and/or modify it under the terms
// of the GNU General Public License as published by the Free Software
// Foundation; either version 2, or (at your option) any later
// version.

// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this library; see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330, Boston,
// MA 02111-1307, USA.

#include <algorithm>
#include <set>

namespace benbear
{

using namespace std;

template <typename BiIterator>
void
sort_combination (BiIterator first, BiIterator last)
{
  if (first == last) // no element
    return;

  BiIterator i = first;
  ++i;
  if (i == last)     // one element
    return;
  
  int half = distance (first, last) / 2;  // half of the length
  BiIterator middle = first;
  advance (middle, half);  // middle += half

  sort_combination (first, middle);     // sort first part
  sort_combination (middle, last);      // sort second part

  inplace_merge (first, middle, last);  // merge two parts
}

template <typename BiIterator>
void
adjust_combination (BiIterator first, BiIterator middle, BiIterator last)
{
  // the front part or the back part have no elements
  if ((first == middle) || (middle == last))
    return;

  sort_combination (first, middle);
  sort_combination (middle, last);

  BiIterator b = middle;
  --b;
  BiIterator j = lower_bound (middle, last, *b);
  reverse (j, last);
  reverse (middle, last);
}

template <typename BiIterator>
void
init_combination (BiIterator first, BiIterator middle, BiIterator last,
    bool min)
{
  sort_combination (first, last);

  if (min == false)
    {
      // the max combination
      reverse (first, last);
      reverse (first, middle);
    }
}

template <typename BiIterator>
bool
next_combination (BiIterator first, BiIterator middle, BiIterator last)
{
  if ((first == middle) || (middle == last))
    return false;

  // last element of [first, middle)
  BiIterator b = middle;
  --b;

  if (*b < *middle)
    {
      BiIterator j = b;
      while ((++b != last) && (*j < *b))
	 {
	   iter_swap (j, b);
	   j = b;
	 }
      return true;
    }

  BiIterator e = last;
  --e;
  while (e != middle)
    {
      BiIterator k = e;
      --k;
      if (!(*k < *e))
 e = k;
      else
 break;
    }
  
  BiIterator f = e;
  ++f;
  while ((f != last) && !(*f < *e))
    ++f;

  if (!(*first < *e))
    {
      reverse (first, middle);
      reverse (first, last);
      return false;
    }

  if (*b < *e)
    {
      BiIterator bb = b;
      while ((++bb != e) && !(*b < *bb))
 ;
      reverse (bb, f);
      reverse (b, f);
    }
  else
    {
      BiIterator i = b;
      while (!(*--i < *e))
 ;
      
      BiIterator j = last;
      while (!(*i < *--j))
 ;

      iter_swap (i, j);
      reverse (++i, middle);
      reverse (i, j);
    }
  return true;
}

template <typename BiIterator>
bool
prev_combination (BiIterator first, BiIterator middle, BiIterator last)
{
  if ((first == middle) || (middle == last))
    return false;
  
  BiIterator b = middle;
  --b;
  
  if (*middle < *b)
    {
      BiIterator i = upper_bound (first, middle, *middle);
      if (i != b)
 iter_swap (i, middle);
      else
 {
   BiIterator s = middle;
   while ((++s != last) && !(*s < *middle))
     ;
   reverse (b, s);
 }

      return true;
    }
  
  BiIterator e = last;
  --e;
  while (e != middle)
    {
      BiIterator k = e;
      --k;
      if (!(*k < *e))
 e = k;
      else
 break;
    }
  
  BiIterator f = e;
  ++f;
  while ((f != last) && !(*f < *e))
    ++f;

  if (f == last)
    {
      reverse (first, last);
      reverse (first, middle);
      return false;
    }

  BiIterator i = upper_bound (first, middle, *f);
  if (i == b)
    {
      BiIterator s = f;
      while ((++s != last) && !(*s < *f))
 ;

      reverse (b, f);
      reverse (b, s);
    }
  else
    {
      iter_swap (i, f);
      reverse (++i, f);
      reverse (i, middle);
    }
  return true;
}

} // end of namespace benbear


// for test:
#include <iostream>
#include <cstdlib>
#include <vector>
using namespace std;

template <typename BiIterator>
void
show_array (BiIterator f, BiIterator m, BiIterator l)
{
  while (f != m)
    cout << *f++;
  cout << " ";
  while (f != l)
    cout << *f++;
  cout << endl;
}

template <class T>
void combi (vector< T >& vec, int n, vector< T >& des)
{
try {
	do {
		des.insert(des.end(), vec.begin(), vec.begin()+n);
	} while (benbear::next_combination (vec.begin(), vec.begin()+n, vec.end()));
} catch(...) { cout << "combi" << endl; }
}