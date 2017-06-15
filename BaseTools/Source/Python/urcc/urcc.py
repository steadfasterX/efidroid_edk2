## @file
# generate uefi resource file
#
#  Copyright (c) 2017, The EFIDroid Project<BR>
#
#  This program and the accompanying materials
#  are licensed and made available under the terms and conditions of the BSD License
#  which accompanies this distribution.  The full text of the license may be found at
#  http://opensource.org/licenses/bsd-license.php
#
#  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#

##
# Import Modules
#
from optparse import OptionParser
import sys
import os
import struct
import xml.etree.ElementTree as ET
from Common.Misc import PathClass
from Common.String import NormPath
from Common import EdkLogger
from Common.BuildVersion import gBUILD_VERSION
from Common.MultipleWorkspace import MultipleWorkspace as mws
from Common.BuildToolError import *

## Version and Copyright
versionNumber = "1.0" + ' ' + gBUILD_VERSION
__version__ = "%prog Version " + versionNumber
__copyright__ = "Copyright (c) 2017, The EFIDroid Project"

## Tool entrance method
#
# This method mainly dispatch specific methods per the command line options.
# If no error found, return zero value so the caller of this tool can know
# if it's executed successfully or not.
#
#   @retval 0     Tool was successful
#   @retval 1     Tool failed
#
def main():
    global Options
    Options = myOptionParser()

    EdkLogger.Initialize()

    if Options.compile:
        urc = Urc(PathClass(NormPath(Options.input)))
        urc.compile(PathClass(NormPath(Options.output)))
        return 0

    return 1

class CopyItem:
    TYPE_FILE = 0
    TYPE_DIRECTORY = 1

    def __init__(self, UrcFile, child):
        # get source file path
        self.src = os.path.abspath(os.path.join(UrcFile.Dir, child.get('from')))
        if not os.path.exists(self.src):
            EdkLogger.error("build", FILE_OPEN_FAILURE, "%s used by %s was not found" % (self.src, UrcFile), 
                            ExtraData=self.src + "\n\t")

        if os.path.isfile(self.src):
            self.type = CopyItem.TYPE_FILE
        elif os.path.isdir(self.src):
            self.type = CopyItem.TYPE_DIRECTORY

        # get destination file path
        self.dst = child.get('to')
        self.dst_segments = self.dst.split('/')

        # validate destination file path
        if not self.dst.startswith('/'):
            EdkLogger.error("build", FILE_PARSE_FAILURE, "path doesn't start with '/': %s" % (self.dst),
                            ExtraData=UrcFile.Path + "\n\t")
        if self.dst.endswith('/') and len(self.dst)!=1:
            EdkLogger.error("build", FILE_PARSE_FAILURE, "path ends with '/': %s" % (self.dst),
                            ExtraData=UrcFile.Path + "\n\t")
        if '.' in self.dst_segments:
            EdkLogger.error("build", FILE_PARSE_FAILURE, "path contains '.': %s" % (self.dst),
                            ExtraData=UrcFile.Path + "\n\t")
        if '..' in self.dst_segments:
            EdkLogger.error("build", FILE_PARSE_FAILURE, "path contains '..': %s" % (self.dst),
                            ExtraData=UrcFile.Path + "\n\t")
        if '\\' in self.dst:
            EdkLogger.error("build", FILE_PARSE_FAILURE, "path contains '\\': %s" % (self.dst),
                            ExtraData=UrcFile.Path + "\n\t")

class FileItem:
    def __init__(self, src, dst):
        self.src = src
        self.dst = dst

    def __str__(self):
        return "['%s' -> '%s']" % (self.src, self.dst)

    def __repr__(self):
        return "['%s' -> '%s']" % (self.src, self.dst)

class Urc:
    @staticmethod
    def __align(a, b):
        return (((a) + (b) - 1) / (b)) * (b)

    @staticmethod
    def __zeropad(f, length):
        for i in range (0, Urc.__align(length, 4)-length):
            f.write(struct.pack('B', 0x00))

    def __init__(self, File):
        try:
            xtree = ET.parse(File.Path)
        except BaseException, X:
            EdkLogger.error("build", FILE_PARSE_FAILURE, ExtraData=File.Path + "\n\t" + str(X))
        self.xroot = xtree.getroot()

        self.symbol_name = 'main_resources'
        attr_symbol_name = self.xroot.get('symbol_name')
        if attr_symbol_name:
            self.symbol_name = attr_symbol_name

        self.copyitems = []
        self.dependencies = []
        self.fileitems = []
        for child in self.xroot:
            if child.tag == 'copy':
                copyitem = CopyItem(File, child)
                self.copyitems.append(copyitem)

                if copyitem.type == CopyItem.TYPE_FILE:
                    self.fileitems.append(FileItem(copyitem.src, copyitem.dst))
                    self.dependencies.append(copyitem.src)

                elif copyitem.type == CopyItem.TYPE_DIRECTORY:
                    srcpath_len = len(copyitem.src)
                    for dirpath, dirnames, filenames in os.walk(copyitem.src, followlinks=True):
                        self.dependencies.append(dirpath)

                        for filename in filenames:
                            self.dependencies.append(os.path.join(dirpath, filename))

                            srcpathrel = dirpath[srcpath_len:]
                            if srcpathrel.startswith('/'):
                                srcpathrel = srcpathrel[1:]
                            self.fileitems.append(FileItem(os.path.join(dirpath, filename), os.path.join(copyitem.dst, srcpathrel, filename)))

    def getDependencies(self):
        return self.dependencies

    def compile(self, OutputFile):
        urcb_filename = os.path.splitext(OutputFile.Path)[0]+'.urcb'

        # generate urcb
        with open(urcb_filename, 'wb') as f:
            # write header
            f.write(struct.pack('<4cI',
                'U','R','C','C',
                len(self.fileitems)
            ))

            for fileitem in self.fileitems:
                filename_len = len(fileitem.dst)+1
                filesize = os.path.getsize(fileitem.src)

                # write header
                f.write(struct.pack('<4cQQ',
                    'U','R','C','F',
                    filename_len, filesize
                ))

                # write filename
                f.write(fileitem.dst)
                f.write(struct.pack('B', 0x00))
                Urc.__zeropad(f, filename_len)

                # write data
                with open(fileitem.src, 'rb') as fdata:
                    f.write(fdata.read())
                Urc.__zeropad(f, filesize)

        # generate assembler
        with open(OutputFile.Path, 'w') as f:
            f.write('.section .rodata\n')
            f.write('.align 4\n')
            f.write('.global %s\n' % self.symbol_name)
            f.write('%s:\n' % self.symbol_name)
            f.write('.incbin "%s"\n' % urcb_filename)

## Parse command line options
#
# Using standard Python module optparse to parse command line option of this tool.
#
#   @retval Opt   A optparse.Values object containing the parsed options
#   @retval Args  Target of build command
#
def myOptionParser():
    Parser = OptionParser(description=__copyright__, version="%prog " + str(versionNumber))
    Parser.add_option("-o", "--output", dest="output", type="string", help="Output file path")
    Parser.add_option("-c", "--compile", dest="compile", action="store_true", type=None)
    (Options, args) = Parser.parse_args()
    if len(args) != 1:
        Parser.error("incorrect number of arguments")
    Options.input = args[0]
    return Options

if __name__ == '__main__':
    r = main()
    ## 0-127 is a safe return range, and 1 is a standard default error
    if r < 0 or r > 127: r = 1
    sys.exit(r)
