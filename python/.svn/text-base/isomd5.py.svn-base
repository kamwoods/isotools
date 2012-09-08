#A PYTHON FILE FOR GETTING MD5 CHECKSUMS FROM AN ISO9660 IMAGE
import os, sys
sys.path.insert(0, '/u/sjhoward/tmp/pycdio-0.13')
import pycdio
import iso9660
import stat
import md5
#import pdb

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

def md5_checksum (iso, iso_filename):
	#pdb.set_trace()
	if not iso.is_open():
		print "\nError...could not open %s as an IS09660 image.\n" % iso_imagename
		sys.exit(1)
	statbuf = iso.stat(iso_filename, True)
	if statbuf is None:
		print "Error...could not get file info for %s." % iso_filename
		iso.close()
		sys.exit(2)
	m = md5.new()
	blocks=ceil(statbuf['size'] / pycdio.ISO_BLOCKSIZE)
	counter = 0
	for i in range (blocks):
		lsn = statbuf['LSN'] + i
		size, buf = iso.seek_read(lsn)
		if pycdio.ISO_BLOCKSIZE > (statbuf['size'] - counter):
			m.update(buf[:statbuf['size'] - counter])
		else:
			m.update(buf)
		counter += pycdio.ISO_BLOCKSIZE
	if size <= 0:
		print "Error...invalid LSN %d in ISO9660 file %s." % (lsn, iso_filename)
		sys.exit(4)
	return m.hexdigest()	

def main ():
	if not len(sys.argv) > 2:	
		iso_imagename = sys.argv[1]
		iso = iso9660.ISO9660.IFS(iso_imagename)
		file_name = iso_imagename[:-4].split('/')[-1] + '.checksum'
		file = open(file_name, 'a')
		for f in bft(iso, '/'):	
			m = md5_checksum (iso, f)
			file.write('Hex md5 checksum for ' + f + ' is ' + m + '\n')
		file.close()
		iso.close()
		sys.exit(0)
	else:
		iso_imagename = sys.argv[1]
		iso_filename = sys.argv[2]
		iso = iso9660.ISO9660.IFS(iso_imagename)
		m = md5_checksum (iso, iso_filename)
		print 'Hex md5 digest number for ' + iso_filename + ' is ' + m	
		iso.close()
		sys.exit(0)	

if __name__ == '__main__':
	main()

	
