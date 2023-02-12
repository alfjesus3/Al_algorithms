## gpr-cpp-toy

### Goals
- [ ] Adapting the sparse gaussian processes regression toy problem from [here](https://krasserm.github.io/2020/12/12/gaussian-processes-sparse/);
- [ x ] Use the [Limbo](https://github.com/resibots/limbo) C++ library for B.O. and GPr;
- [ ] try in the more complex regression problem -> e.g. multivariable sensory data


### Notes
* Tested in WSL2 ubuntu 20.04
* To integrate with Cmake I referred to the following issues: [1st](https://github.com/resibots/limbo/issues/259) and [2nd](https://github.com/resibots/limbo/issues/313);
* Additionally, the [Limbo compilation tutorial](http://resibots.eu/limbo/tutorials/compilation.html) was used; 


## Building and running
```
# Firstly install all the dependencies
source install_dependencies.sh
# Build and make
mkdir -p build
cd build
cmake .. -DUSE_NLOPT="../deps/nlopt" -DUSE_LIBCMAES="../deps/libcmaes"
make -j8
./my_limbo_test
```


### TODOs
* fix issue compilation with `limbo` external flags not being recognized -> https://stackoverflow.com/questions/25533831/passing-cmake-variables-to-externalproject-add