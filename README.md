# Installation

This file contains quick instructions for getting Weenix to run on
Redhat-derived or Debian-derived Linux flavors. If you're using a virtual machine with the Weenix Vagrantfile, the dependencies should be installed automatically when the machine is provisioned.

See also [Getting Started with Weenix](https://github.com/brown-cs1690/handout/wiki/Getting-Started-with-Weenix) for more thorough documentation.

1. Download and install dependencies.

   On recent versions of Ubuntu or Debian, you can simply run:

   ```bash
   $ sudo apt-get install git-core build-essential gcc gdb qemu genisoimage make python python-argparse cscope xterm bash grub xorriso
   ```

   or on Redhat:

   ```bash
   $ sudo yum install git-core gcc gdb qemu genisoimage make python python-argparse cscope xterm bash grub2-tools xorriso
   ```

2. Compile Weenix:

   ```bash
   $ make
   ```

3. Invoke Weenix:

   ```bash
   $ ./weenix -n
   ```

   or, to run Weenix under gdb, run:

   ```bash
   $ ./weenix -n -d gdb
   ```
   You may also need to install `pyelftools`, to do so, make sure that you have pip3 installed. Once you have that installed, you can run `pip3 install pyelftools`. 