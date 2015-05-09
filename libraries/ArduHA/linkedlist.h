/*
  ArduHA - ArduixPL - xPL library for Arduino(tm)
  Copyright (c) 2012/2014 Mathieu GRENET.  All right reserved.

  This file is part of ArduHA / ArduixPL.

    ArduixPL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    ArduixPL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ArduixPL.  If not, see <http://www.gnu.org/licenses/>.

	  Modified 2014-3-23 by Mathieu GRENET 
	  mailto:mathieu@mgth.fr
	  http://www.mgth.fr
*/

#ifndef LINKEDLIST_H
#define LINKEDLIST_H
#include <ArduHA.h>

/// <summary>macro to iterate pointers of the linked list</summary>
//#define foreachlnkfrom(cls,lnk,from) for (cls*volatile* lnk = from; *lnk; lnk = &(*lnk)->LinkedList<cls>::_next)
#define foreachlnk(cls,lnk,from) for (cls** lnk = &from; *lnk; lnk = &(*lnk)->LinkedObject<cls>::next())
//#define foreachlnk(cls,lnk) foreachlnkfrom(cls,lnk,&LinkedList<cls>::first())

/// <summary>macro to iterate all members of a list</summary>
//#define foreachfrom(cls,obj,from) for(cls* obj=from;obj;obj=obj->LinkedList<cls>::next())
#define foreach(cls,obj,from) for(cls* obj=from;obj;obj=obj->LinkedObject<cls>::next())

//#define foreach(cls,obj) foreachfrom(cls,obj,LinkedList<cls>::first())


/// <summary>get sign of a signed value</summary>
/// <returns>-1, 0 or 1</returns>
template <typename T> int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}


/// <summary>Linked list template</summary>
/// This is not a full implementation like vertex, with iterators and so, and it can only be used with objects that inherits it.
/// Another drawback of not using iterators is that an object can only be linked once.
/// But it has great advantages :
/// Objects can get automatically linked at construction, with nothing more than declaration.
/// <remarks>Note the special syntax : the template argument have to be the child class itself.</remarks>
template<class cls>
class LinkedObject
{
protected:
	cls* _next;

public:
	/// <summary>reference to the next slot</summary>
	cls*&  next() { return _next; }
	operator cls*&() { return _next; }

	/// <summary>last element or null if list is empty</summary>
	cls*& last()
	{
		cls** c = &_next;
		while (*c != NULL) c = &(*c)->_next;
		return *c;
	}

	/// <summary>add this element to list</summary>
	/// <param name="lnk">reference to a link where to insert element</param>
	/// <remarks>
	/// pass <c>previous.next()</c> or <c>cls::first()</c>
	/// </remarks>
	void link(cls*& lnk)
	{
			_next = lnk;
			lnk = (cls*)this;
	}

	/// <summary>remove an element from the list</summary>
	/// <param name="fst">reference to head of the list, default : <c>firstRef()</c></param>
	void unlink(cls*& fst)
	{
			foreachlnk(cls, lnk, fst)
			{
				if (*lnk == this)
				{
					*lnk = _next;
					break;
				}
			}
			_next = NULL;
	}

	/// <summary>see if object is already linked</summary>
	bool linked(cls*& fst) {
			foreach(cls, lnk, fst)
			{
				if (lnk == this)
				{
					return true;
				}
			}
			return false;
	}

	/// <summary>move element at his position</summary>
	/// <remarks>
	/// child object must implement <c>compare()</>
	/// </remarks>
	void relocate(cls*& fst)
	{
			((cls*)this)->unlink(fst);

				cls** lnk = &fst;

				while (*lnk)
				{
					if (((cls*)this)->compare(**lnk) < 0) break;
					lnk = &( (*lnk)->next() );
				}
				((cls*)this)->link(*lnk);
	}

	/// <summary>item creation with no argument, unlinked by default</summary>
	/// <remarks>unlinked element point to itsef</remarks>
	LinkedObject() {
			_next = (cls*)this;
	}

	/// <summary>linked item creation</summary>
	/// <param name="lnk">element where to link</param>
	LinkedObject(cls*& lnk) { link(lnk); }
};


template<class cls>
class LinkedList
{
protected:
	/// <summary>head of the list</summary>
	/// <remarq>should not be set to null at construction time because element could be already linked</remarq>
	cls* _first;

public:
	/// <summary>get first element of the list</summary>
	cls*& first() { return _first; }

	/// <summaryso that you can pass the list itself when pointer to element is requered</summary>
	operator cls*&() { return _first; }

	/// <summary>last element or null if list is empty</summary>
	cls*& last() {
		cls** c = &_first;
		while (*c != NULL) c = &(*c)->next();
		return *c;
	}

	/// <summary>count elements to end</summary>
	/// <param name="fst">element to start count from</param>
	int count()
	{
		int cnt = 0;
		//for (cls* c = _first; c; c = c->next())
		foreach(cls, obj, *this)
		{ cnt++; }
		return cnt;
	}

};

/// <summary>new elements will be linked at construction</summary>
template<class cls>
class AutoList : public LinkedObject<cls>
{
protected:
	static LinkedList<cls> _list;

public:
	static LinkedList<cls>& List() { return _list; }

	AutoList() : LinkedObject<cls>(_list.first()) { }
};

template<class cls>
LinkedList<cls> AutoList<cls>::_list;

#endif