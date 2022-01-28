#!/usr/bin/env python

import sys 
import re
import traceback
from optparse import OptionParser


#depPath=sys.argv[1]
srcPath=sys.argv[2]

path=re.sub('\..*','',srcPath)
#print "#.d=%s,.s=%s  path=%s"%(depPath,srcPath,path)


row=sys.stdin.readline().rstrip()

(objFile,srcPathes)=re.split(r':',row)

#print "#%s , %s "%(objFile,srcPathes)
print "%s.o:%s"%(path,srcPathes)
#print row
for row in sys.stdin:
    row=row.rstrip()
    print row
