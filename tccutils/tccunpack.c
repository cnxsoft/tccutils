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

static const uint8_t hdr_old[] =
    { 0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00 };
static const uint8_t hdr_new[] =
    { 0x5b, 0x48, 0x45, 0x41, 0x44, 0x45, 0x52, 0x5d }; /* [HEADER] */

static const char *name[] = { "boot.img", "system.img", "recovery.img" };

int
main(int argc, char *argv[])
{
	off_t size;
	uint32_t ioff, isize;
	uint8_t *buf, *p;
	int fd, i, img;

	if (argc != 2) {
		fprintf(stderr, "usage: %s tcc8900_mtd.img\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if ((fd = open(argv[1], O_RDONLY)) == -1)
		err(EXIT_FAILURE, "%s", argv[1]);

	if ((size = lseek(fd, 0, SEEK_END)) == -1)
		err(EXIT_FAILURE, "%s", argv[1]);

	if ((buf = mmap(NULL, size, PROT_READ, MAP_SHARED | MAP_FILE, fd, 0))
	    == MAP_FAILED)
		err(EXIT_FAILURE, "%s", argv[1]);

	if (memcmp(buf, hdr_old, 8) == 0) {
		p = &buf[0x10];
		ioff = 0x40;
		for (i = 0; i < 3; i++) {
			isize = p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24;

			if ((img = open(name[i], O_WRONLY | O_CREAT | O_TRUNC,
			    0644)) == -1 ||
			    write(img, &buf[ioff], isize) == -1 ||
			    close(img) == -1)
				err(EXIT_FAILURE, "%s", name[i]);

			printf("%08x-%08x %s %d bytes\n", ioff,
			    ioff + isize - 1, name[i], isize);

			p += 4;
			ioff += isize;
		}
	} else if (memcmp(buf, hdr_new, 8) == 0) {
		p = &buf[0x38];
		ioff = 0x40;
		for (i = 0; i < 3; i++) {
			isize = p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24;

			if ((img = open(name[i], O_WRONLY | O_CREAT | O_TRUNC,
			    0644)) == -1 ||
			    write(img, &buf[ioff], isize) == -1 ||
			    close(img) == -1)
				err(EXIT_FAILURE, "%s", name[i]);

			printf("%08x-%08x %s %d bytes\n", ioff,
			    ioff + isize - 1, name[i], isize);

			p += isize + 0x10;
			ioff += isize + 0x10;
		}
	} else
		errx(EXIT_FAILURE, "invalid signature");

	munmap(buf, size);
	close(fd);

	return EXIT_SUCCESS;
}
