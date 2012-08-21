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

int
main(int argc, char *argv[])
{
	off_t size;
	uint32_t tmp;
	uint8_t *buf, crc[4], crc128k[4], *rgb;
	int fd, ofd, rgbfd;

	if (argc != 4) {
		fprintf(stderr, "usage: %s lk.rom rgb565le.raw lknew.rom\n",
		    argv[0]);
		exit(EXIT_FAILURE);
	}

	memset(crc, 0, sizeof(crc));
	memset(crc128k, 0, sizeof(crc128k));

	if ((ofd = open(argv[3], O_RDWR | O_CREAT | O_TRUNC, 0666)) == -1)
		err(EXIT_FAILURE, "%s", argv[3]);

	if ((fd = open(argv[1], O_RDONLY)) == -1)
		err(EXIT_FAILURE, "%s", argv[1]);
	if ((size = lseek(fd, 0, SEEK_END)) == -1)
		err(EXIT_FAILURE, "%s", argv[1]);
	if ((buf = mmap(NULL, size, PROT_READ, MAP_SHARED | MAP_FILE, fd, 0))
	    == MAP_FAILED)
		err(EXIT_FAILURE, "%s", argv[1]);

	if ((rgbfd = open(argv[2], O_RDONLY)) == -1)
		err(EXIT_FAILURE, "%s", argv[2]);
	if ((rgb = mmap(NULL, 768000, PROT_READ, MAP_SHARED | MAP_FILE,
	    rgbfd, 0)) == MAP_FAILED)
		err(EXIT_FAILURE, "%s", argv[2]);

	size -= 774716;

	tmp = 0;
	TCCCRC(tmp, buf, 16);
	TCCCRC(tmp, crc, 4);
	TCCCRC(tmp, buf + 28, size - 28);
	TCCCRC(tmp, rgb, 768000);
	TCCCRC(tmp, buf + size + 768000, 774716 - 768000);
	crc[0] = (tmp >> 0) & 0xff;
	crc[1] = (tmp >> 8) & 0xff;
	crc[2] = (tmp >> 16) & 0xff;
	crc[3] = (tmp >> 24) & 0xff;

	tmp = 0;
	TCCCRC(tmp, buf, 16);
	TCCCRC(tmp, crc, 4);
	TCCCRC(tmp, buf + 28, 131072 - 28);
	crc128k[0] = (tmp >> 0) & 0xff;
	crc128k[1] = (tmp >> 8) & 0xff;
	crc128k[2] = (tmp >> 16) & 0xff;
	crc128k[3] = (tmp >> 24) & 0xff;

	tmp = 0;

	write(ofd, buf, 16);
	write(ofd, crc128k, 4);
	write(ofd, &tmp, 4);
	write(ofd, crc, 4);
	write(ofd, buf + 28, size - 28);
	write(ofd, rgb, 768000);
	write(ofd, buf + size + 768000, 774716 - 768000);

	size += 774716;

	munmap(rgb, 768000);
	close(rgbfd);
	munmap(buf, size);
	close(fd);
	close(ofd);

	return EXIT_SUCCESS;
}
