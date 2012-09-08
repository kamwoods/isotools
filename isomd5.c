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
#include <openssl/md5.h>
#include <libgen.h>

iso9660_t *p_iso;
const char *psz_fname;
const char *base;
MD5_CTX fc;

void printchecksum(MD5_CTX *c)
{
  int i;
  unsigned char md[MD5_DIGEST_LENGTH];
  MD5_Final(md, c);
  for (i=0; i < MD5_DIGEST_LENGTH; i++)
    printf("%02x", md[i]);
}

int file(iso9660_stat_t *p_statbuf)
{
  MD5_CTX c;
  int size = p_statbuf->size;
  int lsn  = p_statbuf->lsn;
  char buf[ISO_BLOCKSIZE];
  int i;

  MD5_Init(&c);
  for (i = 0; i < size ; i += ISO_BLOCKSIZE) 
    {
      if ( ISO_BLOCKSIZE != iso9660_iso_seek_read (p_iso, buf, 
						   lsn + i/ISO_BLOCKSIZE , 1) )
	{
	  fprintf(stderr, "Fatal Error %s reading ISO 9660 at LSN %lu\n",
		  psz_fname, (long unsigned int) lsn);
	  return(1);
	}
      else
	{
	  if (ISO_BLOCKSIZE > size - i)
	    {
	      MD5_Update(&c, buf, size - i);
	      MD5_Update(&fc, buf, size - i);
	    }
	  else
	    {
	      MD5_Update(&c, buf, ISO_BLOCKSIZE);
	      MD5_Update(&fc, buf, ISO_BLOCKSIZE);
	    }
	}
    }
  printchecksum(&c);
  printf (" %s\n", base);
  return(0);
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
      return(1);
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
	iso9660_name_translate(p_statbuf->filename, filename);
	if (p_statbuf->type == _STAT_FILE)
	  {
	    if (file(p_statbuf))
	      {
		_cdio_list_free (entlist, true);
		return(1);
	      }
	  }
	else
	  {
	    if (dir(newpath,filename))
	      {
		_cdio_list_free (entlist, true);
		return(1);
	      }
	  }
      }
    _cdio_list_free (entlist, true);
    return(0);
  }
  else
    {
      fprintf(stderr,"Fatal Error %s Couldn't open %s\n", newpath, psz_fname);
      return(1);
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

  if (argc < 2)
    exit(0);
  psz_fname = argv[1];
  MD5_Init(&fc);
  p_iso = iso9660_open_ext (psz_fname, ISO_EXTENSION_NONE);
  
  if (NULL == p_iso) {
    fprintf(stderr, "Fatal Error %s could not find an ISO 9660 image\n", 
	    psz_fname);
    iso9660_close(p_iso);
    exit(1);
  }

  base = basename(psz_fname);
  if (dir(0, "") == 0)
    {
      printchecksum(&fc);
      printf(" %s\n", base);
    }
  iso9660_close(p_iso);
  exit(0);
}


    
