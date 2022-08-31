// Copyright (c) Microsoft Corporation.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <deque>
#include <forward_list>
#include <ranges>
#include <span>
#include <stack>
#include <type_traits>
#include <utility>
#include <vector>

#include <range_algorithm_support.hpp>

using namespace std;

template <class T>
struct accessible_adaptor : T {
    using T::T;

    auto& get_container() {
        return this->c;
    }
    auto& get_container() const {
        return this->c;
    }
};

template <ranges::input_range Rng, size_t N>
void test_stack(const size_t presize, Rng&& rng, const int (&expected)[N]) {
    accessible_adaptor<stack<int>> d{deque<int>(presize, -1)};

    d.push_range(forward<Rng>(rng));
    const auto count      = static_cast<ptrdiff_t>(presize);
    const auto& container = d.get_container();
    assert(ranges::count(container | views::take(count), -1) == count);
    assert(ranges::equal(container | views::drop(count), expected));
}

template <ranges::input_range Rng, size_t N>
void test_stack(const int (&expected)[N]) {
    test_stack(0, Rng{expected}, expected);
    test_stack(2, Rng{expected}, expected);
}

static constexpr int some_ints[] = {0, 1, 2, 3, 4, 5, 6, 7};

struct instantiator {
    template <ranges::input_range R>
    static void call() {
        test_stack<R>(some_ints);
    }
};

template <class Category, test::Common IsCommon, bool is_random = derived_from<Category, random_access_iterator_tag>>
using move_only_view = test::range<Category, const int, test::Sized{is_random}, test::CanDifference{is_random},
    IsCommon, test::CanCompare{derived_from<Category, forward_iterator_tag>},
    test::ProxyRef{!derived_from<Category, contiguous_iterator_tag>}, test::CanView::yes, test::Copyability::move_only>;

void test_copyable_view() {
    test_stack<span<const int>>(some_ints);
}

void test_move_only_view() {
    test_stack<move_only_view<input_iterator_tag, test::Common::no>>(some_ints);
    test_stack<move_only_view<forward_iterator_tag, test::Common::no>>(some_ints);
    test_stack<move_only_view<forward_iterator_tag, test::Common::yes>>(some_ints);
    test_stack<move_only_view<bidirectional_iterator_tag, test::Common::no>>(some_ints);
    test_stack<move_only_view<bidirectional_iterator_tag, test::Common::yes>>(some_ints);
    test_stack<move_only_view<random_access_iterator_tag, test::Common::no>>(some_ints);
    test_stack<move_only_view<random_access_iterator_tag, test::Common::yes>>(some_ints);
}

void test_c_array() {
    test_stack(0, some_ints, some_ints);
}

void test_lvalue_vector() {
    vector vec(from_range, some_ints);
    test_stack(0, vec, some_ints);
}

void test_lvalue_forward_list() {
    forward_list lst(ranges::begin(some_ints), ranges::end(some_ints));
    test_stack(0, lst, some_ints);
}

int main() {
    // Validate views
    test_copyable_view();
    test_move_only_view();

    // Validate non-views
    test_c_array();
    test_lvalue_vector();
    test_lvalue_forward_list();

    test_in<instantiator, const int>();
}