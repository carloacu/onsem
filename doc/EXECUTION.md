# Execution


## Run from command line

The following command lines should be run from the root of the build folder.

### Execution of the debug gui

:warning: To have the micro working, look at the [Start the STT (Speech To Text)](#start-the-stt) heading.

```bash
./onsemgui/onsemgui --databases linguistic/databases --share_semantic ../share/semantic
```

### Execution of the tests

```bash
./tests/onsemgtests/onsemgtests --databases linguistic/databases --share_semantic ../share/semantic
```




## Run from Qt Creator IDE


### Arguments to set for onsemgui

The arguments have to be set here:<br />
Projects > Build & Run > Run<br />
Projects > Run > Run configuration > Select "onsemgui"<br />
Projects > Run > Command line arguments > Copy past the arguments below<br />

:warning: To have the micro working, look at the [Start the STT (Speech To Text)](#start-the-stt) heading.

```bash
--databases ../linguistic/databases --share_semantic ../../share/semantic
```


### Arguments to set for onsemgtests

The arguments have to be set here:<br />
Projects > Build & Run > Run<br />
Projects > Run > Run configuration > Select "onsemgtests"<br />
Projects > Run > Command line arguments > Copy past the arguments below<br />


```bash
--databases ../../linguistic/databases --share_semantic ../../../share/semantic
```

### Arguments to set for databasegeneratorprepocessinggui

The arguments have to be set here:<br />
Projects > Build & Run > Run<br />
Projects > Run > Run configuration > Select "onsemgtests"<br />
Projects > Run > Command line arguments > Copy past the arguments below<br />


```bash
--build_root ../.. --share_db ../../../share
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
* "(gdb) onsemgui" - to start onsemgui<br />
* "(gdb) onsemgtests" - to start onsemgtests<br />
<br />

Suggestion to have a nice theme:<br />
CTRL + SHIFT + P<br />
Click on open settings (JSON)<br />
then add that<br />
    "workbench.colorTheme": "Visual Studio Dark - C++"<br />






