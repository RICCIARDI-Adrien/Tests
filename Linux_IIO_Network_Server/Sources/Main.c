/** @file Main.c
 * Linux IIO internet server running on a non-Linux OS.
 * @author Adrien RICCIARDI
 */
#include <fcntl.h>
#include <ops.h>
#include <parser.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//-------------------------------------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------------------------------------
static ssize_t readfd_io(struct parser_pdata *pdata, void *dest, size_t len)
{
	return 0;
}

static ssize_t writefd_io(struct parser_pdata *pdata, const void *src, size_t len)
{
	return len;
}

//-------------------------------------------------------------------------------------------------
// Entry point
//-------------------------------------------------------------------------------------------------
int main(int __attribute__((unused)) argc, char __attribute__((unused)) *argv[])
{
	int flags, ret;
	struct iio_context *ctx = NULL;
	yyscan_t scanner;
	struct parser_pdata pdata;

	flags = fcntl(STDIN_FILENO, F_GETFL);
	fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
	flags = fcntl(STDOUT_FILENO, F_GETFL);
	fcntl(STDOUT_FILENO, F_SETFL, flags | O_NONBLOCK);

	pdata.ctx = ctx;
	pdata.stop = false;
	pdata.fd_in = STDIN_FILENO;
	pdata.fd_out = STDOUT_FILENO;
	pdata.verbose = true;
	pdata.pool = NULL;
	pdata.xml_zstd = NULL;
	pdata.xml_zstd_len = 0;
	pdata.fd_in_is_socket = false;
	pdata.fd_out_is_socket = false;
	pdata.is_usb = false;
	pdata.readfd = readfd_io;
	pdata.writefd = writefd_io;

	yylex_init_extra(&pdata, &scanner);

	do {
		output(&pdata, "iio-daemon > ");
		ret = yyparse(scanner);
	} while (!pdata.stop && ret >= 0);

	return EXIT_SUCCESS;
}
