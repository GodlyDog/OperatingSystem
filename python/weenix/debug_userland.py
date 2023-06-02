from elftools.elf.elffile import ELFFile
from os import path

class DebugUserland(gdb.Command):
    def __init__(self):
      super(DebugUserland, self).__init__("debug-userland", gdb.COMMAND_USER)

    def invoke(self, arg, from_tty):
        directory = 'user/usr/bin/'
        filename = directory + arg + '.exec'
        if not path.exists(filename):
            filename = 'user/bin/' + arg + '.exec'
        if arg == 'init':
            filename = 'user/sbin/init.exec'
        elf = ELFFile(open(filename, 'rb'))
        text_section = elf.get_section_by_name('.text')
        entry = text_section.header.sh_addr
        gdb.execute(f"add-symbol-file {filename} {entry}")

        symtab = elf.get_section_by_name('.symtab')
        main = symtab.get_symbol_by_name('main')[0]
        main_addr = main.entry.st_value
        gdb.execute(f"break *{main_addr}")
        
DebugUserland()
