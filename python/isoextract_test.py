import sys
import time
import pycdio
import iso9660
import isoextract
import os
import types

def main ():
#	isos = ['/raid/sudoc_iso/30000050015340.iso']
#	isos = ['/raid/sudoc_iso/30000102565649.iso']
	isos = os.listdir('/raid/sudoc_iso')
	sourcepath = '/raid/sudoc_iso'
#	basepath = '/home/digarchive/sjhoward'		
#	basepath = '/u/sjhoward/PythonISO/python/test' 
	counter = 0
	LOG = open('iso_extraction.log', 'a')
	for i in isos[0:100]:
		counter += 1
		iso_imagename = os.path.join(sourcepath, i)
		iso = iso9660.ISO9660.IFS(iso_imagename)

		LOG.write('------BEGIN ERROR LIST FOR ' + iso_imagename + '------\n\n')
#		start_time = time.clock()
		
#		fullpath = os.path.join(basepath, iso_imagename.split('/')[-1][:-4])
#		os.mkdir(fullpath, 0777)
				
		successes = 0
		total_failures = 0 
		null_extent_failures = 0
		try:
			for file in isoextract.bft(iso, '/'):	
				try:
					#isoextract.extract_file(iso, iso_imagename, file, fullpath) 
					isoextract.extract_file(iso, iso_imagename, file) 
					LOG.write('  Successfully extracted ' + file + '\n')
				except Exception, e:		
					LOG.write('  Error extracting ' + file +
							': ' + e.__str__() + '\n')
					total_failures += 1
					if isinstance(e, isoextract.ISONullExtentError):
						null_extent_failures += 1
				else:
					successes += 1
		except Exception, e:
			LOG.write('  Error generating depth-first traversal for ' + iso_imagename +
					': ' + e.__str__() + '\n')

#		end_time = time.clock()
#		proc_time = end_time - start_time	
		"""
		if counter % 300:
			trash = os.listdir(fullpath)
			for t in trash:
				try:
					os.remove(os.path.join(fullpath, t))
				except Exception, e:
					LOG.write('Error removing extracted file ' +
							os.join(fullpath, t) + ' ' +
							e.__str__() + '\n')
			try:	
				os.rmdir(fullpath)
			except Exception, e:
				LOG.write('Error removing directory ' + fullpath + ': ' +
						e.__str__() + '\n')	
		else:
			LOG.write('Saving extracted files in /home/digarchive/sjhoward\n')
		"""		
		LOG.write('\nSuccessfully extracted ' + repr(successes) + ' files \nErrors extracting ' + repr(total_failures) + ' files\n' +
					'Null extent errors from ' + repr(null_extent_failures) + ' files\n' +
					'\n--------END ERROR LIST FOR ' + iso_imagename + '----\n...\n...\n...\n')
					#'\nProcessor time: ' + repr(proc_time) + '\n' +
		LOG.flush()
		iso.close()

	LOG.close()
	#iso.close()
	sys.exit(0)


if __name__ == '__main__':
	main() 
