#include <sys/types.h>
#include <cdio/cdio.h>
#include <cdio/iso9660.h>
#include <cdio/types.h>
#include <cdio/logging.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <libgen.h>
#include <magic.h>
#include <time.h>
#include <libgen.h>

iso9660_t *p_iso;
uint8_t i_joliet_level;
const char *psz_fname;

magic_t mdb;


int extractfile(const iso9660_stat_t *p_statbuf)
{
  int i;
  int size = p_statbuf->size;
  int lsn  = p_statbuf->lsn;
  const char *mstring = 0;
  char buf[ISO_BLOCKSIZE];
  char filename[4096];

  iso9660_name_translate_ext(p_statbuf->filename, filename,i_joliet_level);

  // try reading file once to see if there's an error

  for (i = 0; i < size; i += ISO_BLOCKSIZE)
    {
      if ( ISO_BLOCKSIZE != iso9660_iso_seek_read (p_iso, buf, 
						   lsn + (i/ISO_BLOCKSIZE), 1) )
	{
	  printf("ERROR reading ISO 9660 file %s at LSN %lu\n",
		  psz_fname, (long unsigned int) lsn);
	  return;
	}
    }

  for (i = 0; i < size; i += ISO_BLOCKSIZE)
    {
      if ( ISO_BLOCKSIZE != iso9660_iso_seek_read (p_iso, buf, 
						   lsn + (i/ISO_BLOCKSIZE), 1) )
	{
	  // bad file just return !
	  return;
	}
      else
	{
	  if (i == 0)
	    {
	      mstring = magic_buffer(mdb, buf, 
			     (size > ISO_BLOCKSIZE) ? ISO_BLOCKSIZE : size);
	      printf ("FILE,%s,%s,%d\n", mstring,filename,size);
	    }
	  if (ISO_BLOCKSIZE > size -i )
	    {
	      fwrite(buf, 1, size - i, stdout);
	    }
	  else
	    {
	      fwrite(buf, 1, ISO_BLOCKSIZE, stdout);
	    }
	}
    }
}

int extractdir(const char *path)
{
  char newpath[4096];
  CdioList_t *entlist;
  CdioListNode_t *entnode;
  entlist = iso9660_ifs_readdir (p_iso, path);
  printf("DIR\n");
    
  /* Iterate over the list of nodes that iso9660_ifs_readdir gives  */

  if (entlist) {
    _CDIO_LIST_FOREACH (entnode, entlist)
      {
	char filename[4096];
	const char *mstring = 0;
	char timestring[128];
	iso9660_stat_t *p_statbuf = 
	  (iso9660_stat_t *) _cdio_list_node_data (entnode);
	iso9660_name_translate_ext(p_statbuf->filename, filename,i_joliet_level);
	//strftime(timestring, 128, "%d-%b-%Y %H:%M",&p_statbuf->tm);
	if (p_statbuf->type == _STAT_FILE)
	  {
	    char buf[ISO_BLOCKSIZE];
	    int size = p_statbuf->size;
	    if (iso9660_iso_seek_read(p_iso, buf, p_statbuf->lsn, 1))
	      {
		if (size > ISO_BLOCKSIZE)
		  mstring = magic_buffer(mdb, buf, ISO_BLOCKSIZE);
		else
		  mstring = magic_buffer(mdb, buf, size);
	      }
	    printf("file,%s,%s,%d,%d\n",mstring,filename,
		   mktime(&p_statbuf->tm), size);
	  }
	else
	  {
	    if ((filename && filename[0] && (filename[0] == '.') &&
		 ((filename[1] == 0) || ((filename[1] == '.') &&
					 (filename[2] == 0)))))
	      continue;
	    printf("dir,,%s,%d,0\n",filename, mktime(&p_statbuf->tm));
	  }
      }
    _cdio_list_free (entlist, true);
  }
}

void loghandler(cdio_log_level_t level, const char message[])
{
  if ((level == CDIO_LOG_ERROR) || (level == CDIO_LOG_ERROR))
    {
      printf("ERROR %s %s\n", psz_fname, message);
      exit(0);
    }
}


int findfile(const char *path )
{
  CdioList_t *entlist;
  CdioListNode_t *entnode;

  char *dname = strdup(path);
  char *fname = strdup(path);
  fname = basename(fname);
  dname = dirname(dname);

  entlist = iso9660_ifs_readdir(p_iso, dname);

  if (entlist) {
    _CDIO_LIST_FOREACH (entnode, entlist)
      {
	char filename[4096];
	iso9660_stat_t *p_statbuf =
	  	  (iso9660_stat_t *) _cdio_list_node_data (entnode);
	iso9660_name_translate_ext(p_statbuf->filename, filename,i_joliet_level);
        if (strcmp(filename, fname) == 0)
	  {
	    if (p_statbuf->type == _STAT_FILE)
	      extractfile(p_statbuf);
	    else
	      extractdir(path);
	    goto exit;
	  }
      }
  }
  else
    printf("ERROR couldn't find %s\n", dname);
  printf("ERROR couldn't open %s\n", path);
 exit:
  _cdio_list_free(entlist, true);
  //  free(fname);
  //  free(dname);
}


int main(int argc, const char *argv[])
{
  const char *path;
  if (argc < 3)
    return;
  
  psz_fname = argv[1];
  path = argv[2];

  cdio_log_set_handler(loghandler);
  mdb = magic_open(MAGIC_MIME);
  if (magic_load(mdb,"/usr/share/file/magic"))
    printf("ERROR problem loading mime database : %s", magic_error(mdb));
  //  p_iso = iso9660_open_fuzzy_ext (psz_fname,ISO_EXTENSION_ALL,5);
    p_iso = iso9660_open_ext (psz_fname, ISO_EXTENSION_ALL);
  //  p_iso = iso9660_open_ext (psz_fname,0);
  
  if (NULL == p_iso) {
    printf("ERROR Sorry, could not find an ISO 9660 image from %s\n", 
	    psz_fname);
    iso9660_close(p_iso);
    exit(1);
  }

  i_joliet_level = iso9660_ifs_get_joliet_level(p_iso);

  if (strcmp(path, "") == 0)
    extractdir(path);
  else
    findfile(path);

  magic_close(mdb);
  iso9660_close(p_iso);
}


    
