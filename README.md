# protoc-plugin-examples

Simple protoc plugin examples for learning and demonstration.
See different branches for different examples.

## How To Build

```sh
# ...after cloning...
(protoc-plugin-examples)$ mkdir build && cd build
(build)$ cmake -DCMAKE_INSTALL_PREFIX=. ..
(build)$ make install
```

## How To Run

Building creates an executable at that can be run to show the basic proof-of-concept output from the plugin(s).

```sh
(build)$ ./main
```
