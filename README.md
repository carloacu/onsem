onsem
=====

NLP C++ tools



Install compilation dependencies
--------------------------------

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

:warning: This step is mandatory before the compilation. It should be done in the same build folder than the compilation.

```bash
cmake -DBUILD_ONSEM_DATABASE=ON ../ && make -j4
```


### Command line for compilation

```bash
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_ONSEM_DATABASE=OFF -DBUILD_ONSEM_TESTS=ON ../ && make -j4
```



Install runtime dependencies
----------------------------

### Install dot

```bash
sudo apt-get install graphviz
```

### Install vosk

```bash
pip3 install vosk
```




Execution (from command line)
-----------------------------

The following command lines should be run from the root of the build folder.

### Execution of the debug gui

:warning: To have the micro working [go to the "Start the STT" chapter](#start-the-stt-(speech-to-text))

```bash
./voicebotgui/voicebotgui --databases linguistic/databases --share_semantic ../share/semantic
```

### Execution of the tests

```bash
./tests/gtests/semanticreasoner_gtests --databases linguistic/databases --share_semantic ../share/semantic
```





Execution (from Qt Creator IDE)
-------------------------------


### Arguments to set for voicegui

The arguments have to be set here:<br />
Projects > Build & Run > Run<br />
Projects > Run > Run configuration > Select "voicegui"<br />
Projects > Run > Command line arguments > Copy past the arguments below<br />

```bash
--databases ../linguistic/databases --share_semantic ../../share/semantic
```


### Arguments to set for semanticreasoner_gtests

The arguments have to be set here:<br />
Projects > Build & Run > Run<br />
Projects > Run > Run configuration > Select "semanticreasoner_gtests"<br />
Projects > Run > Command line arguments > Copy past the arguments below<br />


```bash
--databases ../../linguistic/databases --share_semantic ../../../share/semantic
```



#Start the STT (Speech To Text)
-------------------------------

To have the "micro" button working in the gui, you need to start the Speech To Text either
in french or in english but not in both languages at the same time.

The following command lines should be run from the repository root folder.


### To start it in french

```bash
sh run_stt.sh fr
```

### To start it in english

```bash
sh run_stt.sh en
```


