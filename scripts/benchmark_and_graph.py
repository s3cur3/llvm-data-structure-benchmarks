import argparse
import contextlib
import os
import subprocess
from graph_results import parse_benchmark_results, graph_results




def main():
    argparser = argparse.ArgumentParser(description='Builds & runs the benchmarks, saves the output to a text file, and produces a number of graphs')
    argparser.add_argument('-f', '--file', type=str, default=None, help='true if you want to use previously saved output; otherwise, we\'ll build and run the benchmark "live"')
    args = argparser.parse_args()

    if args.file:
        with open(args.file) as results_file:
            results_raw = results_file.readlines()
    else:
        results_raw = build_and_run()
        with open('output.txt', 'w') as out_file:
            out_file.writelines([line + '\n' for line in results_raw])

    for max_elements in [64, 256, 1024, 16384]:
        graph_results(
            parse_benchmark_results(results_raw, None, max_elements),
            os.path.join('graphs', "container_size_up_to_%d" % max_elements))


def build_and_run():
    with friendly_cd_to_repo_root():
        subprocess.check_call(['cmake', 'CMakeLists.txt'])
        subprocess.check_call(['make'])
        benchmark_output_bytes = subprocess.check_output(['./llvm_data_structure_benchmarks', '--benchmark_color=false'])
        return benchmark_output_bytes.decode('utf-8').splitlines()


@contextlib.contextmanager
def friendly_cd_to_repo_root(additional_path=None):
    """
    A context manager for changing directories to some specific subdirectory
    of this repo.
    :type additional_path: str|None

    >>> prev_path = os.getcwd()
    >>> with friendly_cd_to_repo_root('scripts'):
    ...     assert os.getcwd().endswith(os.path.join('llvm-data-structure-benchmarks', 'scripts'))
    >>> assert prev_path == os.getcwd()
    """
    def _cd_up_until_file_path_exists(file_to_check_for, max_levels=3):
        for i in range(max_levels):
            if not file_to_check_for in os.listdir(os.getcwd()):
                os.chdir('..')
        assert file_to_check_for in os.listdir(os.getcwd()), 'It looks like you didn\'t run this script from an expected location'

    prev_dir = os.getcwd()
    try:
        _cd_up_until_file_path_exists('data_structure_benchmarks.cpp')
        if additional_path:
            os.chdir(additional_path)
        yield
    finally:
        os.chdir(prev_dir)


if __name__ == "__main__":
    main()
