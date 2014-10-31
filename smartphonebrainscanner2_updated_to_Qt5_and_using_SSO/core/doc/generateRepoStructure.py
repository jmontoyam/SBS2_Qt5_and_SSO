import os
import sys
import datetime

level_diff = 2

maxLevel = dict()

maxLevel["core"] = 999
maxLevel["apps"] = 2

def printLevel(name, level, fullpath):
	global level_diff
	for i in range(level*level_diff): print "",
	print "* *%s*"%name
	try:
		desc = open(fullpath+"/"+"README").read().strip()
		if len(desc) > 100:
			desc = desc[:desc[0:100].rfind(" ")]+" ..."
		for i in range(level*level_diff): print "",
		print "_"+desc+"_"
	except IOError:
		pass
		
def printDirs(name, level, simpleName):
	global maxLevel
	try:
		if maxLevel[simpleName] < level:
			return
	except KeyError:
		pass
	dirs = os.listdir(name)
	for dir in dirs:
		origDir = dir
		dir = name + "/" + dir
		if origDir[0] == '.':
			continue
		if not os.path.isdir(dir):
			continue
		if level == 1: simpleName = origDir
		printLevel(origDir, level, dir)
		printDirs(dir, level+1, simpleName)

print "Generated using %s on %.19s from %s"%(sys.argv[0], datetime.datetime.now(), open(".git/FETCH_HEAD").read().split("\t")[2].strip())
print ""
printDirs(".", 1, ".")

