#include <getopt.h>
#include <linux/limits.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* pad a string with pad_len spaces */
char* pad(size_t pad_len)
{
	char* pad_str = (char*) calloc(pad_len+1, sizeof(char));
	if (pad_str == NULL)
		return pad_str;
	memset(pad_str, ' ', pad_len);
	return pad_str;
}

/* print a usage message to stderr */
int usage(const char* prog_name)
{
	size_t pad_len = strlen("usage: ")+strlen(prog_name)+1;
	char* pad_str = pad(pad_len);
	fprintf(stderr, "usage: %s [-i]  [-c] [-p | -t] <battery_path>\n%s -i: show battery status icon\n%s -c: show charging status\n%s -p: show battery percent\n%s -t: show time remaining\n", prog_name, pad_str, pad_str, pad_str, pad_str);
	free(pad_str);
	exit(EXIT_FAILURE);
}

/* print a usage message to stderr */
int error(const char* error_scope, const char* error_msg)
{
	fprintf(stderr, "%s: %s\n", error_scope, error_msg);
	exit(EXIT_FAILURE);
}

void read_content_of_file(const char* battery_path, const char* file_name, char* result_buffer, int len)
{
	char full_file_path[PATH_MAX+1];

	/* create the battery capacity path */
	strncpy(full_file_path, battery_path, PATH_MAX+1);

	if (battery_path[strlen(battery_path)-1] != '/') {
		strncat(full_file_path, "/", PATH_MAX);
		strncat(full_file_path, file_name, PATH_MAX);
	} else {
		strncat(full_file_path, file_name, PATH_MAX);
	}

	FILE* file = fopen(full_file_path, "r");
	if (file == NULL)
		error("read_content_of_file", "unable to open file");
	fread(result_buffer, sizeof(char), len, file);
	fclose(file);
}

/* the numerical value of the battery capacity */
int get_battery_percent(const char* battery_path)
{

	char read_battery_capacity[4];
	read_battery_capacity[3] = '\0';

	read_content_of_file(battery_path, "capacity", read_battery_capacity, 3);

	/* FIXME: will break if there is more than one newline in the string */
	for (int i = 3; i >= 0; --i) {
		if (read_battery_capacity[i] == '\n') {
			read_battery_capacity[i] = '\0';
			break;
		}
	}

	if (strlen(read_battery_capacity) == 0)
		error("get_battery_percent", "file contained no readable content");

	/* convert the content of the file to a number  */
	long capacity = strtol(read_battery_capacity, NULL, 10);
	if (capacity == LONG_MIN || capacity == LONG_MAX)
		error("get_battery_percent", "file contained invalid content");

	return capacity;
}

/* true if the battery is charging, false if not */
bool get_battery_charge_status(const char* battery_path)
{
	return true;
}


int main(int argc, char** argv)
{
	bool show_icon = false, show_percent = true, show_time = false, show_charging = false;
	int opt;

	/* parse options using getopt; no per-option arguments */
	while ((opt = getopt(argc, argv, "icpt")) != -1) {
		switch (opt) {
			case 'i':
				show_icon = true;
				break;
			case 'c':
				show_charging = true;
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

	/* get the battery path from the last option */
	if (optind >= argc)
		error("args", "battery_path not specified");
	const char* battery_path = argv[optind];

	int percent = get_battery_percent(battery_path);
	bool charging = get_battery_charge_status(battery_path);
	if (show_icon) {
		if (show_charging && charging) {
			/* nf-mdi-battery_charging */
			fputs("", stdout);
		} else if (percent >= 100) {
			/* nf-mdi-battery */
			fputs("", stdout);
		} else if (percent >= 90) {
			/* nf-mdi-battery_90 */
			fputs("", stdout);
		} else if (percent >= 80) {
			/* nf-mdi-battery_80 */
			fputs("", stdout);
		} else if (percent >= 70) {
			/* nf-mdi-battery_70 */
			fputs("", stdout);
		} else if (percent >= 60) {
			/* nf-mdi-battery_60 */
			fputs("", stdout);
		} else if (percent >= 50) {
			/* nf-mdi-battery_50 */
			fputs("", stdout);
		} else if (percent >= 40) {
			/* nf-mdi-battery_40 */
			fputs("", stdout);
		} else if (percent >= 30) { 
			/* nf-mdi-battery_30 */
			fputs("", stdout);
		} else if (percent >= 20) { 
			/* nf-mdi-battery_20 */
			fputs("", stdout);
		} else if (percent >= 10) {
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
