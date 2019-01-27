#include <benchmark/benchmark.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/SmallSet.h>
#include <llvm/ADT/DenseMap.h>
#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include <random>       // for std::default_random_engine
#include "data_types.h"
#include "ArrayTypes.h"

static inline int rand(int min, int max) { return min + rand() % (max - min + 1); }

#define BENCHMARK_VECTORS 1
#define BENCHMARK_MAPS 1
#define BENCHMARK_SETS 1


#define DO_BENCHMARK_TEMPLATE(bm_function, container) \
		BENCHMARK_TEMPLATE(bm_function, container<int>,     int    )->Arg(4)->Arg(8)->Arg(16)->Arg(32)->Arg(64)->Arg(256)->Arg(1024)->Arg(4096)->Arg(16384); \
		BENCHMARK_TEMPLATE(bm_function, container<size_16>, size_16)->Arg(4)->Arg(8)->Arg(16)->Arg(32)->Arg(64)->Arg(256)->Arg(1024)->Arg(4096)->Arg(16384); \
		BENCHMARK_TEMPLATE(bm_function, container<size_64>, size_64)->Arg(4)->Arg(8)->Arg(16)->Arg(32)->Arg(64)->Arg(256)->Arg(1024)->Arg(4096)->Arg(16384);

#define BENCHMARK_TEMPLATES_3(bm_function, container0, container1, container2) \
		DO_BENCHMARK_TEMPLATE(bm_function, container0); \
		DO_BENCHMARK_TEMPLATE(bm_function, container1); \
		DO_BENCHMARK_TEMPLATE(bm_function, container2);

#define BENCHMARK_TEMPLATES_4(bm_function, container0, container1, container2, container3) \
		DO_BENCHMARK_TEMPLATE(bm_function, container0); \
		DO_BENCHMARK_TEMPLATE(bm_function, container1); \
		DO_BENCHMARK_TEMPLATE(bm_function, container2); \
		DO_BENCHMARK_TEMPLATE(bm_function, container3);

#define BENCHMARK_TEMPLATES_5(bm_function, container0, container1, container2, container3, container4) \
		DO_BENCHMARK_TEMPLATE(bm_function, container0); \
		DO_BENCHMARK_TEMPLATE(bm_function, container1); \
		DO_BENCHMARK_TEMPLATE(bm_function, container2); \
		DO_BENCHMARK_TEMPLATE(bm_function, container3); \
		DO_BENCHMARK_TEMPLATE(bm_function, container4);

/**
 * Give you a random ordering of [size] indices. Use this ordering to test random access of indices.
 * This ordering will be stored, and the same ordering returned for *all* future calls with this size.
 * This allows us to make a true apples-to-apples comparison between container types.
 * (You wouldn't want the randomized indices to change between different containers, lest you get
 * really lucky with one container and really unlucky with another.)
 */
const std::vector<int> & randomize_lookup_indices(size_t size);

#if BENCHMARK_VECTORS
	template <typename Data> using SmallVec8    = llvm::SmallVector<Data, 8>;
	template <typename Data> using SmallVec16   = llvm::SmallVector<Data, 16>;
	template <typename Data> using SmallVec1024 = llvm::SmallVector<Data, 1024>;

	template<class ContainerT, class ValueT>
	void BM_vector_emplace_back(benchmark::State &state) {
		for(auto _ : state)
		{
			ContainerT container;
			container.reserve(state.range(0));
			for(int i = 0; i < state.range(0); ++i)
			{
				container.emplace_back(generate_value<ValueT>(i));
				benchmark::DoNotOptimize(container.data());
			}
			benchmark::ClobberMemory();
		}
	}
	//BENCHMARK_TEMPLATES_4(BM_vector_emplace_back, vector, SmallVec8, SmallVec16, SmallVec1024);

	template<class ContainerT, class ValueT>
	void BM_vector_sequential_read(benchmark::State& state) {
		std::vector<ValueT> client;
		client.reserve(state.range(0));
		for(int i = 0; i < state.range(0); ++i)
		{
			client.emplace_back(generate_value<ValueT>(i));
		}

		ContainerT container(client.begin(), client.end());
		ValueT val;
		for(auto _ : state)
		{
			for(const auto &item : container)
			{
				benchmark::DoNotOptimize(val = item);
			}
			benchmark::ClobberMemory();
		}
	}
	BENCHMARK_TEMPLATES_5(BM_vector_sequential_read, std::vector, SmallVec8, SmallVec16, SmallVec1024, FixedArray);

	template<class ContainerT, class ValueT>
	void BM_vector_rand_read(benchmark::State& state) {
		std::vector<ValueT> client;
		client.reserve(state.range(0));
		for(int i = 0; i < state.range(0); ++i)
		{
			client.emplace_back(generate_value<ValueT>(i));
		}

		const std::vector<int> &indices_to_lookup = randomize_lookup_indices(state.range(0));

		ContainerT container(client.begin(), client.end());
		ValueT val;
		for(auto _ : state)
		{
			for(const auto &idx : indices_to_lookup)
			{
				benchmark::DoNotOptimize(val = *(container.begin() + idx));
			}
			benchmark::ClobberMemory();
		}
	}
	BENCHMARK_TEMPLATES_5(BM_vector_rand_read, std::vector, SmallVec8, SmallVec16, SmallVec1024, FixedArray);
#endif // BENCHMARK_VECTORS


#if BENCHMARK_MAPS
	template <typename Data> using ptr_map = std::map<intptr_t, Data>;
	template <typename Data> using ptr_unordered_map = std::unordered_map<intptr_t, Data>;
	template <typename Data> using ptr_dense_map = llvm::DenseMap<intptr_t, Data>;
	template <typename Data> using ptr_array_map = ArrayMap<intptr_t, Data>;

	template<class ContainerT, class ValueT>
	void BM_map_insert(benchmark::State &state) {
		for(auto _ : state)
		{
			ContainerT container;
			for(int i = 0; i < state.range(0); ++i)
			{
				container[i] = generate_value<ValueT>(i);
			}
			benchmark::ClobberMemory();
		}
	}
	BENCHMARK_TEMPLATES_3(BM_map_insert, ptr_map, ptr_unordered_map, ptr_dense_map);

	template<class ContainerT, class ValueT>
	void BM_map_lookup(benchmark::State &state) {
		std::map<intptr_t, ValueT> client;
		for(int i = 0; i < state.range(0); ++i)
		{
			client[i] = generate_value<ValueT>(i);
		}

		const std::vector<int> &keys_to_lookup = randomize_lookup_indices(state.range(0));

		ContainerT container(client.begin(), client.end());
		benchmark::ClobberMemory();
		ValueT val;
		for(auto _ : state)
		{
			for(const auto &key : keys_to_lookup)
			{
				benchmark::DoNotOptimize(val = container[key]);
			}
			benchmark::ClobberMemory();
		}
	}
	BENCHMARK_TEMPLATES_4(BM_map_lookup, ptr_map, ptr_unordered_map, ptr_dense_map, ptr_array_map);
#endif // BENCHMARK_MAPS

#if BENCHMARK_SETS
template <typename Data> using sm_set8 = llvm::SmallSet<Data, 8>;
template <typename Data> using sm_set16 = llvm::SmallSet<Data, 16>;

template<class ContainerT, class ValueT>
void BM_set_insert(benchmark::State &state) {
	for(auto _ : state)
	{
		ContainerT container;
		for(int i = 0; i < state.range(0); ++i)
		{
			container.insert(generate_value<ValueT>(i));
		}
		benchmark::ClobberMemory();
	}
}
BENCHMARK_TEMPLATES_3(BM_set_insert, std::set, sm_set8, sm_set16);

template<class ContainerT, class ValueT>
void BM_set_read(benchmark::State& state) {
	std::vector<ValueT> vals_to_lookup;
	vals_to_lookup.reserve(state.range(0));
	std::set<ValueT> client;
	for(int i = 0; i < state.range(0); ++i)
	{
		client.insert(generate_value<ValueT>(i));
		vals_to_lookup.push_back(generate_value<ValueT>(i));
	}
	shuffle(vals_to_lookup.begin(), vals_to_lookup.end(), std::default_random_engine(0));
	
	ContainerT container(client.begin(), client.end());
	int count = 0;
	for(auto _ : state)
	{
		for(const auto &val : vals_to_lookup)
		{
			benchmark::DoNotOptimize(count += container.count(val));
		}
		benchmark::ClobberMemory();
	}
}
BENCHMARK_TEMPLATES_4(BM_set_read, std::set, sm_set8, sm_set16, ArraySet);
#endif // BENCHMARK_SETS


BENCHMARK_MAIN();









const std::vector<int> & randomize_lookup_indices(size_t size)
{
	static llvm::DenseMap<int, std::vector<int>> s_existing_orderings;

	auto it = s_existing_orderings.find(size);
	if(it == s_existing_orderings.end())
	{
		std::vector<int> new_ordering;
		new_ordering.reserve(size);
		for(int i = 0; i < size; ++i)
		{
			new_ordering.push_back(i);
		}
		shuffle(new_ordering.begin(), new_ordering.end(), std::default_random_engine(0));
		it = s_existing_orderings.insert(make_pair(size, new_ordering)).first;
	}
	return it->second;
}

