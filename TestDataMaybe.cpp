/*******************************************************************************
 *
 * Copyright (c) 2016, 2017 Tamás Seller. All rights reserved.
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

#include "data/Maybe.h"

using namespace pet;

TEST_GROUP(Maybe) {};

TEST(Maybe, Sanity)
{
	Maybe m = 123;
	CHECK((bool)m && *m == 123);

	decltype(m) n;
	CHECK(!n);

	n = m;
	CHECK((bool)n && *n == 123);

	m = 234;
	CHECK((bool)n && *n == 123);
	CHECK((bool)m && *m == 234);

	auto o(m);
	CHECK((bool)o && *o == 234);
}

TEST(Maybe, Movable)
{
	struct Movable
	{
		Movable() {}

		Movable(const Movable&) = delete;
		Movable& operator =(const Movable&) = delete;

		Movable(Movable&&) = default;
		Movable& operator =(Movable&&) = default;
	};

	Maybe<Movable> m(nullptr);
	CHECK((bool)m);

	decltype(m) n;
	CHECK(!n);

	n = pet::move(m);
	CHECK(!!n);
}

TEST(Maybe, Immobile)
{
	struct Immobile
	{
		Immobile() {}

		Immobile(const Immobile&) = delete;
		Immobile& operator =(const Immobile&) = delete;

		Immobile(Immobile&&) = delete;
		Immobile& operator =(Immobile&&) = delete;
	};

	Maybe<Immobile> m(nullptr);
	CHECK((bool)m);

	decltype(m) n;
	CHECK(!n);
}
