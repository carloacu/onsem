onsem
=====

NLP C++ tools



Install dependencies
--------------------

### Install boost

```bash
sudo apt-get install libboost-all-dev
```


### Install gtest

Look at readme/install_gtest.txt for more details

```bash
sudo apt-get install libgtest-dev
```



Compilation
-----------


### Create and go inside a build folder

```bash
mkdir ${ONSEM_ROOT}/build
cd ${ONSEM_ROOT}/build
```


### Command line for databases compilation

```bash
cmake -DBUILD_ONSEM_DATABASE=ON ../ && make -j4
```


### Command line for compilation

```bash
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_ONSEM_DATABASE=OFF -DBUILD_ONSEM_TESTS=ON ../ && make -j4
```

Execution (from command line)
-----------------------------


### Execution of the debug gui

```bash
./voicebotgui/voicebotgui --databases linguistic/databases --share_semantic ../share/semantic
```


### Execution of the tests

```bash
./tests/gtests/semanticreasoner_gtests --databases linguistic/databases --share_semantic ../share/semantic
```





Execution (from Qt Creator IDE)
-------------------------------

### Execution of the debug gui

```bash
./voicebotgui --databases ../linguistic/databases --share_semantic ../../share/semantic
```


### Execution of the tests

```bash
./semanticreasoner_gtests --databases ../../linguistic/databases --share_semantic ../../../share/semantic
```

