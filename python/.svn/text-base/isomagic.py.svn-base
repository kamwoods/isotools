#A PYTHON FILE FOR EXTRACTING FILE MAGIC NUMBERS FROM AN IS09660 DISK IMAGE

import os, sys
sys.path.insert(0, '/u/sjhoward/tmp/pycdio-0.13')
import pycdio
import iso9660
import stat
import magic

def ceil(x): return int(round(x+0.5)) 

def bft_inner (iso, top = '/', depthfirst = False):
	nodes = iso.readdir(top)
	if not depthfirst:
		yield top, nodes
	for node in nodes:
		st = iso.stat(top + node[0])
		if st['is_dir']:
			if not node[0] == '.' and not node[0] == '..':
				for (newtop, children) in bft_inner (iso, top + node[0] + '/', depthfirst):
					yield newtop, children

def bft (iso, topdir):
	flist = []
	for (basepath, children) in bft_inner(iso, topdir, False):
		for child in children:
			if not iso.stat(basepath + child[0])['is_dir']:
				flist.append(basepath + child[0])
	return flist				

def isomagic_file_info (iso, iso_filename):
	if not iso.is_open():
		print "\nError...could not open image as an ISO96660 image.\n"
		sys.exit(1)
	statbuf = iso.stat(iso_filename, True)
	if statbuf is None:
		print "Error...could not get file info for %s." % iso_filename
		iso.close()
		sys.exit(2)
	size, buf = iso.seek_read(statbuf['LSN'])
	
	ms = magic.open(magic.MAGIC_NONE)
	ms.load()
	type = ms.buffer(buf)
	return type
		
def main ():
	if not len(sys.argv) > 2:
		iso_imagename = sys.argv[1]
		iso = iso9660.ISO9660.IFS(iso_imagename)
		file_name = iso_imagename[:-4].split('/')[-1] + '.magic'
		file = open(file_name, 'a')
		for i in bft(iso, '/'):
			t = isomagic_file_info(iso, i)
			file.write('File: ' + i + ', Magic File Info: ' + t + '\n')
		file.close()
		iso.close()
		sys.exit(0)	
	else:
		iso_imagename = sys.argv[1]
		iso_filename = sys.argv[2]
		iso = iso9660.ISO9660.IFS(iso_imagename)
		t = isomagic_file_info(iso, iso_filename)
		print t
		iso.close()
		sys.exit(0)

if __name__ == '__main__':
	main()

	

