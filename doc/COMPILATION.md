# Compilation


## Command line for databases compilation

:warning: This step is mandatory before the compilation. It should be done in the same build folder tha>

```bash
cmake -B build -DBUILD_ONSEM_DATABASE=ON -DCMAKE_BUILD_TYPE=Release ./ && make -C build -j4
```


## Command line for compilation

```bash
cmake -B build -DBUILD_ONSEM_DATABASE=OFF -DCMAKE_BUILD_TYPE=Debug -DBUILD_ONSEM_TESTS=ON -DBUILD_DB_GENERATOR_PREPROCESSING=ON ./ && make -C build -j4
```


