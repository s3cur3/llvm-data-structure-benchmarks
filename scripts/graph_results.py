#!/usr/bin/env python3
import os
from collections import defaultdict, namedtuple
import argparse
import re
import plotly


BenchmarkResults = namedtuple('BenchmarkResults', ['data', 'sizes_in_bytes', 'cardinalities'])


def main():
    argparser = argparse.ArgumentParser(description='Graphs the results of running the benchmarks')
    argparser.add_argument('--file', type=str, help='Path to the file that contains the benchmark output')
    argparser.add_argument('--min_elements', type=int, default=0, help='The minimum container size to graph; 0 for no minimum')
    argparser.add_argument('--max_elements', type=int, default=0, help='The maximum container size to graph; 0 for no maximum')
    args = argparser.parse_args()

    results = read_benchmark_results(args.file, args.min_elements, args.max_elements)
    graph_results(results, 'out')


def read_benchmark_results(file_path, min_elements=None, max_elements=None):
    """
    :type file_path str
    :type min_elements int|None
    :type max_elements int|None
    :rtype BenchmarkResults
    :return The parsed benchmark results file. The data member dict looks like this:
        {
            benchmark_function_str: {
                data_size_int: {
                   container_type_str: {
                        num_elements_int: cpu_time_nanoseconds
                    }
                }
            }
        }
        While the sizes_in_bytes and cardinalities members are sorted lists.
    """
    def data_type_to_size(data_type):
        if data_type == "int":
            return 4
        elif data_type == "size_16":
            return 16
        elif data_type == "size_64":
            return 64
        raise Exception("Unknown type " + data_type)

    # Regex for individual iterations of the benchmark
    # Group 1: benchmark function name, e.g., BM_vector_sequential_read
    # Group 2: container type, e.g., FixedArray<size_16>
    # Group 3: data type, e.g., int or size_16
    # Group 4: number of elements, between 4 and 16384
    # Group 5: clock time in ns
    # Group 6: CPU time in ns
    # Group 7: iteration count
    benchmark_re = re.compile(r"^(\w+)<([\w<>:, ]+), (\w+)>\/(\d+)\s+(\d+) ns\s+(\d+) ns\s+(\d+)$")

    data = defaultdict(lambda: defaultdict(lambda: defaultdict(lambda: defaultdict(int))))
    data_sizes = set()
    cardinalities = set()

    with open(file_path) as input:
        for line in input:
            match = benchmark_re.match(line)
            if match:
                benchmark_fn = match.group(1)
                container_type = match.group(2)
                data_size = data_type_to_size(match.group(3))
                num_elements = int(match.group(4))
                cpu_time = int(match.group(6))
                meets_min_requirements = not min_elements or num_elements >= min_elements
                meets_max_requirements = not max_elements or num_elements <= max_elements
                if meets_min_requirements and meets_max_requirements:
                    data[benchmark_fn][data_size][container_type][num_elements] = cpu_time
                    data_sizes.add(data_size)
                    cardinalities.add(num_elements)
    return BenchmarkResults(data=data, sizes_in_bytes=sorted(data_sizes), cardinalities=sorted(cardinalities))


def graph_results(benchmark_results, out_dir):
    """
    :type benchmark_results: BenchmarkResults
    :type out_dir str
    """
    try:
        os.makedirs(out_dir)
    except os.error:
        pass

    for benchmark_fn, data_sizes_for_fn in benchmark_results.data.items():
        for data_size, container_types_at_size in data_sizes_for_fn.items():
            # Make a graph of the time required of each container type with this data size and any number of elements
            traces = []
            for container_type, num_elements_for_container in container_types_at_size.items():
                times = []  # CPU time in nanoseconds
                for cardinality in benchmark_results.cardinalities:
                    times.append(num_elements_for_container[cardinality])
                traces.append(plotly.graph_objs.Scatter(
                    x=benchmark_results.cardinalities,
                    y=times,
                    mode='lines+markers',
                    name=container_type
                ))
            layout = plotly.graph_objs.Layout(
                title="%s() Time (at %d Byte Data Size) by Number of Elements" % (benchmark_fn, data_size),
                xaxis=dict(title='Number of Elements in the Container'),
                yaxis=dict(title='Time (nanoseconds)')
            )
            figure = plotly.graph_objs.Figure(data=traces, layout=layout)
            # plotly.offline.plot(figure,
            #                     filename="%s_data_size_%d.html" % (benchmark_fn, data_size),
            #                     auto_open=False)
            plotly.io.write_image(figure, os.path.join(out_dir, "%s_data_size_%d.png" % (benchmark_fn, data_size)))

        # We need separate graphs by container size.
        # E.g., if you know your container will have 8 elements, here's the fastest container for iteration.
        # If you know it'll have 1,000, loook at the other graph.
        # Graphs plot the time required by data size for each container


def debug_dump_data(data):
    for benchmark_fn, data_by_container_type in data.items():
        print(benchmark_fn + ":")
        for container_type, data_by_size in data_by_container_type.items():
            print("\t", container_type)
            for size, data_by_num_elements in data_by_size.items():
                print("\t\tData size", size)
                for elements, cpu_time_ns in data_by_num_elements.items():
                    print("\t\t\t", elements, ":", cpu_time_ns)


if __name__ == "__main__":
    main()
