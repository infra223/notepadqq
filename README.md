# <img src="https://user-images.githubusercontent.com/4319621/36906314-e3f99680-1e35-11e8-90fd-f959c9641f36.png" alt="Notepadqq" width="32" height="32" /> Notepadqq

### Links

* [What is it?](#what-is-it)
* [Build it yourself](#build-it-yourself)

#### What is it?

Notepadqq is a text editor designed by developers, for developers.

*This fork of Notepadqq replaces the JavaScript-centric CodeMirror editor with a native component. It is faster and less resource-consuming than CodeMirror, but still experimental.*

*Please help me to improve this program by reporting any bugs on the Issues page.*

![screenshot_20180302_163505](https://notepadqq.com/s/images/snapshot1.png)

Please visit our [Wiki](https://github.com/notepadqq/notepadqq/wiki) for more screenshots and details.

Build it yourself
-----

| Build dependencies    | Dependencies      |
|-----------------------|-------------------|
| Qt 5.6 or higher      | Qt 5.6 or higher  |
| libqt5svg5-dev        | libqt5svg5        |
| qttools5-dev-tools    | coreutils         |
| libuchardet-dev       | libuchardet       |

#### Get the source

    $ git clone https://github.com/JuBan1/notepadqq.git
    $ cd notepadqq
    $ git checkout ote

#### Build

    notepadqq$ ./configure --prefix /usr
    notepadqq$ make
    
If you encounter errors make sure to have the necessary libraries installed. For Ubuntu you can do that using apt-get:

    notepadqq$ sudo apt-get install qt5-default qttools5-dev-tools libqt5svg5 libqt5svg5-dev libuchardet-dev

For CentOS:

    notepadqq$ sudo yum install -y qt5-qtbase-devel qt5-qttools-devel qt5-qtsvg-devel uchardet
    
#### Install

You can run notepadqq from its build output folder. If however you want to install it, first build it
by following the above steps, then run:

    notepadqq$ sudo make install

#### Qt

If the newest version of Qt isn't available on your distribution, you can use the [online installer](http://www.qt.io/download-open-source) to get the latest libraries and install them into your home directory (`$HOME/Qt`). Notepadqq will automatically use them.
