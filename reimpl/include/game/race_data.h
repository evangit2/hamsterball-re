/* race_data.h - Race data parser for RaceData.xml
 * Original: App_LoadRaceData reads par times and medal thresholds
 */
#ifndef RACE_DATA_H
#define RACE_DATA_H

#define MAX_RACE_TYPES 16
#define MAX_RACE_NAME 64

typedef struct {
    char name[MAX_RACE_NAME];     /* e.g. "BEGINNERRACE" */
    int time_limit;               /* Race time in seconds */
    float par_time;               /* Par time */
    float weasel_time;            /* Developer best time */
    float gold_time;
    float silver_time;
    float bronze_time;
    float cam_param;              /* Camera follow parameter */
} race_data_t;

typedef struct {
    race_data_t races[MAX_RACE_TYPES];
    int race_count;
} race_data_set_t;

/* Parse RaceData.xml */
int race_data_load(race_data_set_t *rds, const char *xml_path);

/* Find race by name */
race_data_t *race_data_find(race_data_set_t *rds, const char *name);

void race_data_free(race_data_set_t *rds);

#endif