#!/usr/bin/env python3
from collections import defaultdict
import argparse
import re
import plotly
import plotly.graph_objs as go


def main():
    argparser = argparse.ArgumentParser(description='Graphs the results of running the benchmarks')
    argparser.add_argument('--file', type=str, help='Path to the file that contains the benchmark output')
    argparser.add_argument('--min_elements', type=int, default=0, help='The minimum container size to graph; 0 for no minimum')
    argparser.add_argument('--max_elements', type=int, default=0, help='The maximum container size to graph; 0 for no maximum')
    args = argparser.parse_args()

    # Regex for individual iterations of the benchmark
    # Group 1: benchmark function name, e.g., BM_vector_sequential_read
    # Group 2: container type, e.g., FixedArray<size_16>
    # Group 3: data type, e.g., int or size_16
    # Group 4: number of elements, between 4 and 16384
    # Group 5: clock time in ns
    # Group 6: CPU time in ns
    # Group 7: iteration count
    benchmark_re = re.compile(r"^(\w+)<([\w<>, ]+), (\w+)>\/(\d+)\s+(\d+) ns\s+(\d+) ns\s+(\d+)$")

    # Structure:
    # {
    #     benchmark_function_str: {
    #         data_size_int: {
    #            container_type_str: {
    #                 num_elements_int: cpu_time_nanoseconds
    #             }
    #         }
    #     }
    # }
    data = defaultdict(lambda: defaultdict(lambda: defaultdict(lambda: defaultdict(int))))
    data_sizes = set()
    cardinalities = set()

    with open(args.file) as input:
        for line in input:
            match = benchmark_re.match(line)
            if match:
                benchmark_fn = match.group(1)
                container_type = match.group(2)
                data_size = data_type_to_size(match.group(3))
                num_elements = int(match.group(4))
                cpu_time = int(match.group(6))
                meets_min_requirements = not args.min_elements or num_elements >= args.min_elements
                meets_max_requirements = not args.max_elements or num_elements <= args.max_elements
                if meets_min_requirements and meets_max_requirements:
                    data[benchmark_fn][data_size][container_type][num_elements] = cpu_time
                    data_sizes.add(data_size)
                    cardinalities.add(num_elements)

        data_sizes = sorted(data_sizes)
        cardinalities = sorted(cardinalities)

        for benchmark_fn, data_sizes_for_fn in data.items():
            for data_size, container_types_at_size in data_sizes_for_fn.items():
                # Make a graph of the time required of each container type with this data size and any number of elements
                traces = []
                for container_type, num_elements_for_container in container_types_at_size.items():
                    times = []  # CPU time in nanoseconds
                    for cardinality in cardinalities:
                        times.append(num_elements_for_container[cardinality])
                    traces.append(go.Scatter(
                        x=cardinalities,
                        y=times,
                        mode='lines+markers',
                        name=container_type
                    ))
                layout = go.Layout(
                    title="%s() Time (at %d Byte Data Size) by Number of Elements" % (benchmark_fn, data_size),
                    xaxis=dict(title='Number of Elements'),
                    yaxis=dict(title='Time (nanoseconds)')
                )
                figure = go.Figure(data=traces, layout=layout)
                plotly.offline.plot(figure,
                                    filename=benchmark_fn + '_' + 'data_size_' + str(data_size),
                                    auto_open=False)

            # We need separate graphs by container size.
            # E.g., if you know your container will have 8 elements, here's the fastest contianer for iteration.
            # If you know it'll have 1,000, loook at the other graph.
            # Graphs plot the time required by data size for each container


def data_type_to_size(data_type):
    if data_type == "int":
        return 4
    elif data_type == "size_16":
        return 16
    elif data_type == "size_64":
        return 64
    raise Exception("Unknown type " + data_type)


def debug_dump_data(data):
    for benchmark_fn, data_by_container_type in data.items():
        print(benchmark_fn + ":")
        for container_type, data_by_size in data_by_container_type.items():
            print("\t", container_type)
            for size, data_by_num_elements in data_by_size.items():
                print("\t\tData size", size)
                for elements, cpu_time_ns in data_by_num_elements.items():
                    print("\t\t\t", elements, ":", cpu_time_ns)


main()
