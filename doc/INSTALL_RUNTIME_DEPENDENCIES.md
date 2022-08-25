# Install runtime dependencies


## Install dot

It is to print the structure of the analyzed texts.

```bash
sudo apt-get install graphviz
```


## Install vosk

It is for the Speech To Text in the gui. To make it run, look at the [Start the STT (Speech To Text)](#start-the-s>

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
