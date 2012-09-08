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

#define CEILING(x, y) ((x+(y-1))/y)
#define MIN(x,y) ((x < y) ? x : y)
iso9660_t *p_iso;
const char *psz_fname;
char *base;
uint8_t i_joliet_level;


magic_t mdb;

int file(iso9660_stat_t *p_statbuf, const char *path, const char *name)
{
  int i;
  const char *mstring = 0;
  int size = p_statbuf->size;
  char buf[ISO_BLOCKSIZE];
  const lsn_t lsn = p_statbuf->lsn;

  if ( ISO_BLOCKSIZE != iso9660_iso_seek_read (p_iso, buf, lsn, 1) )
    {
      fprintf(stderr, "Error reading ISO 9660 file %s at LSN %lu\n",
	      psz_fname, (long unsigned int) lsn);
      return;
    }
  else
    {
      if (size > ISO_BLOCKSIZE)
	mstring = magic_buffer(mdb, buf, ISO_BLOCKSIZE);
      else
	mstring = magic_buffer(mdb, buf, size);
    }
  printf (" %s/%s %s\n", path, name, mstring);
}

int dir(const char *path, const char *dirname)
{
  char newpath[4096];
  CdioList_t *entlist;
  CdioListNode_t *entnode;

  if (dirname && (dirname[0] == '.') && ((dirname[1] == 0) ||
					 ((dirname[1] == '.') &&
					  (dirname[2] == 0))))
    return;

  if ((path ? strlen(path) : 0)+ strlen(dirname) + 2 > 4096)
    {
      fprintf(stderr, "string too long %s/%s\n", path, dirname);
      return;
    }

  if (path)
    {
      strcat(strcat(strcpy(newpath, path), "/"),dirname);
    }
  else
    {
      strcpy(newpath, dirname);
    }
  
  entlist = iso9660_ifs_readdir (p_iso, newpath);
    
  /* Iterate over the list of nodes that iso9660_ifs_readdir gives  */

  if (entlist) {
    _CDIO_LIST_FOREACH (entnode, entlist)
      {
	char filename[4096];
	iso9660_stat_t *p_statbuf = 
	  (iso9660_stat_t *) _cdio_list_node_data (entnode);
     iso9660_name_translate_ext(p_statbuf->filename, filename,i_joliet_level);
	if (p_statbuf->type == _STAT_FILE)
	  {
	    file(p_statbuf,newpath,filename);
	  }
	else
	  {
	    dir(newpath,filename);
	  }
      }
    _cdio_list_free (entlist, true);

  }
}

void loghandler(cdio_log_level_t level, const char message[])
{
  if ((level == CDIO_LOG_ERROR) || (level == CDIO_LOG_ERROR))
    {
      fprintf(stderr, "Critical Error %s %s\n", psz_fname, message);
    }
}

int main(int argc, const char *argv[])
{
  cdio_log_set_handler(loghandler);
  mdb = magic_open(MAGIC_MIME);
  if (magic_load(mdb,"/usr/share/file/magic"))
    fprintf(stderr, "problem loading mime database : %s", magic_error(mdb));

  if (argc < 2)
    exit(0);
  psz_fname = argv[1];
  p_iso = iso9660_open_ext (psz_fname, ISO_EXTENSION_ALL);
  
  if (NULL == p_iso) {
    fprintf(stderr, "Sorry, could not find an ISO 9660 image from %s\n", 
	    psz_fname);
    iso9660_close(p_iso);
    exit(1);
  }
  i_joliet_level = iso9660_ifs_get_joliet_level(p_iso);
  base = basename(psz_fname);
  dir(0, "");
  magic_close(mdb);
  iso9660_close(p_iso);
}


    
