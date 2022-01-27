/** @file Operations.c
 * Implement the operation functions called by the parser, see IIO ops.h for functions prototypes.
 * @author Adrien RICCIARDI
 */
#include <ops.h>
#include <stdio.h>

//-------------------------------------------------------------------------------------------------
// Private constants
//-------------------------------------------------------------------------------------------------
#define OPERATIONS_LOG(String_Format, ...) printf("[%s:%d] " String_Format, __FUNCTION__, __LINE__, ##__VA_ARGS__)

//-------------------------------------------------------------------------------------------------
// Private types
//-------------------------------------------------------------------------------------------------
struct iio_device
{
	int test;
};

struct iio_channel
{
	int test;
};

//-------------------------------------------------------------------------------------------------
// Private variables
//-------------------------------------------------------------------------------------------------
static struct iio_device Operations_IIO_Device;
static struct iio_channel Operations_IIO_Channel;

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
struct iio_device *iio_context_find_device(const struct iio_context __attribute__((unused)) *ctx, const char __attribute__((unused)) *name)
{
	return &Operations_IIO_Device;
}

struct iio_channel *iio_device_find_channel(const struct iio_device __attribute__((unused)) *dev, const char __attribute__((unused)) *name, bool __attribute__((unused)) output)
{
	return &Operations_IIO_Channel;
}

const char *iio_context_get_xml(const struct iio_context __attribute__((unused)) *ctx)
{
	return "Invalid XML";
}

int close_dev(struct parser_pdata __attribute__((unused)) *pdata, struct iio_device __attribute__((unused)) *dev)
{
	OPERATIONS_LOG("Closed device.\n");
	return 0;
}

ssize_t get_trigger(struct parser_pdata __attribute__((unused)) *pdata, struct iio_device __attribute__((unused)) *dev)
{
	OPERATIONS_LOG("Get trigger.\n");
	return 2;
}

int open_dev(struct parser_pdata __attribute__((unused)) *pdata, struct iio_device __attribute__((unused)) *dev, size_t samples_count, const char *mask, bool cyclic)
{
	OPERATIONS_LOG("Samples count : %lu, mask : \"%s\", cyclic : %d.\n", samples_count, mask, cyclic);
	return 0;
}

ssize_t read_chn_attr(struct parser_pdata __attribute__((unused)) *pdata, struct iio_channel __attribute__((unused)) *chn, const char *attr)
{
	OPERATIONS_LOG("Attribute : \"%s\".\n", attr);
	return 2;
}

ssize_t read_dev_attr(struct parser_pdata __attribute__((unused)) *pdata, struct iio_device __attribute__((unused)) *dev, const char *attr, enum iio_attr_type type)
{
	OPERATIONS_LOG("Attribute : \"%s\", type : %d.\n", attr, type);
	return 2;
}

ssize_t read_line(struct parser_pdata __attribute__((unused)) *pdata, char *buf, size_t len)
{
	OPERATIONS_LOG("Length : %lu.\n", len);
	buf[0] = 0;
	return 0;
}

ssize_t rw_dev(struct parser_pdata __attribute__((unused)) *pdata, struct iio_device __attribute__((unused)) *dev, unsigned int nb, bool is_write)
{
	OPERATIONS_LOG("Number : %u, is write : %d.\n", nb, is_write);
	return nb;
}

int set_buffers_count(struct parser_pdata __attribute__((unused)) *pdata, struct iio_device __attribute__((unused)) *dev, long value)
{
	OPERATIONS_LOG("Buffers count : %ld.\n", value);
	return 0;
}

int set_timeout(struct parser_pdata __attribute__((unused)) *pdata, unsigned int timeout)
{
	OPERATIONS_LOG("Timeout : %u.\n", timeout);
	return 0;
}

ssize_t set_trigger(struct parser_pdata __attribute__((unused)) *pdata, struct iio_device __attribute__((unused)) *dev, const char *trig)
{
	OPERATIONS_LOG("Trigger : \"%s\".\n", trig);
	return 2;
}

ssize_t write_all(struct parser_pdata __attribute__((unused)) *pdata, const void __attribute__((unused)) *src, size_t len)
{
	OPERATIONS_LOG("Bytes to write count : %lu.\n", len);
	return len;
}

ssize_t write_dev_attr(struct parser_pdata __attribute__((unused)) *pdata, struct iio_device __attribute__((unused)) *dev, const char *attr, size_t len, enum iio_attr_type type)
{
	OPERATIONS_LOG("Attribute : \"%s\", length : %lu, type : %d.\n", attr, len, type);
	return len;
}

ssize_t write_chn_attr(struct parser_pdata __attribute__((unused)) *pdata, struct iio_channel __attribute__((unused)) *chn, const char *attr, size_t len)
{
	OPERATIONS_LOG("Attribute : \"%s\", length : %lu.\n", attr, len);
	return len;
}
