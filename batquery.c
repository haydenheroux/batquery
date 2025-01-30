#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* FIXME This isn't standardized anywhere. */
#define PATH_MAX 256

/* Pad a string with pad_len spaces. */
/* TODO There is likely already a standard method for padding. */
char *pad(size_t pad_len) {
  char *pad_str = (char *)calloc(pad_len + 1, sizeof(char));
  if (pad_str == NULL)
    return pad_str;
  memset(pad_str, ' ', pad_len);
  return pad_str;
}

/* Return true if the string starts with the prefix. */
bool prefix(const char *prefix, const char *str) {
  return strncmp(prefix, str, strlen(prefix)) == 0;
}

/* Reads a number from the string. Returns the number. Advances the string
 * pointer to after the number. */
double read_number(char **str) {
  char *start = strchr(*str, '=');
  char *end = strchr(start, '\n');
  /* Move one character forward, past the '='. */
  start++;
  *end = '\0';
  double number = atof(start);
  /* Replace the newline. */
  *end = '\n';
  /* Move one character forward past the newline. */
  *str = ++end;
  return number;
}

/* Print a usage message to stderr. */
void usage(const char *prog_name) {
  size_t pad_len = strlen("usage: ") + strlen(prog_name) + 1;
  char *pad_str = pad(pad_len);
  fprintf(stderr,
          "usage: %s [-i] [-p | -d | -t] <battery_path>\n%s -i: show battery "
          "status icon\n%s -p: show battery "
          "percent\n%s -d: show discharge rate\n%s -t: show time remaining\n",
          prog_name, pad_str, pad_str, pad_str, pad_str);
  free(pad_str);
}

/* Print an error message to stderr of format error_scope: error_msg and exit.
 */
/* TODO Stub this out if not in debug mode */
void error(const char *error_scope, const char *error_msg) {
  fprintf(stderr, "%s: %s\n", error_scope, error_msg);
  exit(EXIT_FAILURE);
}

/* Read content of a file into a string. */
void read_content_of_file(const char *battery_path, const char *file_name,
                          char *result_buffer, int len) {
  char full_file_path[PATH_MAX + 1];

  /* Create the battery capacity path. */
  strncpy(full_file_path, battery_path, PATH_MAX + 1);

  /* Add a trailing slash if one is not present. */
  /* TODO Move this logic to outside this function? */
  if (battery_path[strlen(battery_path) - 1] != '/')
    strncat(full_file_path, "/", PATH_MAX);

  /* battery_path/ -> battery_path/file_name */
  strncat(full_file_path, file_name, PATH_MAX);

  /* Delimit file path, just in case. */
  full_file_path[PATH_MAX] = '\0';

  FILE *file = fopen(full_file_path, "r");
  if (file == NULL)
    error("read_content_of_file", "unable to open file");

  size_t bytes = fread(result_buffer, sizeof(char), len, file);

  /* There is a case where the end of the file is encountered sooner than
   * expected, so only fail on explicit error. */
  if (bytes != sizeof(char) * len && ferror(file))
    error("read_content_of_file", "error while reading the file");

  fclose(file);
}

/* The numerical value of the battery capacity. */
int get_battery_percent(const char *battery_path) {
  char read_battery_capacity[4];
  read_battery_capacity[3] = '\0';

  read_content_of_file(battery_path, "capacity", read_battery_capacity, 3);

  /* FIXME Will break if there is more than one newline in the string. */
  for (int i = 3; i >= 0; --i) {
    if (read_battery_capacity[i] == '\n') {
      read_battery_capacity[i] = '\0';
      break;
    }
  }

  /* If nothing (or total garbage) was read there is nothing to do. */
  if (strlen(read_battery_capacity) == 0)
    error("get_battery_percent", "file contained no readable content");

  /* Convert the content of the file to a number. */
  long capacity = strtol(read_battery_capacity, NULL, 10);
  if (capacity == LONG_MIN || capacity == LONG_MAX)
    error("get_battery_percent", "file contained invalid content");

  return capacity;
}

/* True if the battery is charging, false if not. */
bool get_battery_charge_status(const char *battery_path) {
  char battery_charging;

  /* Read just the first character of the charge status */
  read_content_of_file(battery_path, "status", &battery_charging, 1);
  /* TODO Support more charge states. */
  /* TODO Use switch statement. */
  /* 'C' == charging */
  if (battery_charging == 'C')
    return true;

  return false;
}

/* The discharge rate of the battery in watts. */
double get_battery_discharge_rate(const char *battery_path) {
  char uevent[1024];
  uevent[1023] = '\0';

  read_content_of_file(battery_path, "uevent", uevent, 1024);

  double micro_volts = 0;
  double micro_amps = 0;

  char *line = uevent;
  while ((line = strchr(line, '\n')) != NULL) {
    /* Move one character forward, past the newline. */
    line++;

    if (prefix("POWER_SUPPLY_VOLTAGE_NOW=", line)) {
      micro_volts = read_number(&line);
    }

    if (prefix("POWER_SUPPLY_CURRENT_NOW=", line)) {
      micro_amps = read_number(&line);
    }
  }

  double volts = micro_volts / 1000000;
  double amps = micro_amps / 1000000;
  double battery_discharge_rate_watts = volts * amps;
  return battery_discharge_rate_watts;
}

/* The capacity of the battery now in watt-hours. */
double get_battery_capacity_now(const char *battery_path) {
  char uevent[1024];
  uevent[1023] = '\0';

  read_content_of_file(battery_path, "uevent", uevent, 1024);

  double charge_now_micro_amp_hours = 0;
  double voltage_now_micro_volts = 0;

  char *line = uevent;
  while ((line = strchr(line, '\n')) != NULL) {
    /* Move one character forward, past the newline. */
    line++;

    if (prefix("POWER_SUPPLY_CHARGE_NOW=", line)) {
      charge_now_micro_amp_hours = read_number(&line);
    }

    if (prefix("POWER_SUPPLY_VOLTAGE_NOW=", line)) {
      voltage_now_micro_volts = read_number(&line);
    }
  }

  double charge_now_amp_hours = charge_now_micro_amp_hours / 1000000;
  double voltage_now_volts = voltage_now_micro_volts / 1000000;
  double battery_capacity_now_watt_hours =
      charge_now_amp_hours * voltage_now_volts;
  return battery_capacity_now_watt_hours;
}

enum mode { NONE, PERCENT, DISCHARGE_RATE, TIME_REMAINING };

int main(int argc, char **argv) {
  bool show_icon = false;
  enum mode mode = NONE;
  int opt;

  /* Parse options using getopt; no per-option arguments. */
  while ((opt = getopt(argc, argv, "ipdt")) != -1) {
    switch (opt) {
    case 'i':
      show_icon = true;
      break;
    case 'p':
      mode = PERCENT;
      break;
    case 'd':
      mode = DISCHARGE_RATE;
      break;
    case 't':
      mode = TIME_REMAINING;
      break;
    default:
      usage(argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  /* Error if there is no final argument. */
  if (optind >= argc) {
    usage(argv[0]);
    error("args", "battery_path not specified");
  }

  /* Get the battery path from the last option. */
  const char *battery_path = argv[optind];

  /* Get battery information. */
  int percent = get_battery_percent(battery_path);
  /* TODO bool -> enum to support more battery charge states. */
  bool charging = get_battery_charge_status(battery_path);
  double discharge_rate_watts = get_battery_discharge_rate(battery_path);
  double capacity_now_watt_hours = get_battery_capacity_now(battery_path);
  double time_remaining_hours = capacity_now_watt_hours / discharge_rate_watts;

  switch (mode) {
  case NONE:
    usage(argv[0]);
    error("args", "mode flag not specified");
    break;
  case PERCENT:
    if (show_icon) {
      if (charging) {
        /* nf-md-battery_charging */
        fputs("󰂄", stdout);
      } else if (percent >= 100) {
        /* nf-md-battery */
        fputs("󰁹", stdout);
      } else if (percent >= 90) {
        /* nf-md-battery_90 */
        fputs("󰂂", stdout);
      } else if (percent >= 80) {
        /* nf-md-battery_80 */
        fputs("󰂁", stdout);
      } else if (percent >= 70) {
        /* nf-md-battery_70 */
        fputs("󰂀", stdout);
      } else if (percent >= 60) {
        /* nf-md-battery_60 */
        fputs("󰁿", stdout);
      } else if (percent >= 50) {
        /* nf-md-battery_50 */
        fputs("󰁾", stdout);
      } else if (percent >= 40) {
        /* nf-md-battery_40 */
        fputs("󰁽", stdout);
      } else if (percent >= 30) {
        /* nf-md-battery_30 */
        fputs("󰁼", stdout);
      } else if (percent >= 20) {
        /* nf-md-battery_20 */
        fputs("󰁻", stdout);
      } else if (percent >= 10) {
        /* nf-md-battery_10 */
        fputs("󰁺", stdout);
      } else {
        /* nf-md-battery_alert */
        fputs("󰂃", stdout);
      }
      putchar(' ');
    }
    printf("%d", percent);
    if (show_icon) {
      putchar('%');
    }
    break;
  case DISCHARGE_RATE:
    printf("%.2f", discharge_rate_watts);
    if (show_icon) {
      putchar(' ');
      putchar('W');
    }
    break;
  case TIME_REMAINING:
    if (show_icon) {
      double frac = fmod(time_remaining_hours, 1);
      int hours = floor(time_remaining_hours - frac);
      int minutes = floor(frac * 60);
      if (hours) {
        printf("%d hour", hours);
        if (hours > 1) {
          putchar('s');
        }
      }
      if (hours && minutes) {
        putchar(' ');
      }
      if (minutes) {
        printf("%d minute", minutes);
        if (minutes > 1) {
          putchar('s');
        }
      }
    } else {
      printf("%.2f", time_remaining_hours);
    }
    break;
  }

  putchar('\n');

  exit(EXIT_SUCCESS);
}
