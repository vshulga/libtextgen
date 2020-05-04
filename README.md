# Welcome to libtextgen!

### Overview

The libtextgen project develops a portable, efficient C++14 library that can generate texts using Markov chain algorithm. It uses [curl](https://curl.haxx.se) command line tool for text downloading. The library was successfully built/tested with Visual Studio 14, GCC 7.5.0, Clang 11.0.0 for x86_64 platform.

### Getting the Source Code and Building/Testing libtextgen

1. Download or clone the repository:
    * `git clone https://github.com/vshulga/libtextgen.git`

2. Configure and build the project:
    * `cd libtextgen`
    * `mkdir build`
    * `cd build`
    * `cmake [-G <generator>] ..`
    * `cmake --build . [--config <config>]`
    
3. Test the project:
    * `ctest [--build-config <config>]`
    
### Examples

1. Print help:
    ```
    textgen -h
    Usage: textgen [options] ...
    Options:
        -c      download concurrency (1000 by default)
        -g      generate text from model (0 by default)
        -h      print help (0 by default)
        -i      input file (stdin by default)
        -l      global locale (user-preferred by default)
        -n      text prefix length (1 by default)
        -o      output file (stdout by default)
        -p      generated text prefix
        -r      word regex (\w+ by default)
        -t      train model from text (0 by default)
        -w      generated text size (1000000 by default)
    ```
2. Train model:
    ```
    textgen -t -l en_US.UTF-8 -o war_and_peace.model https://www.gutenberg.org/files/2600/2600-0.txt
    ```
    There should be en_US.UTF-8 locale in the system of course.
    
3. Generate text:
    ```
    textgen -g -i war_and_peace.model
    ```
    There is no need to specify the locale since the text is generated as a stream of bytes.
    
4. Train model and generate text:
    ```
    textgen -t -g -l en_US.UTF-8 https://www.gutenberg.org/files/2600/2600-0.txt
    ```
    
5. Train model and generate text using pipeline:
    ```
    textgen -t -l en_US.UTF-8 https://www.gutenberg.org/files/2600/2600-0.txt | textgen -g
    ```
    
6. Download two files for training and generate text:
    ```
    textgen -t -g -l en_US.UTF-8 https://www.gutenberg.org/files/2600/2600-0.txt https://www.gutenberg.org/files/14741/14741-0.txt
    ```
    
7. Train model and generate text with custom options:
    ```
    textgen -t -g -n 2 -r \S+ -c 1 -p "Prince Andrew" -w 10 -l en_US.UTF-8 https://www.gutenberg.org/files/2600/2600-0.txt https://www.gutenberg.org/files/14741/14741-0.txt
    the feeling he had left for you will agree to 
    ```
    
    * download two files, one file at once (-c 1)
    * extract words using "non space" regex (-r \\S+)
    * 2-word prefixes (-n 2)
    * generate starting from "Prince Andrew" prefix
    * generate maximum 10 words (-w 10)
