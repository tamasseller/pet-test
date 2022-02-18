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

#include "data/LinkedList.h"
#include "1test/Test.h"

using namespace pet;

struct TargetLike;

class PtrLike
{
    TargetLike* wrapped = nullptr;

    friend TargetLike;
    inline PtrLike(TargetLike* wrapped): wrapped(wrapped) {}
public:
    inline PtrLike () = default;
    PtrLike(const PtrLike&) = delete;

    inline PtrLike (nullptr_t) {}
    inline PtrLike (PtrLike&& o): wrapped(o.wrapped) {
        o.wrapped = nullptr;
    }

    inline PtrLike &operator =(nullptr_t)
    {
        wrapped = nullptr;
        return *this;
    }

    inline PtrLike &operator =(PtrLike&& o)
    {
        wrapped = o.wrapped;
        o.wrapped = nullptr;
        return *this;
    }

    inline TargetLike* operator->() {
        return wrapped;
    }

    inline TargetLike* testAccess() const {
        return wrapped;
    }

    inline bool operator==(const PtrLike& o) const volatile{
        return wrapped == o.wrapped;
    }

    inline bool operator==(nullptr_t) const volatile{
        return !wrapped;
    }
};

struct TargetLike
{
    PtrLike next;

    inline PtrLike getPtr() {
        return this;
    }
};

TEST_GROUP(LinkedListSpecial) {};

TEST(LinkedListSpecial, Sanity)
{
    TargetLike t1, t2;

    LinkedPtrList<PtrLike> uut;

    CHECK(uut.remove(t2.getPtr()) == nullptr);

    CHECK(uut.isEmpty());
    CHECK(uut.add(t1.getPtr()));
    CHECK(!uut.isEmpty());

    int n = 0;
    for(const auto &f: uut)
    {
        CHECK(f.testAccess() == &t1);
        n++;
    }
    CHECK(n == 1);

    CHECK(!uut.add(t1.getPtr()));
    CHECK(!uut.isEmpty());

    int m = 0;
    for(const auto &f: uut)
    {
        CHECK(f.testAccess() == &t1);
        m++;
    }
    CHECK(m == 1);

    CHECK(uut.remove(t1.getPtr()) == t1.getPtr());

    int o = 0;
    for(const auto &f: uut) { o++; }
    CHECK(o == 0);

    CHECK(uut.isEmpty());
}

TEST(LinkedListSpecial, TwoItems)
{
    TargetLike t1, t2;

    LinkedPtrList<PtrLike> uut;

    CHECK(uut.isEmpty());
    uut.fastAdd(t1.getPtr());
    CHECK(uut.addBack(t2.getPtr()));
    CHECK(!uut.addBack(t1.getPtr()));
    CHECK(!uut.isEmpty());

    auto it = uut.begin();
    auto start = it;

    CHECK(uut.end() == uut.end());
    CHECK(it != uut.end());
    CHECK(it == start);
    CHECK(start == it);
    CHECK((*it).testAccess() == &t1);
    ++it;
    CHECK(it != uut.end());
    CHECK(uut.end() != it);
    CHECK(it != start);
    CHECK(start != it);
    CHECK((*it).testAccess() == &t2);
    ++it;
    CHECK(it == uut.end());
    CHECK(uut.end() == it);
    CHECK(it != start);
    CHECK(start != it);
}
