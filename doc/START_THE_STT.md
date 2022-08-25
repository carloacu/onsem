# Start the STT (Speech To Text)


The following command lines should be run from this repository root folder.

To have the "micro" button working in the gui, you need to start the Speech To Text either
in french or in english but not in both languages at the same time.



## Before to start any Speech to text, install vosk and download the STT models

```bash
pip3 install vosk
```

Download english model [here](https://drive.google.com/file/d/1sVQ_odyDPJcW6-FdsOHjeN2dTVcdmR>
french model [here](https://drive.google.com/file/d/1hm96XGHsUcnt5A9Iuyl34UYt-QdyfCEF/view?us>
in stt/models folder.


## To start the STT in french

```bash
sh run_stt.sh fr
```

## To start the STT in english

```bash
sh run_stt.sh en
```
