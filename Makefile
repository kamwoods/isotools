all:  isomd5 isomagic isoextract

isomd5: isomd5.c
	gcc -o isomd5 isomd5.c -liso9660 -lcdio -lssl

isomagic: isomagic.c
	gcc -o isomagic isomagic.c -liso9660 -lcdio -lmagic

isoextract: isoextract.c
	gcc -g -o isoextract isoextract.c -liso9660 -lcdio -lmagic
