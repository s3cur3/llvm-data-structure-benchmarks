# llvm-data-structure-benchmarks

A benchmark for cache efficient data structures.

Full writeup (explaining the data structures being examined, as well as a discussion of the results) can be [found on my blog](https://tylerayoung.com/?p=1277).

## Compiling & running the easy way

`scripts` contains a Python script that can automate the entire process of building, running, and graphing the output. To use it, you'll need to (once) set up a Python virtual environment like so:

1. `$ cd <path to repo>/scripts`
2. `$ virtualenv env -p python3`
3. `$ env/bin/pip3 install -r package_requirements.txt`

Then, run the script via:
`$ env/bin/python3 benchmark_and_graph.py`

After the program churns for a few minutes, you'll get a new `graphs` directory full of PNGs.

## Compiling & running the hard way

1. `$ cmake CMakeLists.txt`
2. `$ make`
3. `./data_structure_benchmarks > my_output_file.txt`
4. Set up a Python virtual environment in your `scripts` directory as above
5. `$ cd scripts`
6. `$ env/bin/python3 graph_results.py --file my_output_file.txt [--min-elements=N] [--max-elements=M]`
