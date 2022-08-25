# Execution


## Run from command line

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




## Run from Qt Creator IDE


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



## Run from VsCode


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






