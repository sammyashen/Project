#ifndef __XINZHI_CJSON_H_
#define __XINZHI_CJSON_H_

typedef struct{
	char *id;
	char *name;
	char *country;
	char *path;
	char *timezone;
	char *timezone_offset;

} xinzhi_location_config_t;

typedef struct{
	char *text;
	char *code;
	char *temperature;

} xinzhi_now_config_t;


typedef struct {
	xinzhi_location_config_t location_struct;
	xinzhi_now_config_t now_struct;
	char *last_update;

} xinzhi_results_config_t;

int user_parse_json(char *json_data);



#endif


