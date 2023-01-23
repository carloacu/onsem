# Compilation


## Clone sub modules

```bash
cd ${ONSEM_ROOT}
git submodule update --init --recursive
```

## Create and go inside a build folder

```bash
mkdir ${ONSEM_ROOT}/build
cd ${ONSEM_ROOT}/build
```


## Command line for databases compilation

:warning: This step is mandatory before the compilation. It should be done in the same build folder tha>

```bash
cmake -DBUILD_ONSEM_DATABASE=ON ../ && make -j4
```


## Command line for compilation

```bash
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_ONSEM_DATABASE=OFF -DBUILD_ONSEM_TESTS=ON -DBUILD_DB_GENERATOR_PREPROCESSING=ON ../ && make -j4
```


