/*-
 * Copyright (c) 2010 FUKAUMI Naoki.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/mman.h>
#include <err.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tcccrc.h"

struct hdr {
	char str1[12];
	uint8_t crc[4];
	char str2[16];
	char str3[16];
} __attribute__ ((packed));

int
main(int argc, char *argv[])
{
	struct hdr h;
	off_t size;
	uint32_t crc = 0;
	uint8_t *buf, ih[16];
	int fd, ofd;

	if (argc != 5) {
		fprintf(stderr, "usage: %s boot.img system.img recovery.img "
		    "tcc8900_mtd.img\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if ((ofd = open(argv[4], O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1)
		err(EXIT_FAILURE, "%s", argv[4]);

	memset(&h, 0, sizeof(h));
	strncpy(h.str1, "[HEADER]0", sizeof(h.str1) - 1);
	strncpy(h.str2, "RAW_IMAGE", sizeof(h.str2) - 1);
	strncpy(h.str3, "MTD", sizeof(h.str3) - 1);
	TCCCRC(crc, &h, sizeof(h));
	write(ofd, &h, sizeof(h));

	if ((fd = open(argv[1], O_RDONLY)) == -1)
		err(EXIT_FAILURE, "%s", argv[1]);
	if ((size = lseek(fd, 0, SEEK_END)) == -1)
		err(EXIT_FAILURE, "%s", argv[1]);
	if ((buf = mmap(NULL, size, PROT_READ, MAP_SHARED | MAP_FILE, fd, 0))
	    == MAP_FAILED)
		err(EXIT_FAILURE, "%s", argv[1]);

	memset(ih, 0, sizeof(ih));
	ih[0] = 1;
	ih[8] = (size >> 0) & 0xff;
	ih[9] = (size >> 8) & 0xff;
	ih[10] = (size >> 16) & 0xff;
	ih[11] = (size >> 24) & 0xff;
	TCCCRC(crc, ih, sizeof(ih));
	write(ofd, ih, sizeof(ih));
	TCCCRC(crc, buf, size);
	write(ofd, buf, size);

	munmap(buf, size);
	close(fd);

	if ((fd = open(argv[2], O_RDONLY)) == -1)
		err(EXIT_FAILURE, "%s", argv[2]);
	if ((size = lseek(fd, 0, SEEK_END)) == -1)
		err(EXIT_FAILURE, "%s", argv[2]);
	if ((buf = mmap(NULL, size, PROT_READ, MAP_SHARED | MAP_FILE, fd, 0))
	    == MAP_FAILED)
		err(EXIT_FAILURE, "%s", argv[2]);

	memset(ih, 0, sizeof(ih));
	ih[0] = 2;
	ih[8] = (size >> 0) & 0xff;
	ih[9] = (size >> 8) & 0xff;
	ih[10] = (size >> 16) & 0xff;
	ih[11] = (size >> 24) & 0xff;
	TCCCRC(crc, ih, sizeof(ih));
	write(ofd, ih, sizeof(ih));
	TCCCRC(crc, buf, size);
	write(ofd, buf, size);

	munmap(buf, size);
	close(fd);

	if ((fd = open(argv[3], O_RDONLY)) == -1)
		err(EXIT_FAILURE, "%s", argv[3]);
	if ((size = lseek(fd, 0, SEEK_END)) == -1)
		err(EXIT_FAILURE, "%s", argv[3]);
	if ((buf = mmap(NULL, size, PROT_READ, MAP_SHARED | MAP_FILE, fd, 0))
	    == MAP_FAILED)
		err(EXIT_FAILURE, "%s", argv[3]);

	memset(ih, 0, sizeof(ih));
	ih[0] = 3;
	ih[8] = (size >> 0) & 0xff;
	ih[9] = (size >> 8) & 0xff;
	ih[10] = (size >> 16) & 0xff;
	ih[11] = (size >> 24) & 0xff;
	TCCCRC(crc, ih, sizeof(ih));
	write(ofd, ih, sizeof(ih));
	TCCCRC(crc, buf, size);
	write(ofd, buf, size);

	munmap(buf, size);
	close(fd);

	h.crc[0] = (crc >> 0) & 0xff;
	h.crc[1] = (crc >> 8) & 0xff;
	h.crc[2] = (crc >> 16) & 0xff;
	h.crc[3] = (crc >> 24) & 0xff;
	pwrite(ofd, h.crc, sizeof(h.crc), 12);

	close(ofd);

	return EXIT_SUCCESS;
}
