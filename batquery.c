#include <getopt.h>
#include <linux/limits.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* pad(size_t pad_len)
{
	char* pad_str = (char*) calloc(pad_len + 1, sizeof(char));
	if (pad_str == NULL)
		return pad_str;
	memset(pad_str, ' ', pad_len);
	return pad_str;
}

int usage(const char* prog_name)
{
	size_t pad_len = strlen("usage: ") + strlen(prog_name) + 1;
	char* pad_str = pad(pad_len);
	fprintf(stderr, "usage: %s [-i] [-p | -t] <battery_path>\n", prog_name);
	fprintf(stderr, "%s -i: show battery status icon\n", pad_str);
	fprintf(stderr, "%s -p: show battery percent\n", pad_str);
	fprintf(stderr, "%s -t: show time remaining\n", pad_str);
	free(pad_str);
	exit(EXIT_FAILURE);
}

int error(const char* error_scope, const char* error_msg)
{
	fprintf(stderr, "%s: %s\n", error_scope, error_msg);
	exit(EXIT_FAILURE);
}

int get_battery_percent(const char* battery_path)
{
	char battery_capacity_path[PATH_MAX];
	int n;

	if (battery_path[strlen(battery_path)-1] != '/') {
		n = snprintf(battery_capacity_path, PATH_MAX, "%s/capacity", battery_path);
	} else {
		n = snprintf(battery_capacity_path, PATH_MAX, "%scapacity", battery_path);
	}

	if (n < 0) {
		error("get_battery_percent", "unable to create path");
	}

	FILE* battery_capacity_file = fopen(battery_capacity_path, "r");
	if (battery_capacity_file == NULL)
		error("get_battery_percent", "unable to open file");

	char read_battery_capacity[4];
	read_battery_capacity[3] = '\0';
	/* TODO: there is likely an instance where fread will fail; currently this is unhandled */
	fread(read_battery_capacity, sizeof(char), 3, battery_capacity_file);
	fclose(battery_capacity_file);

	for (int i = 0; i < 3; i++) {
		if (read_battery_capacity[i] == '\n')
			read_battery_capacity[i] = '\0';
	}

	if (strlen(read_battery_capacity) == 0)
		error("get_battery_percent", "file contained no readable content");

	long capacity = strtol(read_battery_capacity, NULL, 10);
	if (capacity == LONG_MIN || capacity == LONG_MAX)
		error("get_battery_percent", "file contained invalid content");

	return capacity;
}

int main(int argc, char** argv)
{
	bool show_icon = false, show_percent = true, show_time = false;
	int opt;
	while ((opt = getopt(argc, argv, "ipt")) != -1) {
		switch (opt) {
			case 'i':
				show_icon = true;
				break;
			case 'p':
				show_percent = true;
				if (show_time)
					show_time = false;
				break;
			case 't':
				show_time = true;
				if (show_percent)
					show_percent = false;
				break;
			default:
				usage(argv[0]);
		}
	}

	if (optind >= argc)
		error("args", "battery_path not specified");
	const char* battery_path = argv[optind];
	if (battery_path == NULL || strlen(battery_path) == 0)
		error("args", "battery_path specified was empty");

	int percent = get_battery_percent(battery_path);
	if (show_icon) {
		if (percent == 100) {
			/* nf-mdi-battery */
			fputs("", stdout);
		} else if (percent >= 90 && percent < 100) {
			/* nf-mdi-battery_90 */
			fputs("", stdout);
		} else if (percent >= 80 && percent < 90) {
			/* nf-mdi-battery_80 */
			fputs("", stdout);
		} else if (percent >= 70 && percent < 80) {
			/* nf-mdi-battery_70 */
			fputs("", stdout);
		} else if (percent >= 60 && percent < 70) {
			/* nf-mdi-battery_60 */
			fputs("", stdout);
		} else if (percent >= 50 && percent < 60) {
			/* nf-mdi-battery_50 */
			fputs("", stdout);
		} else if (percent >= 40 && percent < 50) {
			/* nf-mdi-battery_40 */
			fputs("", stdout);
		} else if (percent >= 30 && percent < 40) {
			/* nf-mdi-battery_30 */
			fputs("", stdout);
		} else if (percent >= 20 && percent < 30) {
			/* nf-mdi-battery_20 */
			fputs("", stdout);
		} else if (percent >= 10 && percent < 20) {
			/* nf-mdi-battery_10 */
			fputs("", stdout);
		} else {
			/* nf-mdi-battery_alert */
			fputs("", stdout);
		}
		putchar(' ');
	}

	if (show_percent)
		printf("%d%%\n", percent);

	exit(EXIT_SUCCESS);
}
