/*******************************************************************************
 *
 * Copyright (c) 2016 Tamás Seller. All rights reserved.
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

#include "heap/Heap.h"

#include "1test/Test.h"
#include "1test/Mock.h"

#include <set>
#include <vector>
#include <algorithm>

#include <cstdint>
#include <climits>
#include <cstring>

#undef new

using namespace pet;

template<class SizeType, unsigned int spare>
class MockPolicy: HeapBase<SizeType>
{
public:
	static constexpr uintptr_t freeHeaderSize = spare;
	using Base = HeapBase<SizeType>;

	std::vector<typename Base::Block> freeBlocks;

	void add(typename Base::Block block)
	{
		MOCK(HeapPolicy)::CALL(add);
		this->freeBlocks.insert(this->freeBlocks.begin(), block);
	}

	void init(typename Base::Block block) {
		this->freeBlocks.insert(this->freeBlocks.begin(), block);
	}

	void remove(typename Base::Block block)
	{
		MOCK(HeapPolicy)::CALL(remove);
		for(auto it = freeBlocks.begin(); it != freeBlocks.end(); it++) {
			if((*it).ptr == block.ptr) {
				this->freeBlocks.erase(it);
				return;
			}
		}
	}

	typename Base::Block findAndRemove(unsigned int size)
	{
		MOCK(HeapPolicy)::CALL(findAndRemove);

		for(auto it = freeBlocks.begin(); it != freeBlocks.end(); it++)
		{
			if((*it).getSize() >= size)
			{
				auto ret = *it;
				this->freeBlocks.erase(it);
				return ret;
			}
		}

		return 0;
	}

	void update(unsigned int size, typename Base::Block block) {
		MOCK(HeapPolicy)::CALL(update);
	}
};

template<class SizeType, unsigned int alignmentBits, unsigned int spare, bool useChecksum>
struct HeapTest
{
	class Uut: public Heap<MockPolicy<SizeType, spare>, SizeType, alignmentBits, useChecksum>
	{
	    uint32_t space[2048 / sizeof(uint32_t) + 1];
	public:
	    static constexpr size_t size = 2048;
	    void *const start = (char*)space + 1;

		inline Uut(): Uut::Heap(((char*)space) + 1, size) {}
	};

	TEST_GROUP(HeapHost)
	{
		Uut *heap;

		TEST_SETUP() { heap = new Uut; }
		TEST_TEARDOWN() { delete heap; }

		void* alloc(uintptr_t size, bool shouldFail = false)
		{
			auto ret = heap->alloc(size);

			CHECK((ret == nullptr) == shouldFail);
			CHECK(!(((uintptr_t)ret) & ~(-1u << alignmentBits)));

			return ret;
		}

		auto stats() {
			return this->heap->getStats((char*)this->heap->start);
		}

		bool checkStatsEmpty()
		{
			auto stats = this->stats();
			return stats.nUsed == 0
				&& stats.totalUsed == 0
				&& stats.longestFree == stats.totalFree
				&& stats.totalFree < this->heap->size;
		}

		bool checkStatsOneFree(int totalUsed, int nUsed)
		{
			auto stats = this->stats();
			return stats.nUsed == nUsed
				&& stats.totalUsed >= totalUsed
				&& stats.longestFree == stats.totalFree
				&& stats.totalFree < this->heap->size - totalUsed;
		}

		bool checkStatsMultipleFree(int totalUsed, int nUsed)
		{
			auto stats = this->stats();
			return stats.nUsed == nUsed
				&& stats.totalUsed >= totalUsed
				&& stats.longestFree <= stats.totalFree
				&& stats.totalFree < this->heap->size - totalUsed;
		}

		bool checkStatsFull(int totalUsed, int nUsed)
		{
			auto stats = this->stats();
			return stats.nUsed == nUsed
				&& stats.totalUsed >= totalUsed
				&& stats.longestFree == 0
				&& stats.totalFree == 0;
		}
	};

	BEGIN_TEST_CASE(HeapHost, Sanity)
	{
		CHECK(this->checkStatsEmpty());

		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		MOCK(HeapPolicy)::EXPECT(add);
		auto r = this->alloc(100);

		CHECK(this->checkStatsOneFree(100, 1));

		MOCK(HeapPolicy)::EXPECT(remove);
		MOCK(HeapPolicy)::EXPECT(add);
		this->heap->free(r);

		CHECK(this->checkStatsEmpty());
	}
	END_TEST_CASE()

	BEGIN_TEST_CASE(HeapHost, OverSizedFails)
	{
		CHECK(this->checkStatsEmpty());

		this->alloc(uintptr_t(-1), true);

		CHECK(this->checkStatsEmpty());
	}
	END_TEST_CASE()

	BEGIN_TEST_CASE(HeapHost, Deplete)
	{
		const auto size = this->heap->size / 2;

		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		MOCK(HeapPolicy)::EXPECT(add);
		auto r = this->alloc(size);

		CHECK(this->checkStatsOneFree(size, 1));

		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		this->alloc(size, true);

		CHECK(this->checkStatsOneFree(size, 1));
	}
	END_TEST_CASE()

	BEGIN_TEST_CASE(HeapHost, ManySmall)
	{
		std::set<void*> returned;

		for(int i = 0; i < 30; i++)
		{
			MOCK(HeapPolicy)::EXPECT(findAndRemove);
			MOCK(HeapPolicy)::EXPECT(add);
			auto r = this->alloc(1);
			CHECK(returned.count(r) == 0);
			returned.insert(r);

			CHECK(this->checkStatsOneFree(i + 1, i + 1));
		}
	}
	END_TEST_CASE()

	BEGIN_TEST_CASE(HeapHost, ForwardMergeWithEnd)
	{
		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		MOCK(HeapPolicy)::EXPECT(add);
		auto r = this->alloc(4);
		CHECK(r);

		CHECK(this->checkStatsOneFree(4, 1));

		MOCK(HeapPolicy)::EXPECT(remove);
		MOCK(HeapPolicy)::EXPECT(add);
		this->heap->free(r);

		CHECK(this->checkStatsEmpty());

		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		MOCK(HeapPolicy)::EXPECT(add);
		auto r2 = this->alloc(4);
		CHECK(r == r2);

		CHECK(this->checkStatsOneFree(4, 1));
	}
	END_TEST_CASE()

	BEGIN_TEST_CASE(HeapHost, NoMerge)
	{
		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		MOCK(HeapPolicy)::EXPECT(add);
		auto r = this->alloc(4);
		CHECK(r);

		CHECK(this->checkStatsOneFree(4, 1));

		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		MOCK(HeapPolicy)::EXPECT(add);
		auto r2 = this->alloc(4);
		CHECK(r != r2);

		CHECK(this->checkStatsOneFree(2 * 4, 2));

		MOCK(HeapPolicy)::EXPECT(add);
		this->heap->free(r);

		CHECK(this->checkStatsMultipleFree(4, 1));
	}
	END_TEST_CASE()

	BEGIN_TEST_CASE(HeapHost, BackwardsMerge)
	{
		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		MOCK(HeapPolicy)::EXPECT(add);
		auto r = this->alloc(4);
		CHECK(r);

		CHECK(this->checkStatsOneFree(4, 1));

		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		MOCK(HeapPolicy)::EXPECT(add);
		auto r2 = this->alloc(4);
		CHECK(r != r2);

		CHECK(this->checkStatsOneFree(2 * 4, 2));

		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		MOCK(HeapPolicy)::EXPECT(add);
		auto r3 = this->alloc(4);
		CHECK(r != r3 && r2 != r3);

		CHECK(this->checkStatsOneFree(3 * 4, 3));

		MOCK(HeapPolicy)::EXPECT(add);
		this->heap->free(r);

		CHECK(this->checkStatsMultipleFree(2 * 4, 2));

		MOCK(HeapPolicy)::EXPECT(update);
		this->heap->free(r2);

		CHECK(this->checkStatsMultipleFree(4, 1));
	}
	END_TEST_CASE()

	BEGIN_TEST_CASE(HeapHost, ForwardsMergeNotEnd)
	{
		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		MOCK(HeapPolicy)::EXPECT(add);
		auto r = this->alloc(4);
		CHECK(r);

		CHECK(this->checkStatsOneFree(4, 1));

		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		MOCK(HeapPolicy)::EXPECT(add);
		auto r2 = this->alloc(4);
		CHECK(r != r2);

		CHECK(this->checkStatsOneFree(2 * 4, 2));

		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		MOCK(HeapPolicy)::EXPECT(add);
		auto r3 = this->alloc(4);
		CHECK(r != r3 && r2 != r3);

		CHECK(this->checkStatsOneFree(3 * 4, 3));

		MOCK(HeapPolicy)::EXPECT(add);
		this->heap->free(r2);

		CHECK(this->checkStatsMultipleFree(2 * 4, 2));

		MOCK(HeapPolicy)::EXPECT(remove);
		MOCK(HeapPolicy)::EXPECT(add);
		this->heap->free(r);

		CHECK(this->checkStatsMultipleFree(4, 1));
	}
	END_TEST_CASE()


	BEGIN_TEST_CASE(HeapHost, BothWaysMerge)
	{
		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		MOCK(HeapPolicy)::EXPECT(add);
		auto r = this->alloc(4);
		CHECK(r);

		CHECK(this->checkStatsOneFree(4, 1));

		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		MOCK(HeapPolicy)::EXPECT(add);
		auto r2 = this->alloc(4);
		CHECK(r != r2);

		CHECK(this->checkStatsOneFree(2 * 4, 2));

		MOCK(HeapPolicy)::EXPECT(add);
		this->heap->free(r);

		CHECK(this->checkStatsMultipleFree(4, 1));

		MOCK(HeapPolicy)::EXPECT(remove);
		MOCK(HeapPolicy)::EXPECT(update);
		this->heap->free(r2);

		CHECK(this->checkStatsEmpty());
	}
	END_TEST_CASE()

	BEGIN_TEST_CASE(HeapHost, AllocFreeLongest)
	{
		auto s = this->stats();

		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		auto r = this->alloc(s.longestFree);
		CHECK(r);

		CHECK(this->checkStatsFull(s.longestFree, 1));

		MOCK(HeapPolicy)::EXPECT(add);
		this->heap->free(r);

		CHECK(this->checkStatsEmpty());
	}
	END_TEST_CASE()

	BEGIN_TEST_CASE(HeapHost, AllocFreeLongestMinusOne)
	{
		auto s = this->stats();

		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		auto r = this->alloc(s.longestFree - 1);
		CHECK(r);

		CHECK(this->checkStatsFull(s.longestFree - 1, 1));

		MOCK(HeapPolicy)::EXPECT(add);
		this->heap->free(r);

		CHECK(this->checkStatsEmpty());
	}
	END_TEST_CASE()

	BEGIN_TEST_CASE(HeapHost, BackwardsMergeFromEnd)
	{
		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		MOCK(HeapPolicy)::EXPECT(add);
		auto r = this->alloc(100);
		CHECK(r);

		CHECK(this->checkStatsOneFree(4, 1));

		auto s = this->stats();

		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		auto r2 = this->alloc(s.longestFree);
		CHECK(r != r2);

		CHECK(this->checkStatsFull(4 + s.longestFree, 2));

		MOCK(HeapPolicy)::EXPECT(add);
		this->heap->free(r);

		CHECK(this->checkStatsOneFree(s.longestFree, 1));

		MOCK(HeapPolicy)::EXPECT(update);
		this->heap->free(r2);

		CHECK(this->checkStatsEmpty());
	}
	END_TEST_CASE()

	BEGIN_TEST_CASE(HeapHost, ShrinkNonFull)
	{
		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		MOCK(HeapPolicy)::EXPECT(add);
		auto r = this->alloc(1000);
		CHECK(r);

		CHECK(this->checkStatsOneFree(1000, 1));

		MOCK(HeapPolicy)::EXPECT(remove);
		MOCK(HeapPolicy)::EXPECT(add);
		auto newSize = this->heap->resize(r, 100);
		CHECK(100 <= newSize && newSize < 1000);

		CHECK(this->checkStatsOneFree(100, 1));
	}
	END_TEST_CASE()

	BEGIN_TEST_CASE(HeapHost, NoShrinkSmall)
	{
		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		MOCK(HeapPolicy)::EXPECT(add);
		auto r = this->alloc(2);
		CHECK(r);

		CHECK(this->checkStatsOneFree(2, 1));

		auto newSize = this->heap->resize(r, 1);
		CHECK(2 <= newSize);

		CHECK(this->checkStatsOneFree(2, 1));
	}
	END_TEST_CASE()

	BEGIN_TEST_CASE(HeapHost, ShrinkFromFull)
	{
		auto s = this->stats();

		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		auto r = this->alloc(s.longestFree);
		CHECK(r);

		CHECK(this->checkStatsOneFree(s.longestFree, 1));

		MOCK(HeapPolicy)::EXPECT(add);
		auto newSize = this->heap->resize(r, s.longestFree / 2);
		CHECK(s.longestFree / 2 <= newSize && newSize < s.longestFree);

		CHECK(this->checkStatsOneFree(s.longestFree / 2, 1));
	}
	END_TEST_CASE()

	BEGIN_TEST_CASE(HeapHost, ShrinkNextNotFree)
	{
		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		MOCK(HeapPolicy)::EXPECT(add);
		auto r = this->alloc(1000);
		CHECK(r);

		CHECK(this->checkStatsOneFree(1000, 1));

		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		MOCK(HeapPolicy)::EXPECT(add);
		auto r2 = this->alloc(100);
		CHECK(r2);

		CHECK(this->checkStatsMultipleFree(100 + 1000, 2));

		MOCK(HeapPolicy)::EXPECT(add);
		auto newSize = this->heap->resize(r, 10);
		CHECK(10 <= newSize && newSize < 1000);

		CHECK(this->checkStatsMultipleFree(100 + 10, 2));
	}
	END_TEST_CASE()

	BEGIN_TEST_CASE(HeapHost, GrowNonFull)
	{
		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		MOCK(HeapPolicy)::EXPECT(add);
		auto r = this->alloc(100);
		CHECK(r);

		CHECK(this->checkStatsOneFree(100, 1));

		MOCK(HeapPolicy)::EXPECT(remove);
		MOCK(HeapPolicy)::EXPECT(add);
		auto newSize = this->heap->resize(r, 1000);
		CHECK(1000 <= newSize);

		CHECK(this->checkStatsOneFree(1000, 1));
	}
	END_TEST_CASE()

	BEGIN_TEST_CASE(HeapHost, NoGrowFromFull)
	{
		auto s = this->stats();

		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		auto r = this->alloc(s.longestFree);
		CHECK(r);

		CHECK(this->checkStatsOneFree(s.longestFree, 1));

		auto newSize = this->heap->resize(r, s.longestFree + 1000);
		CHECK(s.longestFree <= newSize && newSize < s.longestFree + 1000);

		CHECK(this->checkStatsOneFree(s.longestFree, 1));
	}
	END_TEST_CASE()

	BEGIN_TEST_CASE(HeapHost, NoGrowNextNotFree)
	{
		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		MOCK(HeapPolicy)::EXPECT(add);
		auto r = this->alloc(100);
		CHECK(r);

		CHECK(this->checkStatsOneFree(100, 1));

		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		MOCK(HeapPolicy)::EXPECT(add);
		auto r2 = this->alloc(10);
		CHECK(r2);

		CHECK(this->checkStatsOneFree(100 + 10, 2));

		auto newSize = this->heap->resize(r, 200);
		CHECK(100 <= newSize && newSize < 200);

		CHECK(this->checkStatsOneFree(100 + 10, 2));
	}
	END_TEST_CASE()

	BEGIN_TEST_CASE(HeapHost, NoGrowNextNotBigEnough)
	{
		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		MOCK(HeapPolicy)::EXPECT(add);
		auto r = this->alloc(100);
		CHECK(r);

		CHECK(this->checkStatsOneFree(100, 1));

		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		MOCK(HeapPolicy)::EXPECT(add);
		auto r2 = this->alloc(10);
		CHECK(r2);

		CHECK(this->checkStatsOneFree(100 + 10, 2));

		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		MOCK(HeapPolicy)::EXPECT(add);
		auto r3 = this->alloc(100);
		CHECK(r3);

		CHECK(this->checkStatsOneFree(100 + 100 + 10, 3));

		MOCK(HeapPolicy)::EXPECT(add);
		this->heap->free(r2);

		CHECK(this->checkStatsMultipleFree(100 + 100, 2));

		auto newSize = this->heap->resize(r, 200);
		CHECK(100 <= newSize && newSize < 200);

		CHECK(this->checkStatsMultipleFree(100 + 100, 2));
	}
	END_TEST_CASE()

	BEGIN_TEST_CASE(HeapHost, GrowMergeNext)
	{
		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		MOCK(HeapPolicy)::EXPECT(add);
		auto r = this->alloc(100);
		CHECK(r);

		CHECK(this->checkStatsOneFree(100, 1));

		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		MOCK(HeapPolicy)::EXPECT(add);
		auto r2 = this->alloc(10);
		CHECK(r2);

		CHECK(this->checkStatsOneFree(100 + 10, 2));

		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		MOCK(HeapPolicy)::EXPECT(add);
		auto r3 = this->alloc(100);
		CHECK(r3);

		CHECK(this->checkStatsOneFree(100 + 100 + 10, 3));

		MOCK(HeapPolicy)::EXPECT(add);
		this->heap->free(r2);

		CHECK(this->checkStatsMultipleFree(100 + 100, 2));

		MOCK(HeapPolicy)::disable();

		auto newSize = this->heap->resize(r, 110);
		CHECK(110 <= newSize);

		MOCK(HeapPolicy)::enable();

		CHECK(this->checkStatsOneFree(110 + 100, 2) || this->checkStatsMultipleFree(110 + 100, 2));
	}
	END_TEST_CASE()

	BEGIN_TEST_CASE(HeapHost, DropHalfNoPrev)
	{
		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		MOCK(HeapPolicy)::EXPECT(add);
		auto r = this->alloc(100);
		CHECK(r);

		CHECK(this->checkStatsOneFree(100, 1));

		MOCK(HeapPolicy)::EXPECT(add);
		auto r2 = this->heap->dropFront(r, 50);
		CHECK(r <= r2 && r2 <= ((char*)r + 50));

		CHECK(this->checkStatsMultipleFree(50, 1));
	}
	END_TEST_CASE()

	BEGIN_TEST_CASE(HeapHost, NoDropOne)
	{
		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		MOCK(HeapPolicy)::EXPECT(add);
		auto r = this->alloc(100);
		CHECK(r);

		CHECK(this->checkStatsOneFree(100, 1));

		auto r2 = this->heap->dropFront(r, 1);
		CHECK(r == r2);

		CHECK(this->checkStatsOneFree(100, 1));
	}
	END_TEST_CASE()

	BEGIN_TEST_CASE(HeapHost, DropAll)
	{
		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		MOCK(HeapPolicy)::EXPECT(add);
		auto r = this->alloc(100);
		CHECK(r);

		CHECK(this->checkStatsOneFree(100, 1));

		MOCK(HeapPolicy)::EXPECT(add);
		auto r2 = this->heap->dropFront(r, 100);
		CHECK(r < r2 && r2 < (char*)r + 100);

		CHECK(this->checkStatsMultipleFree(0, 1));
	}
	END_TEST_CASE()

	BEGIN_TEST_CASE(HeapHost, NoDropSmallNoPrev)
	{
		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		MOCK(HeapPolicy)::EXPECT(add);
		auto r = this->alloc(100);
		CHECK(r);

		CHECK(this->checkStatsOneFree(100, 1));

		MOCK(HeapPolicy)::disable();

		static constexpr auto offs = 1 << alignmentBits;
		auto r2 = this->heap->dropFront(r, offs);
		CHECK(r <= r2);

		MOCK(HeapPolicy)::enable();

		CHECK(this->checkStatsOneFree(100, 1) || this->checkStatsMultipleFree(100 - offs, 1));
	}
	END_TEST_CASE()

	BEGIN_TEST_CASE(HeapHost, NoDropSmallPrevUsed)
	{
		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		MOCK(HeapPolicy)::EXPECT(add);
		auto p = this->alloc(100);
		CHECK(p);

		CHECK(this->checkStatsOneFree(100, 1));

		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		MOCK(HeapPolicy)::EXPECT(add);
		auto r = this->alloc(100);
		CHECK(r && r != p);

		CHECK(this->checkStatsOneFree(2 * 100, 2));

		MOCK(HeapPolicy)::disable();

		static constexpr auto offs = 1 << alignmentBits;
		auto r2 = this->heap->dropFront(r, offs);
		CHECK(r <= r2);

		MOCK(HeapPolicy)::enable();

		CHECK(this->checkStatsOneFree(2 * 100, 2) || this->checkStatsMultipleFree(100 + 100 - offs, 2));
	}
	END_TEST_CASE()

	BEGIN_TEST_CASE(HeapHost, DropSmallPrevFree)
	{
		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		MOCK(HeapPolicy)::EXPECT(add);
		auto p = this->alloc(100);
		CHECK(p);

		CHECK(this->checkStatsOneFree(100, 1));

		MOCK(HeapPolicy)::EXPECT(findAndRemove);
		MOCK(HeapPolicy)::EXPECT(add);
		auto r = this->alloc(100);
		CHECK(r && r != p);

		CHECK(this->checkStatsOneFree(2 * 100, 2));

		MOCK(HeapPolicy)::EXPECT(add);
		this->heap->free(p);

		CHECK(this->checkStatsMultipleFree(100, 1));

		static constexpr auto offs = 1 << alignmentBits;
		auto r2 = this->heap->dropFront(r, offs);
		CHECK(r < r2 && r2 <= ((char*)r + offs));

		CHECK(this->checkStatsMultipleFree(100 - offs, 1));
	}
	END_TEST_CASE()
};
