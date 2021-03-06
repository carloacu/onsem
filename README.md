onsem
=====


This repository gather some Natural Language Processing (NLP) tools written in C++.<br />
To test it, launch the executable viocebotgui.

The project is also available on Android: https://github.com/carloacu/onsem-android


Install compilation dependencies
--------------------------------

### Install boost

```bash
sudo apt-get install libboost-all-dev
```


### Install gtest

```bash
sudo apt-get install libgtest-dev
```

### Insall cmake

```bash
sudo apt-get install cmake
```

### Install g++

```bash
sudo apt-get install build-essential
```

### Install qt5

It is usefull if you enable BUILD_ONSEM_TESTS. It allows to compile the tests and the gui.

```bash
sudo apt-get install qtbase5-dev
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

It is to print the structure of the analyzed texts.

```bash
sudo apt-get install graphviz
```

### Install vosk

It is for the Speech To Text in the gui. To make it run, look at the [Start the STT (Speech To Text)](#start-the-stt) heading.

```bash
pip3 install vosk
```

If you need to install pip3, run the command

```bash
sudo apt-get -y install python3-pip
```

And here are some dependancies for vosk

```bash
pip install sounddevice
sudo apt-get install libportaudio2
```




Execution (from command line)
-----------------------------

The following command lines should be run from the root of the build folder.

### Execution of the debug gui

:warning: To have the micro working, look at the [Start the STT (Speech To Text)](#start-the-stt) heading.

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

:warning: To have the micro working, look at the [Start the STT (Speech To Text)](#start-the-stt) heading.

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


Execution (from VsCode)
-----------------------

To install vscode on linux:
```bash
sudo apt update
sudo apt-get install build-essential gdb g++
sudo apt-get install snap
sudo snap install --classic code
```

2 configuations:<br />
* "(gdb) voicebotgui" - to start voicebotgui<br />
* "(gdb) semanticreasoner_gtests" - to start semanticreasoner_gtests<br />
<br />

Suggestion to have a nice theme:<br />
CTRL + SHIFT + P<br />
Click on open settings (JSON)<br />
then add that<br />
    "workbench.colorTheme": "Visual Studio Dark - C++"<br />


<span id="start-the-stt"></span>
Start the STT (Speech To Text)
------------------------------

The following command lines should be run from this repository root folder.

To have the "micro" button working in the gui, you need to start the Speech To Text either
in french or in english but not in both languages at the same time.


### Before to start any Speech to text, install vosk and download the STT models

```bash
pip3 install vosk
```

Download english model [here](https://drive.google.com/file/d/1sVQ_odyDPJcW6-FdsOHjeN2dTVcdmRiC/view?usp=sharing) and
french model [here](https://drive.google.com/file/d/1hm96XGHsUcnt5A9Iuyl34UYt-QdyfCEF/view?usp=sharing) and put them
in stt/models folder.


### To start the STT in french

```bash
sh run_stt.sh fr
```

### To start the STT in english

```bash
sh run_stt.sh en
```


