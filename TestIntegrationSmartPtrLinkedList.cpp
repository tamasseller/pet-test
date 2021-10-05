/*******************************************************************************
 *
 * Copyright (c) 2021 Tamás Seller. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *******************************************************************************/

#include "1test/Test.h"
#include "1test/Mock.h"

#include "data/LinkedList.h"
#include "managed/RefCnt.h"
#include "managed/Unique.h"

#include "MockAllocator.h"

template<class Uut, class Element, auto add, bool reverse = true>
void sanityCheck(int n)
{
	Uut uut;

	CHECK(uut.isEmpty());

	for(int i = 0; i < n; i++)
	{
		(uut.*add)(Element::make(reverse ? (n - 1 - i) : i));
	}

	CHECK(!uut.isEmpty());

	int idx = 0;
	for(const auto &e: uut)
	{
		CHECK(e->i == idx++);
	}

	CHECK(!uut.isEmpty());
	CHECK(idx == n);
}


template<class Uut, class Element>
auto create(int n)
{
	Uut uut;

	for(int i = 0; i < n; i++)
	{
		uut.addBack(Element::make(i));
	}

	return uut;
}

template<class Uut, class Element>
void clearCheck(int n)
{
	auto uut = create<Uut, Element>(n);

	CHECK(!uut.isEmpty());
	CHECK(uut.begin() != uut.end());
	CHECK(uut.begin() == uut.begin());
	CHECK(uut.end() != uut.begin());
	CHECK(uut.end() == uut.end());

	uut.clear();

	CHECK(uut.begin() == uut.end());
	CHECK(uut.end() == uut.begin());
	CHECK(uut.isEmpty());
}

template<class Uut, class Element>
void removeCheck(int n)
{
	auto uut = create<Uut, Element>(n);

	for(int i = 0; i < n; i++)
	{
		auto r = uut.begin().remove();
		CHECK(r->i == i);
	}

	CHECK(uut.isEmpty());
}

template<class Uut, class Element>
void removeByRefCheck(int n)
{
	auto uut = create<Uut, Element>(n);

	auto nope = uut.remove(Element::make(n + 1));
	CHECK(nope == nullptr);

	int c = 0;
	while(!uut.isEmpty())
	{
		auto r = uut.remove(uut.iterator().current());
		CHECK(r != nullptr && r->i == c++);
	}

	CHECK(c == n);

	auto nope2 = uut.remove(Element::make(n + 2));
	CHECK(nope2 == nullptr);
}

template<class Uut, class Element>
void moveOneByOneCheck(int n)
{
	auto uut = create<Uut, Element>(n);

	decltype(uut) out;

	int c = 0;
	while(!uut.isEmpty())
	{
		out.add(uut.iterator().remove());
		c++;
	}

	CHECK(c == n);

	int idx = n;
	for(const auto &e: out)
	{
		CHECK(e->i == --idx);
	}
	CHECK(idx == 0);
}

TEST_GROUP(RefCntLinkedPtrListIntegration)
{
	struct Element: pet::RefCnt<Element, Allocator>
	{
		const int i;
		Ptr<> next;

		inline Element(int i): i(i) {}
	};

	using Uut = pet::LinkedPtrList<Element::Ptr<>>;
};

TEST(RefCntLinkedPtrListIntegration, Sanity)
{
	sanityCheck<Uut, Element, &Uut::add>(10);
    CHECK(Allocator::allFreed());
}

TEST(RefCntLinkedPtrListIntegration, SanityFastAdd)
{
	sanityCheck<Uut, Element, &Uut::fastAdd>(10);
    CHECK(Allocator::allFreed());
}

TEST(RefCntLinkedPtrListIntegration, SanityAddBack)
{
	sanityCheck<Uut, Element, &Uut::addBack, false>(10);
    CHECK(Allocator::allFreed());
}

TEST(RefCntLinkedPtrListIntegration, Clear)
{
	clearCheck<Uut, Element>(100);
	CHECK(Allocator::allFreed());
}

TEST(RefCntLinkedPtrListIntegration, RemoveByRef)
{
	removeByRefCheck<Uut, Element>(12);
	CHECK(Allocator::allFreed());
}

TEST(RefCntLinkedPtrListIntegration, Remove)
{
	removeCheck<Uut, Element>(12);
	CHECK(Allocator::allFreed());
}

TEST(RefCntLinkedPtrListIntegration, MoveOneByOne)
{
	moveOneByOneCheck<Uut, Element>(123);
	CHECK(Allocator::allFreed());
}

TEST(RefCntLinkedPtrListIntegration, DoubleAddDenied)
{
	{
		auto uut = create<Uut, Element>(123);

		Element::Ptr<> e = uut.iterator().current();
		CHECK(e->i == 0);

		CHECK(!uut.add(e));
		CHECK(!uut.addBack(e));
	}

	CHECK(Allocator::allFreed());
}

TEST(RefCntLinkedPtrListIntegration, ListMoveCtor)
{
	{
		auto x = create<Uut, Element>(3);
		decltype(x) y(pet::move(x));
	}

	CHECK(Allocator::allFreed());
}

TEST_GROUP(UniqueLinkedPtrListIntegration)
{
	struct Element: pet::Unique<Element, Allocator>
	{
		const int i;
		Ptr<> next;

		inline Element(int i): i(i) {}
	};

	using Uut = pet::LinkedPtrList<Element::Ptr<>>;
};

TEST(UniqueLinkedPtrListIntegration, Sanity)
{
	sanityCheck<Uut, Element, &Uut::add>(10);
    CHECK(Allocator::allFreed());
}

TEST(UniqueLinkedPtrListIntegration, SanityFastAdd)
{
	sanityCheck<Uut, Element, &Uut::fastAdd>(10);
    CHECK(Allocator::allFreed());
}

TEST(UniqueLinkedPtrListIntegration, SanityAddBack)
{
	sanityCheck<Uut, Element, &Uut::addBack, false>(10);
    CHECK(Allocator::allFreed());
}

TEST(UniqueLinkedPtrListIntegration, Clear)
{
	clearCheck<Uut, Element>(100);
	CHECK(Allocator::allFreed());
}

TEST(UniqueLinkedPtrListIntegration, Remove)
{
	removeCheck<Uut, Element>(12);
	CHECK(Allocator::allFreed());
}

TEST(UniqueLinkedPtrListIntegration, RemoveByRef)
{
	removeByRefCheck<Uut, Element>(12);
	CHECK(Allocator::allFreed());
}

TEST(UniqueLinkedPtrListIntegration, ListMoveCtor)
{
	{
		auto x = create<Uut, Element>(3);
		decltype(x) y(pet::move(x));
	}

	CHECK(Allocator::allFreed());
}

TEST(UniqueLinkedPtrListIntegration, MoveOneByOne)
{
	moveOneByOneCheck<Uut, Element>(123);
	CHECK(Allocator::allFreed());
}
