#!/usr/bin/env python
import re

filterList=(r'^#include.*linux',r'^#include.*map.h',r'^#include.*sizes.h',r'^typedef asmlinkage',r'^EXPORT_SYMBOL','^extern const struct map_desc')



def checkRemove(line):
	for fs in filterList:
		if re.search(fs,line):
			return True;
	return False;


def processFile(name):

	infile=open(name)
	outStr=""

	for line in infile:
		if re.search(r'#include.*spec.h', line):
			line = r'#include "spec.h"  //SoFia' + '\n'
		elif checkRemove(line):
			line = r"//SoFia: " + line
		outStr+=line

	#multiline replace
	outStr=re.sub(r'\n(const struct map_desc platform_io_desc(.*\n)*?.*?;)', r'\n/*SoFia: \1*/', outStr)

	infile.close()

	outfile=open(name,'w')
	outfile.write(outStr)
	outfile.close
        

processFile("spec.h")
processFile("spec.c")
