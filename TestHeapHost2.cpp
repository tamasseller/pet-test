/*******************************************************************************
 *
 * Copyright (c) 2020 Tamás Seller. All rights reserved.
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

#include "TestHeapHostSuite.h"

template class HeapTest<uint16_t, 1, 14, true, true>;
template class HeapTest<uint16_t, 2, 14, false, false>;
template class HeapTest<uint16_t, 3, 14, true, true>;
template class HeapTest<uint16_t, 4, 14, false, false>;

template class HeapTest<uint32_t, 2, 14, true, true>;
template class HeapTest<uint32_t, 3, 14, false, false>;
template class HeapTest<uint32_t, 4, 14, true, true>;


