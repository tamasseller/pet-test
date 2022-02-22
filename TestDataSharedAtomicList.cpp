/*******************************************************************************
 *
 * Copyright (c) 2022 Tamás Seller. All rights reserved.
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

#include "data/SharedAtomicList.h"

#include "1test/Test.h"

TEST_GROUP(SharedAtomicList){};

TEST(SharedAtomicList, Sanity)
{
    pet::SharedAtomicList::Element e;
    pet::SharedAtomicList uut;

    {
        auto r = uut.read();
        CHECK("initially empty", r.peek() == nullptr);
    }

    CHECK("can push into empty", uut.push(&e));

    for(int i =0; i < 3; i++)
    {
        CHECK("can not push contained", !uut.push(&e));

        auto r = uut.read();

        CHECK("read does not remove", !uut.push(&e));

        CHECK("can retrieve contained", r.peek() == &e);

        CHECK("peek does not remove", !uut.push(&e));

        r.pop();

        CHECK("pop removes", uut.push(&e));

        CHECK("reader depleted", r.peek() == nullptr);
    }
}

TEST(SharedAtomicList, Ordering)
{
    pet::SharedAtomicList::Element e, f, g;
    pet::SharedAtomicList uut;

    CHECK("can push first", uut.push(&e));
    CHECK("can push second", uut.push(&f));
    CHECK("can push third", uut.push(&g));

    CHECK("can not repush first", !uut.push(&e));
    CHECK("can not repush second", !uut.push(&f));
    CHECK("can not repush third", !uut.push(&g));

    auto r = uut.read();

    CHECK("can retrieve first", r.peek() == &e);
    r.pop();

    CHECK("can retrieve second", r.peek() == &f);
    r.pop();

    CHECK("can push removed again while still reading", uut.push(&f));
    CHECK("can push removed again while still reading", uut.push(&e));

    CHECK("can retrieve third", r.peek() == &g);
    r.pop();

    CHECK("reader depleted", r.peek() == nullptr);

    auto s = uut.read();

    CHECK("can read first element pushed while reading", s.peek() == &f);
    s.pop();

    CHECK("can read second element pushed while reading", s.peek() == &e);
    s.pop();

    CHECK("reader depleted", r.peek() == nullptr);
}
