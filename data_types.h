/**
 * Simple utilities for the data types we'll benchmark
 */

#ifndef LLVM_DATA_STRUCTURE_BENCHMARKS_DATA_TYPES_H
#define LLVM_DATA_STRUCTURE_BENCHMARKS_DATA_TYPES_H

/* A generic struct whose sizeof == 16 bytes */
struct size_16 {
	size_16() : a(0), b(0) { }
	size_16(double a, double b) : a(a), b(b) { }
	bool operator<(const size_16 &other) const { return a < other.a && b < other.b; }
	bool operator==(const size_16 &other) const { return a == other.a && b == other.b; }
	double a;
	double b;
};

/* A generic struct whose sizeof == 64 bytes */
struct size_64 {
	size_64() { }
	size_64(const size_16 &a, const size_16 &b, const size_16 &c, const size_16 &d) : a(a), b(b), c(c), d(d) { }
	bool operator<(const size_64 &other) const { return a < other.a && b < other.b && c < other.c && d < other.d; }
	bool operator==(const size_64 &other) const { return a == other.a && b == other.b && c == other.c && d == other.d; }
	size_16 a;
	size_16 b;
	size_16 c;
	size_16 d;
};

// Template method to generate unique values for the data types we'll be testing.
// This lets us write *one* test function that covers *all* our types, rather than
// having separate methods for test_int(), test_size_16(), test_size_64(), etc.
template<class T> T       generate_value         (int iteration);
template<>        int     generate_value<int>    (int iteration) { return iteration; }
template<>        size_16 generate_value<size_16>(int iteration) { return size_16(iteration, iteration + 1); }
template<>        size_64 generate_value<size_64>(int iteration) { return size_64(size_16(iteration, iteration + 1), size_16(iteration + 2, iteration + 3), size_16(iteration + 4, iteration + 5), size_16(iteration + 6, iteration + 7)); }


#endif //LLVM_DATA_STRUCTURE_BENCHMARKS_DATA_TYPES_H
