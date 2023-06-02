VAGRANT_BOX_MEMORY = 4096
VAGRANT_BOX_CPUS = 4

Vagrant.configure("2") do |config|
  
  # Ubuntu 18
  config.vm.box = "ubuntu/bionic64"

  # Set the Vagrant box's RAM
  config.vm.provider :virtualbox do |vb|
    vb.memory = VAGRANT_BOX_MEMORY
    vb.cpus = VAGRANT_BOX_CPUS
  end

  # Add to the vagrant box's provisioning script: install dependencies for weenix
  config.vm.provision :shell, :inline => %Q{
    sudo apt-get update
    sudo apt-get install python2.7 python-minimal cscope nasm make build-essential grub2-common qemu xorriso genisoimage xterm gdb python3-pip -y
    pip3 install pyelftools
    cat /vagrant/weenix-scripts.txt >> /home/vagrant/.bashrc
    source /home/vagrant/.bashrc
  }
end
